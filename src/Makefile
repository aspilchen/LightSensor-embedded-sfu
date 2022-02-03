CROSS_TOOL = arm-linux-gnueabihf-
CC_C = $(CROSS_TOOL)gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

MYPUBDIR = $(HOME)/Education/cmpt433/public/myApps
PUBDIR = $(HOME)/cmpt433/public/myApps

TARGETS = light_sampler
DEPENDS = main.o pingpot.o potDriver.o
OUTDIR = ""

all: $(TARGETS) deploy

home: $(TARGETS) myDeploy

%.o: %.c 
	$(CC_C) -c $(CFLAGS) $<

$(TARGETS): $(DEPENDS)
	$(CC_C) $^ -o $@ -lpthread

deploy:
	cp $(TARGETS) $(PUBDIR)

myDeploy:
	cp $(TARGETS) $(MYPUBDIR)

clean:
	rm *.o
	rm $(TARGETS)



## Makefile for building embedded application.
## by Brian Fraser
#
## Edit this file to compile extra C files into their own programs.
#TARGETS=main pingPot potDriver
#
#
#PUBDIR = $(HOME)/cmpt433/public/myApps
#OUTDIR = $(PUBDIR)
#CROSS_TOOL = arm-linux-gnueabihf-
#CC_CPP = $(CROSS_TOOL)g++
#CC_C = $(CROSS_TOOL)gcc
#
#CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror 
## -pg for supporting gprof profiling.
#CFLAGS += -pg
#
#
#
## Convert the targets to full-path executables to delete.
#OUTPUTS= $(addprefix $(OUTDIR)/, $(TARGETS))
#
#all: $(TARGETS)
#
#$(TARGETS):
#	$(CC_C) $(CFLAGS) $@.c -o pp -lpthread
#	
##$(OUTDIR)/$@ -lpthread
#
#
#clean:
#	rm -f $(OUTPUTS)
#