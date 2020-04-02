SHELL=/bin/sh
CROSS_COMPILE=/usr/bin/
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld

CFLAGS += -Wall
LDFLAGS +=-L.

MAKE_DIR=$(PWD)

CFLAGS += $(INCLS)
CXXFLAGS += $(INCLS)
#find all the sub-directory
VPATH=$(foreach dir,$(shell find . -type d),$(shell echo $(dir):))
#include all the sub-directory
INCLS=$(foreach dir,$(subst :, ,$(VPATH)),$(shell echo -I$(dir)))
#find all .c and produce .o
C_SRC_PATH = $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.c))
COBJS = $(subst .c,.o,$(C_SRC_PATH))
#find all .cxx and produce .o
CPP_SRC_PATH = $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.cxx))
CPPOBJS = $(subst .cxx,.o,$(CPP_SRC_PATH))

execobjs = $(COBJS) $(CPPOBJS)
#the scripts for compile
.PHONY: all

all: $(execobjs)
	$(CXX) -o test $(execobjs) $(LDFLAGS)

$(COBJS):%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(CPPOBJS):%.o:%.cc
	$(CC) -c $< -o $@ $(CXXFLAGS)
#the scripts for clean
.PHONY: clean

clean:
	rm test $(execobjs)
