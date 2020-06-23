OBJECTS = tlt_logger.o
TARGET = libtlt_logger.so
WFLAGS = -Wall -std=gnu99 -fPIC
INCLUDES = -Iinclude
LIBS += -luci
CFLAGS += -std=gnu99

RM = rm -f

all: $(TARGET)

%.o: %.c
	$(CC) $(WFLAGS) -c -o $@ $(INCLUDES) $(CFLAGS) $<

$(TARGET): $(OBJECTS)
	$(CC) -shared -fPIC $(LIBS) -o $@ $^

clean:
	$(RM) $(TARGET) $(OBJECTS)
