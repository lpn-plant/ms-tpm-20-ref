import time

def wait_for_response(ser):
    response = bytearray()
    
    while ser.inWaiting() <= 0:
    	time.sleep(0.01)
        
    while ser.inWaiting() > 0:
    	data = ser.read(1)
    	input = int.from_bytes(data, "little")
    	response.append(input)
    return response

def compare_cmd(cmd, ref, name):
    if cmd == ref:
        print("\tValid {} command received.".format(name))
    else:
        print("\tInvalid {} command received.".format(name))
        print(res)
