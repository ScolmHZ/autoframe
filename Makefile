#!bash

INC	= ./include
SRC = ./src
BIN = ./
OBJSTMP ?= ./objs_tmp

#DBG_ON_SERVER = 0

ifndef DBG_ON_SERVER 
CROSS = arm-hisiv500-linux-
CC 		= @echo " GCC $@"; $(CROSS)gcc
CPP		= @echo " G++ $@"; $(CROSS)g++
LD		= @echo " LD  $@"; $(CROSS)ld
AR		= @echo " AR	$@"; $(CROSS)ar
RM		= @echo " RM	$@"; rm -f
else
CC 		= @echo " GCC $@"; gcc
CPP		= @echo " G++ $@"; g++
LD		= @echo " LD  $@"; ld
AR		= @echo " AR	$@"; ar
RM		= @echo " RM	$@"; rm -f
endif
OBJS		= $(OBJSTMP)/blurecheck.o \
					$(OBJSTMP)/ia_interface.o \
					$(OBJSTMP)/ia_remnantdetection.o \
					$(OBJSTMP)/ia_subfunc.o \
          			$(OBJSTMP)/ia_qpmap.o

#LIB_OBJS_D = $(patsubst $(SRC)/%.o,$(OBJSTMP)/%.o,$(OBJS))

CFLAGS	+= -I$(INC) 
CFLAGS	+= -Werror
CFLAGS	+= -march=armv5
CFLAGS	+= -Wall
CFLAGS	+= -O2 -Os


AFLAGS	+= -rv
CFLAGS_D:= $(CFLAGS) -g
LDFLAGS	+= -lpthread
LDFLAGS	+= -Wstrict-prototypes

LIBBLURE = $(TOPDIR)libautoframe$(EXT).a
LIBBLURE_D = $(TOPDIR)libautoframe$(EXT)-d.a

TARGET	= $(LIBBLURE) $(LIBBLURE_D)

all: $(TARGET)

$(LIBBLURE): $(OBJS)
	@$(RM) $@;
	@$(AR) $(AFLAGS) $@ $^
$(LIBBLURE_D): $(OBJS_D)
	@$(RM) $@;
	@$(AR) $(AFLAGS) $@ $^
$(EXEC): $(OBJS) $(LIBS)
	@$(CC) -o $@ $^ $(LDFLAGS)


$(OBJSTMP)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	@$(CC)	 $(CFLAGS) -c -o	$@ $<
$(OBJSTMP)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	@$(CPP) $(CFLAGS) -c -o	$@ $<
$(OBJSTMP)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	@$(CC)	 $(CFLAGS_D) -c -o	$@ $<
$(OBJSTMP)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	@$(CPP) $(CFLAGS_D) -c -o	$@ $<



clean:
	@rm -f $(TARGET) $(OBJS) $(OBJS_D);
	@rm -rf $(OBJSTMP) $(DEBUGTMP);
	@find ./ -type f \( -name '*.elf*' -o -name '*.gdb' \) | xargs rm -f
