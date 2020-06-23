CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=librex_lua.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=librex_lua.o librex_lua_f.o common.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -Wall -llua -lpcre -shared -fPIC -DLUA_USE_POSIX -DLUA_USE_DLOPEN -o $@ $^ -I/usr/include/lua5.1
