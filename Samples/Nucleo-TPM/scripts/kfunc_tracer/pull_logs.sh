#!/bin/bash
set -e

if [ -n "$RASPI_IP" ]; then
  echo "RASPI_IP: $RASPI_IP"
else
  echo "RASPI_IP env var not supplied"
  echo "example RASPI_IP export:"
  echo "  export RASPI_IP=pi@192.168.8.170"
fi

echo "creating ./logs directory"
mkdir -p logs
echo "entering ./logs directory"
cd logs
# create tmp file to suppress
# rm: cannot remove '*': No such file or directory
touch tmp
echo "cleaning ./logs directory"
rm *

echo "collecting dmesg tpm_addr logs"
ssh $RASPI_IP dmesg | grep tpm_addr  | sed "s/^\[[ ]*\?\([0-9.]*\)\] \(.*\)/\\1 \\2/"  > tpm_module.log
echo "collecting tpm related /proc/modules entries"
ssh $RASPI_IP sudo cat /proc/modules | grep tpm > modules_addr.log


ssh $RASPI_IP readelf -s /usr/lib/modules/5.10.76-v7+/kernel/drivers/char/tpm/tpm.ko | grep FUNC > tpm.log
ssh $RASPI_IP readelf -s /usr/lib/modules/5.10.76-v7+/kernel/drivers/char/tpm/tpm_tis_core.ko | grep FUNC > tpm_tis_core.log
ssh $RASPI_IP readelf -s /usr/lib/modules/5.10.76-v7+/kernel/drivers/char/tpm/tpm_tis_spi.ko | grep FUNC > tpm_tis_spi.log

