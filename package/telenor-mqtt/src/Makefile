TELENOR_MQTT_SOURCES = cJSON.c telenor-mqtt.c
TELENOR_MQTT_OBJS = $(TELENOR_MQTT_SOURCES:.c=.o)
TELENOR_MQTT_TARGET = telenor_mqtt
TELENOR_MQTT_LIBS = -lm -lcurl -lpthread -ltlt_uci -luci -lcrypto -lmnfinfo -lusb-1.0 -lmosquitto

CFLAGS =  -Wall -Wno-return-local-addr -Wno-comment -lmosquitto -lmnfinfo -lusb-1.0 -luci -lgsm -lgps -lubus -lubox -lblobmsg_json -ljson-c -leventslog -ltlt_uci -lunhandler -ltlt_base -ltlt_socket_man -DRUT
RM = rm -f

all:
	$(CC) $(TELENOR_MQTT_SOURCES) $(CFLAGS) $(TELENOR_MQTT_LIBS) -o $(TELENOR_MQTT_TARGET)

clean:
	$(RM) $(TELENOR_MQTT_OBJS) $TELENOR_MQTT_TARGET)
