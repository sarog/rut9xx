APP_SOURCES = integrate.cc main.cc
APP_TARGET = cmStreamApp
CFLAGS = -Wall -Wextra -lsera -luci -lpthread
RM = rm -f

all:
	$(CXX) -std=c++11 $(APP_SOURCES) $(CFLAGS) -o $(APP_TARGET)

clean:
	$(RM) $(APP_TARGET)
