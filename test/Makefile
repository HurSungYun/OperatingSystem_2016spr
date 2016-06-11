INCLUDE=-I/home/eunhyangkim/os2016/linux/include
GCCROOT=/home/eunhyangkim/Documents/opt/tizen-toolchain-4.9~git-i686_armv7l-tizen-linux-gnueabi-20151113/bin
CFLAGS=$(INCLUDE)
CC=$(GCCROOT)/armv7l-tizen-linux-gnueabi-gcc

all: gpsupdate file_loc randomgps

gpsupdate: gpsupdate.c
	@echo [Arm-cc] $<...
	@$(CC) $(CFLAGS) $< -o $@

file_loc: file_loc.c
	@echo [Arm-cc] $<...
	@$(CC) $(CFLAGS) $< -o $@

randomgps: randomgps.c
	@echo [Arm-cc] $<...
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@rm gpsupdate
