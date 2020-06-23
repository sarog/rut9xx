CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libmdcollectd.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=mdcollectd_data.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -o $@ $^

