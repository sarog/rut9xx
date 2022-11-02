#ifndef LIB_RMS_MQTT_H
#define LIB_RMS_MQTT_H

/**
 * Sends mail data to rms_mqtt application through unix socket.<br>
 * After sending to rms_mqtt, MQTT client will send data to RMS server
 * and server will send mail to the recipient.
 * @param action_id action id which defines what type of message RMS will send (1 - RMS email, 2 - SMTP email)
 * @param alert_id id of alert which will trigger email sending in RMS server
 * @param message mail message.
 * @return 1 if sending to socket was successful.
 */
int sendRMSEmail(int action_id, int alert_id, char *message);

/**
 * Parses data and splits to variables which has been received from messaged<br>
 * Data is splitted by ':' symbol and must be in this order:<br>
 * action_id:alert_id:message
 * @param message
 * @return 1 if sending to socket was successful.
 */
int sendRMSEmailParse(char *message);

#endif