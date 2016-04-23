#===============================================================================
#	DEFINES
#===============================================================================
INC_DIR = ../includes
#CC = gcc
CC = clang
#CFLAGS = -Wall
CFLAGS = -I$(INC_DIR) -Wall
LD = ld
CD = cd
MAKE = make



#===============================================================================
#	TARGETS
#===============================================================================
TARGETS = lib.o
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
LIB     = lib
NET     = net
HEADERS = $(INC_DIR)/$(wildcard *.h)  $(INC_DIR)/$(LIB)/$(wildcard *.h)			\
		  $(INC_DIR)/$(NET)/$(wildcard *.h)


#===============================================================================
#	RULES
#===============================================================================
.c.o:$(HEADERS)
	$(CC) $(CFLAGS) -c -o $*.o $<

$(TARGETS):$(OBJECTS) $(HEADERS)
#$(TARGETS):$(OBJECTS)
	$(LD) -r -o $@ $(OBJECTS)

clean:
	rm $(TARGETS)
	rm *.o
