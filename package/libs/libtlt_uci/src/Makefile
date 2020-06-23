CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libtlt_uci.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=libtlt_uci.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -luci -o $@ $^
