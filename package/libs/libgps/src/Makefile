OBJECTS = gps.o
TARGET = libgps.so
WFLAGS = -Wall -std=gnu99 -fPIC
INCLUDES = -Iinclude
LIBS += -ltlt_logger
CFLAGS += -std=gnu99

RM = rm -f

all: $(TARGET)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

$(TARGET): $(OBJECTS)
	$(CC) -shared -fPIC $(LIBS) -o $@ $^

clean:
	$(RM) $(TARGET) $(OBJECTS)
