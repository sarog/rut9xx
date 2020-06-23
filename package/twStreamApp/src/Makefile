APP_SOURCES = main.c SteamThing.c
APP_OBJS = $(APP_SOURCES:.c=.o)
APP_TARGET = twStreamApp

CFLAGS = -Wall -Wextra -ltwCSdk "-DTW_OS_INCLUDE=\"$(TW_OS_INCLUDE)\""

INCLUDES = -I$(INCLUDE_DIR)/primitiveUtils \
	-I$(INCLUDE_DIR)/messaging \
	-I$(INCLUDE_DIR)/porting \
	-I$(INCLUDE_DIR)/config \
	-I$(INCLUDE_DIR)/api \
	-I$(INCLUDE_DIR)/utils \
	-I$(INCLUDE_DIR)/shapes \
	-I$(INCLUDE_DIR)/websocket \
	-I$(INCLUDE_DIR)/tls \
	-I$(INCLUDE_DIR)/standardProps \
	-I$(INCLUDE_DIR)/thirdParty/cJSON \
	-I$(INCLUDE_DIR)/threadUtils \
	-I$(INCLUDE_DIR)/fileTransfer

RM = rm -f

all:
	$(CC) $(APP_SOURCES) $(CFLAGS) $(INCLUDES) -o $(APP_TARGET)

clean:
	$(RM) $(APP_OBJS) $(APP_TARGET)
