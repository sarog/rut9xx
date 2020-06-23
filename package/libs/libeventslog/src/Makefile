CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libeventslog.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=eventslog.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -lsqlite3 -o $@ $^

