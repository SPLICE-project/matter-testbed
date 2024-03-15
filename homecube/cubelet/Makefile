PREFIX ?= /usr
SBINDIR ?= $(PREFIX)/local/sbin
MANDIR ?= $(PREFIX)/share/man
PKG_CONFIG ?= pkg-config

OBJ_DIR = obj
INC_DIR = include
SRC_DIR = src
SRCS = $(SRC_DIR)/sniff.c $(SRC_DIR)/radiotap.c
OBJS = $(OBJ_DIR)/sniff.o $(OBJ_DIR)/radiotap.o
COMMON_DIR = ../common

DEPS = $(INC_DIR)/platform.h $(INC_DIR)/radiotap.h $(INC_DIR)/radiotap_iter.h

MKDIR ?= mkdir -p
INSTALL ?= install
CC ?= "gcc"

CFLAGS ?= -MMD -O2 -g
CFLAGS += -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -I.

LIBS += -lpcap -ljson-c
LLIBS = -I $(COMMON_DIR)/cube_cubelet_comm.h

all: sniff port-scan

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	 $(CC) -c -o $@ $< $(CFLAGS)

sniff: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(LLIBS)

port-scan:
	$(CC) $(SRC_DIR)/port-scan.c -o port-scan -ljson-c

clean:
	rm -f $(OBJ_DIR)/*.o sniff port-scan
	rm /usr/local/sbin/sniff
	rm /usr/local/sbin/port-scan

install:
	mv sniff $(SBINDIR)
	mv port-scan $(SBINDIR)

.PHONY : clean
