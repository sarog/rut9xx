CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=libsms_utils.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=smsutils.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -ltlt_uci -o $@ $^

