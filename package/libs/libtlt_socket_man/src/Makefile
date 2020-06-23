CC=gcc
WFLAGS=-Wall
CFLAGS=-O2 -ltlt_base
INCLUDES=-Iinclude

LIBNAME=libtlt_socket_man.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=socket_man.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -o $@ $^
