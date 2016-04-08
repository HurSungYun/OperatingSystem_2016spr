INCLUDE=-I/home/ywkim/OS/workspace/linux/include
GCCROOT=/home/ywkim/OS/opt/tizen-toolchain-4.9~git-i686_armv7l-tizen-linux-gnueabi-20151113/bin
CFLAGS=$(INCLUDE)
CC=$(GCCROOT)/armv7l-tizen-linux-gnueabi-gcc

all: selector

selector: selector.c
	@echo [Arm-cc] $<...
	@$(CC) $(CFLAGS) $< -o $@
