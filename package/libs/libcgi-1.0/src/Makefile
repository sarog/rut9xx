CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libcgi.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=base64.o cgi.o cookie.o error.o general.o list.o md5.o session.o string.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -o $@ $^