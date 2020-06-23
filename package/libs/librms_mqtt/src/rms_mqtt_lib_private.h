#ifndef LIB_RMS_MQTT_PRIVATE_H
#define LIB_RMS_MQTT_PRIVATE_H

#include <stdlib.h>

#define RMS_SOCKET_NAME "/tmp/rms_mqtt.sock"

/**
 * Checks if unix socket is available and if possible, try to connect.
 * @return -1 if failed to connect to socket, any other number will be socket file descriptor.
 */
int connectToSocket();

/**
 * Function which makes write to the unix socket.
 * @param text data which will be sent.
 * @param length sendable data length. (Cannot use strlen, because first 4 bytes are converted integer.)
 * @return 1 if sending to socket was successful.
 */
int rmsMQTTSend(char *text, size_t length);

#endif