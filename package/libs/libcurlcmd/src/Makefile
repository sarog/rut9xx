CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libcurlcmd.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=curlcmd.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -leventslog -o $@ $^
