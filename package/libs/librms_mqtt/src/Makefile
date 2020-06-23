CC=gcc
WFLAGS=-Wall
CFLAGS=-O2
INCLUDES=-Iinclude

LIBNAME=librms_mqtt.so

all: $(LIBNAME)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

LIBNL_OBJ=rms_mqtt_lib.o cJSON.o

$(LIBNAME): $(LIBNL_OBJ)
	$(CC) -shared -lmnfinfo -o $@ $^

