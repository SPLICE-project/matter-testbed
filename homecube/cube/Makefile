PREFIX ?= /usr
SBINDIR ?= $(PREFIX)/local/sbin
SRC_DIR = src

CC=gcc
CFLAGS=-ljson-c
COMMON_DIR = ../common
INC = include

LLIBS = -I $(COMMON_DIR)/cube_cubelet_comm.h -I $(INC)/device_discovery.h

.PHONY: recv cubelet-commissioner test

all: recv cubelet-commissioner


%.o: $(SRC_DIR)%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(LLIBS)

recv:
	$(CC) $(SRC_DIR)/recv.c $(INC)/device_discovery.c -o recv -lsqlite3 -ljson-c -std=c99 $(LLIBS)

cubelet-commissioner:
	$(CC) $(SRC_DIR)/cubelet-commissioner.c -o cubelet-commissioner -lsqlite3 -std=c99

clean:
	rm *.o recv cubelet-commissioner
	rm $(SBINDIR)/recv
	rm $(SBINDIR)/cubelet-comissioner
	rm test

test:
	$(CC) $(SRC_DIR)/test.c $(INC)/device_discovery.c -o test -std=c99 $(LLIBS)

install:
	mv recv cubelet-commissioner $(SBINDIR)
