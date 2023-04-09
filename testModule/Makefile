PUBDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = $(PUBDIR)
CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

ACCEL_TARGETS = testAccel.c ../my-linux-code/accel_drv.c ../my-linux-code/i2c_drv.c ../my-linux-code/helpers.c
BUZZER_TARGETS = testBuzzer.c ../my-linux-code/buzzer.c ../my-linux-code/helpers.c

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow

all: testAccel testBuzzer

testAccel:
	$(CC_C) $(CFLAGS) $(ACCEL_TARGETS) -o $(OUTDIR)/testAccel

testBuzzer:
	$(CC_C) $(CFLAGS) $(BUZZER_TARGETS) -o $(OUTDIR)/testBuzzer -lpthread
	