CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=tlt_brand_lua.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=tlt_brand_lua.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -Wall -llua -lbrand -shared -fPIC -DLUA_USE_POSIX -DLUA_USE_DLOPEN -o $@ $^ -I/usr/include/lua5.1 -llua

