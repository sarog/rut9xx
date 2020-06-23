CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libioman.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=iomanl.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -ltlt_uci -lsms_utils -lgsm -lusb-1.0 -leventslog -o $@ $^

