CROSS_TOOL = arm-linux-gnueabihf-
CC_C = $(CROSS_TOOL)gcc
CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

SRCDIR = src
OBJDIR = src/obj

TARGET = LightSampler
DEPENDS = a2d pot_driver a2dping network light_sampler cbuff udp i2cHelper gpioHelper segdis_driver main

SRC = $(addprefix $(SRCDIR)/,  $(DEPENDS:=.c))
OBJ = $(addprefix $(OBJDIR)/,  $(DEPENDS:=.o))

all: setup $(TARGET)

setup:
	mkdir -p $(OBJDIR)


$(TARGET): $(OBJ)
	$(CC_C) $^ -o $@ -lpthread


$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC_C) -c -o $@ $^ $(CFLAGS)


clean:
	rm -r $(OBJDIR)
	rm $(TARGET)