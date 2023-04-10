#!/bin/bash
# Necessary Config Pin requirements to run this assignment!

# 1. Config in/out pins for PRU 
config-pin P8_11 pruout
config-pin p8_16 pruin 
config-pin p8_15 pruin    
# Display something on 14-seg display:
# 2a. Enable I2C:
config-pin P9_18 i2c
config-pin P9_17 i2c

# 3. On target, compile PRU code (after copying to target):
#     make
#     On target, load compiled code:
#     make install_PRU0
# https://stackoverflow.com/questions/14219092/bash-script-bin-bashm-bad-interpreter-no-such-file-or-directory
