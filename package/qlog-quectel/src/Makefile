TEST_SOURCES = main.cpp serial.cpp vetify_arg.cpp
TEST_OBJS = $(TEST_SOURCES:.c=.o)
TEST_TARGET = qlog-quectel

CXXFLAGS=-lstdc++ -lc -lgcc_s -lusb-1.0 -pthread -w -Wall -g -O0 -fpermissive -lpthread

RM = rm -f

all:
	$(CC) $(TEST_SOURCES) $(CXXFLAGS) -o $(TEST_TARGET)

clean:
	$(RM) $(TEST_OBJS) $(TEST_TARGET)
