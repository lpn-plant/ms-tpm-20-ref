import re
import json


# load and parse list of processes from log file to a dictionary.
#
# expected input format from `sudo cat /proc/modules'
# for example:
#   tpm_tis_spi 16384 0 - Live 0x7f0d5000 (O)
#   tpm_tis_core 24576 1 tpm_tis_spi, Live 0x7f116000 (O)
#
# output dictionary structure:
# proc_list[proc_name]['proc_mem_size'] - memory size of the module, in bytes (dec)
# proc_list[proc_name]['proc_instance'] - how many instances of the module are currently loaded
# proc_list[proc_name]['proc_depend'] - if not 0 the module depends upon another module
# proc_list[proc_name]['proc_state'] - load state of the module
# proc_list[proc_name]['proc_mem_offset'] - current kernel memory offset for the loaded module
# proc_list[proc_name]['other']
def get_proc_list(log_dir, file_name):
    with open(log_dir + '/' + file_name, "r") as infile:
        proc_list = {}
        for line in infile:
            line_splitted = re.split(" ", line.rstrip())
            proc_name, proc_mem_size, proc_instance, proc_depend, proc_state, proc_mem_offset, *other = line_splitted
            proc_list[proc_name] = {}
            proc_list[proc_name]['proc_mem_size'] = proc_mem_size
            proc_list[proc_name]['proc_instance'] = proc_instance
            proc_list[proc_name]['proc_depend'] = proc_depend
            proc_list[proc_name]['proc_state'] = proc_state
            proc_list[proc_name]['proc_mem_offset'] = proc_mem_offset
            proc_list[proc_name]['other'] = other
    return proc_list


# load and parse call stack trace data from log file to an array of dictionaries.
#
# expected input format from `dmesg | grep tpm_addr | sed "s/^\[[ ]*\?\([0-9.]*\)\] \(.*\)/\\1 \\2/"'
# sed and the end convert date for
# for example:
# 9.380291 tpm_addr_enter: 0x7f101070 0x8010228c
# 9.380447 tpm_addr_enter: 0x7f101000 0x7f10115c
# 9.380817 tpm_addr_exit: 0x7f101000 0x7f10115c
# 9.380832 tpm_addr_exit: 0x7f101070 0x8010228c
#
# logs are generated from following printk calls included in instrumented kernel module
# printk("tpm_addr_enter: 0x%px 0x%px \n", this_fn, call_site);
# printk("tpm_addr_exit: 0x%px 0x%px \n", this_fn, call_site);
#
# example raw output from dmesg
# [    9.189270] tpm_addr_enter: 0x7f093070 0x8010228c
# [    9.189424] tpm_addr_enter: 0x7f093000 0x7f09315c
# [    9.189799] tpm_addr_exit: 0x7f093000 0x7f09315c
# [    9.189815] tpm_addr_exit: 0x7f093070 0x8010228c
#
# output array structure:
# single callstack entry dictionary structure:
# callstack[0]['time'] - time in seconds
# callstack[0]['direction'] - entry or exit function
# callstack[0]['this_fn'] - address of the start of the current function (hex)
# callstack[0]['call_site'] - address of the start of the caller function (hex)
def get_callstack(log_dir, file_name):
    with open(log_dir+'/'+file_name, "r") as infile:
        callstack = []
        for line in infile:
            line_splitted = re.split(" ", line.rstrip())
            time, direction, this_fn, call_site = line_splitted
            fn_call = {'time': time, 'direction': direction, 'this_fn': this_fn, 'call_site': call_site}
            callstack.append(fn_call)
    return callstack


# load and parse list of module functions from log file to a dictionary.
#
# expected input format from `readelf -s /usr/lib/modules/5.10.76-v7+/kernel/drivers/char/tpm/tpm.ko | grep FUNC'
# for example:
#   66: 00000000    92 FUNC    LOCAL  DEFAULT    3 tpm_clk_enable
#   67: 0000005c   108 FUNC    LOCAL  DEFAULT    3 tpm_request_locality
#
# output dictionary structure:
# functions[addr (dec)] = function_name
def get_module_functions(log_dir, file_name):
    with open(log_dir + '/' + file_name, "r") as infile:
        functions = {}
        for line in infile:
            csv_line = ";".join(line.split())
            line_splitted = re.split(";", csv_line)
            it, addr, size, type, local, smth, nr, function_name = line_splitted
            addr_dec = int(addr, 16)
            functions[addr_dec] = function_name

        return functions


def get_real_offset_and_module(fn_call_addr, proc_list):
    output = ('unknown', 0)
    for proc_name in proc_list:
        proc = proc_list[proc_name]
        # convert string with hex value to int
        proc_start = int(proc['proc_mem_offset'], 16)
        # convert string with value to int
        proc_size = int(proc['proc_mem_size'], 10)
        # convert string with hex value to int
        proc_end = proc_start + proc_size

        if proc_start <= fn_call_addr <= proc_end:
            real_addr = fn_call_addr - proc_start
            output = (proc_name, real_addr)

    return output

LOGS_DIR = 'logs/'
MODULES_ADDR_FILE = 'modules_addr.log'
CALL_STACK_FILE = 'tpm_module.log'
modules_logs = ['tpm.log', 'tpm_tis_core.log', 'tpm_tis_spi.log']

callstack = get_callstack(LOGS_DIR, CALL_STACK_FILE)
proc_list = get_proc_list(LOGS_DIR, MODULES_ADDR_FILE)

modules_functions = {}
for module_log in modules_logs:
    module_name = module_log.rstrip(".log")
    modules_functions[module_name] = get_module_functions(LOGS_DIR, module_log)

# print(json.dumps(modules_functions, indent=2))
# print(json.dumps(callstack, indent=2))
# print(json.dumps(proc_list, indent=2))

for call in callstack:
    call_addr = int(call["this_fn"], 16)
    # print(hex(call_addr))

    proc_name, offset = get_real_offset_and_module(call_addr, proc_list)

    # print(str(proc_name) + ' module, offset ' + str(hex(offset)))

    if proc_name != 'unknown':
        print(proc_name + ".ko: " + modules_functions[proc_name][offset] + "()")
