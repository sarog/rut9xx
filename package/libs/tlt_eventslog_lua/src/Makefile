CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=tlt_eventslog_lua.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=tlt_eventslog_lua.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -Wall -llua -leventslog -shared -fPIC -DLUA_USE_POSIX -DLUA_USE_DLOPEN -o $@ $^ -I/usr/include/lua5.1 -llua

