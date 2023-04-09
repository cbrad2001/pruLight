# Makefile for building embedded application.
# by Brian Fraser

# Edit this file to compile extra C files into their own programs.
TARGETS = my-linux-code
SOURCES = accel_drv.c main.c helpers.c i2c_drv.c game.c pru_code.c analogDisplay.c

PUBDIR = $(HOME)/cmpt433/public/myApps

OUTDIR = $(PUBDIR)
CROSS_TOOL = arm-linux-gnueabihf-
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror -Wshadow
# -pg for supporting gprof profiling.
# CFLAGS += -pg


# Convert the targets to full-path executables to delete.
OUTPUTS= $(addprefix $(OUTDIR)/, $(TARGETS))

# all: $(TARGETS) PrimeThread
all: $(TARGETS)

$(TARGETS):
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(OUTDIR)/$(TARGETS) -lpthread -lm

PrimeThread:
	make --directory=primeThread

testModules:
	make -C ../testModule

clean:
	rm -f $(OUTPUTS)
