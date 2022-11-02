
#ifndef RUTX_GSM_H
#define RUTX_GSM_H

#include <libgsm.h>

#define GSM_DEFAULT_USB_ID "3-1"

int gsm_send_sms(char *phone, char *msg, char *modem_id);
void gsm_send_sms_async(char *phone, char *msg);

#endif //RUTX_GSM_H

