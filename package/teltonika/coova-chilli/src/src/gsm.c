
#include "gsm.h"

int gsm_send_sms(char *phone, char *msg, char *modem_id)
{
	char buffer[256] = {0};
	modem_dev device;

	device = get_modem(modem_id);
	gsmctl_send_sms(device, phone, msg, buffer, modem_id);

	if (!strstr(buffer, "OK"))
		return 1;

	return 0;
}

void gsm_send_sms_async(char *phone, char *msg)
{
	char cmd[256] = { 0 };

	sprintf(cmd, "gsmctl --sms --send \"%s %s\" &", phone, msg);
	system(cmd);
}
