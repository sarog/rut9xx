#pragma once

#include <stdint.h>
#include <stddef.h>
#include <libgsm_modem.h>

#define SMS_ARRAY_SIZE 10

// SMS structure.
typedef struct {
	uint8_t SMSC_Length;
	uint8_t SMSC_Type;
	char SMSC_Number[32];
	int FO_SMSD;
	uint8_t AddrLength;
	uint8_t AddrType;
	char Sender[32];        // sms sender
	uint8_t TP_PID;
	uint8_t TP_DCS;
	char TimeStamp[24];     // sms time
	uint8_t DataLength;
	uint8_t DataheaderLength;
	char Data[0xff];         // sms text
} SMS_DATA;

// SMS structure full.
typedef struct {
	int Index;              // sms Index
	int Status;     // sms Status
	SMS_DATA stru_for_pdu;
} ONE_SMS;

// SMS structure with element number in it.
typedef struct {
	int num;
	int err;
	ONE_SMS  *list;
} SMS_LIST;

// SMS functions.
int message_parser(char *p_buffer, SMS_DATA *p_sms);
int utf8_to_8859_converter(unsigned char *buffer, char *output, int sms_num, int sms_length);
unsigned char char_converter(unsigned int letter);
int create_pdu(char *number, char *text, size_t unicode, char *pdu, char nr_type[3], int all_sms, int sms_num, int random, int sms_length);
SMS_LIST gsmctl_get_sms_list_struct(modem_dev device, int index, char *modem_id);
int gsmctl_get_sms(modem_dev device, int index, char **output, SMS_DATA *sms_struct, char *modem_id);
int gsmctl_get_sms_list(modem_dev device, char *list, char **output, char *modem_id);
int gsmctl_get_sms_memory(modem_dev device, char *output, char *modem_id);
int gsmctl_send_sms(modem_dev device, const char *number, const char *text, char *output, const char *modem_id);
int gsmctl_send_sms_limited(modem_dev device, const char *number, const char *text, unsigned max_multipart_sms, char *output, const char *modem_id);
int gsmctl_send_sms_pdu(char *pdu, char *output, char *modem_id);
int gsmctl_delete_sms(modem_dev device, int index, char *output, char *modem_id);

//base64 decoder
int decode_base64(unsigned char *dest, const char *src);

void log_sms_event(char *text, char *number, modem_type *type);
