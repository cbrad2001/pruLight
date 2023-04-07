PUBDIR = $(HOME)/cmpt433/public/myApps
OUTFILE = testAccel
OUTDIR = $(PUBDIR)
CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

TARGETS = testAccel.c ../my-linux-code/accel_drv.c ../my-linux-code/i2c_drv.c ../my-linux-code/helpers.c

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow

all:
	$(CC_C) $(CFLAGS) $(TARGETS) -o $(OUTDIR)/$(OUTFILE)
