# Project Title

Python scripts for `tpm2_tools` and `ms-tpm-20-ref` experiments.

## Description

The purpose of these scripts was to experiment using `ms-tpm-20-ref` and
`/dev/tty*` devices together with tpm2_tools.

Another goal was to investigate the behavior of loopback `tty` simulated
communication with tpm2_tools.

As it turns out both `tpm2-pytss` and `tpm2-pytss` libraries are both unable
to comunicate with STM32.

Some findings were made in order to find the root cause of failures.
Probably those scripts will be useful for future research - allowing the user to
test communication with STM32 based tpm device.

Installation instructions are provide to build/debug working version of `pytts`
library with compatible `tpm2_tss` lib version.

## Getting Started

### Dependencies

Install pyserial python package
```
pip3 install -r pyserial
```

### Installing

Build and install `tpm2-tss`

```
git clone https://github.com/tpm2-software/tpm2-tss.git
cd tpm2-tss
git checkout 3.1.0
./bootstrap
./configure  --with-device=/dev/ttyACM1 --with-maxloglevel=debug --disable-log-file --enable-debug=yes
make -j8
sudo make install

# To clean the repo some aditional steps are needed
sudo rm -rf doxygen-doc/ src/tss2-esys/.libs/ src/tss2-esys/api/.libs/ src/tss2-fapi/.libs/ src/tss2-fapi/api/.libs/ src/tss2-mu/.libs/ src/tss2-rc/.libs/ src/tss2-sys/.libs/ src/tss2-sys/api/.libs/ src/tss2-tcti/.libs/ src/util/.libs/ .libs/
```

Build and install `tpm2-pytts` - notice the exact commit, that matches version
3.1.0 of `tpm2-tss` library. 

```
git clone https://github.com/tpm2-software/tpm2-pytss
git checkout ba90c7fa9eeca95d7ba7bb89f1cb25ccac243367
python3 setup.py build
sudo python3 setup.py install

# To clean the repo some aditional steps are needed
sudo rm -rf .eggs/ tpm2_pytss.egg-info/ dist
```

### Running

1. Running tty simulator with tpm2_tools

The goal of this test was to replay tpm responses to the startup command.
It was done, to verify the actual messages required for `tpm2_startup`
application to terminate successfully.

Run serial simulator:
`python3 simulator.py`

Run pytss example
`python3 pytss_startup.py 'device:/tmp/ttydevice'`



https://stackoverflow.com/questions/46136178/how-to-write-read-with-poll-functionality-in-kernel-for-tty-driver
