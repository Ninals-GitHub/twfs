#===============================================================================
#	DEFINES
#===============================================================================
INC_DIR = ./includes
INC_CDIR = ./
#CC = gcc
CC = clang
LD = ld
CD = cd
MAKE = make

LIB_SSL = ssl
LIB_CRYPTO = crypto




#===============================================================================
#	TARGETS
#===============================================================================
TARGETS = twfs
LIB     = lib
NET     = net
OBJECTS = main.o twitter_operation.o twfs_internal.o twfs.o					\
		  $(LIB)/$(LIB).o $(NET)/$(NET).o
		  LIB_OBJS= $(LIB)/$(patsubst %.c, %.o, $(wildcard *.c))
		  NET_OBJS= $(NET)/$(patsubst %.c, %.o, $(wildcard *.c))
		  HEADERS = $(INC_DIR)/$(wildcard *.h) $(INC_DIR)/$(LIB)/$(wildcard *.h)		\
		  		  $(INC_DIR)/$(NET)/$(wildcard *.h) $(wildcard *.h)


#===============================================================================
#	FLAGS
#===============================================================================
#CFLAGS = -Wall
CFLAGS = -I$(INC_CDIR) -I$(INC_DIR) -g -Wall `pkg-config fuse --cflags`

#LDFLAGS = -L$(GNU_LIB_DIR) -L$(GNU_SSL_ENG) -L$(USR_LIB)
#LDFLAGS = -L$(GNU_LIB_DIR)
LDFLAGS = -I$(INC_DIR) -I$(INC_CDIR) -g -Wall `pkg-config fuse --libs` -lpthread -ltwfscrypt -Llib

#===============================================================================
 #	RULES
#===============================================================================
.c.o:$(HEADERS)
	$(CC) -c -o $*.o $< $(CFLAGS)

$(TARGETS):$(OBJECTS) $(HEADERS)
#$(TARGETS):$(OBJECTS)
	$(CC) -o $@ $(OBJECTS) -l$(LIB_SSL) -l$(LIB_CRYPTO) $(LDFLAGS)

$(LIB)/$(LIB).o:$(LIB_OBJS)
	( $(CD) $(LIB) ; $(MAKE) )

$(NET)/$(NET).o:$(NET_OBJS)
	( $(CD) $(NET) ; $(MAKE) )

count:
	cat $(LIB)/*.c $(NET)/*.c *.c $(INC_DIR)/*.h $(INC_DIR)/$(LIB)/*.h $(INC_DIR)/$(NET)/*.h | wc -l

clean:
	rm *.o
	( $(CD) $(LIB) ; $(MAKE) $@ )
	( $(CD) $(NET) ; $(MAKE) $@ )
	rm $(TARGETS)

