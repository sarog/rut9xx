#include <common.h>

#ifdef CONFIG_MNFINFO_SUPPORT

#include <mnf_info.h>
#include <linux/ctype.h>
#include <linux/time.h>

#define CONFIG_MAC_VID_STR "001E42"

inline static int validate_num(mnf_field_t *field, const char *old)
{
	for (int i = 0; i < field->length; i++) {
		if (!isdigit(old[i]))
			return 1;
	}
	return 0;
}

/* Set to mac random mac address with our VID, skip if mac already has our VID */
static int restore_mac(mnf_field_t *field, const char *old, char *buf)
{
	unsigned nid;

	// using srand() doesn't work here, for whatever reason...
	unsigned seed = get_timer(0);
	nid = rand_r(&seed) & 0xffffff;

	sprintf(buf, "%s%06x", CONFIG_MAC_VID_STR, nid);

	for (int i = 0; i < strlen(buf); i++) {
		buf[i] = toupper(buf[i]);
	}

	return strncmp(old, CONFIG_MAC_VID_STR, strlen(CONFIG_MAC_VID_STR)) != 0;
}

/* Set to fixed fallback device, skip if field contains valid model number */
static int restore_name(mnf_field_t *field, const char *old, char *buf)
{
	memset(buf, '0', field->length);
	buf[field->length] = '\0';
	memcpy(buf, DEVICE_MODEL_NAME, strlen(DEVICE_MODEL_NAME));

	return strncmp(old, DEVICE_MODEL_NAME, strlen(DEVICE_MODEL_NAME));
}

/* Clear the field with 0's, skip if field contains only digits */
static int restore_num(mnf_field_t *field, const char *old, char *buf)
{
	memset(buf, '0', field->length);
	buf[field->length] = '\0';

	return validate_num(field, old);
}

/* Clear the field with 0xff's, don't overwrite by default */
static int clear(mnf_field_t *field, const char *old, char *buf)
{
	strcpy(buf, "");

	return 0;
}

mnf_field_t mnf_fields[] = {
//	short/long name,     description,        offset, len, restore func   , flags
	MNF_FIELD( 'm', "mac",       "MAC address",         0x00,   6, restore_mac    , MNF_FIELD_BINARY ),
	MNF_FIELD( 'n', "name",      "Model name",          0x10,  12, restore_name   , 0 ),
	MNF_FIELD( 'w', "wps",       "WPS PIN",             0x20,   8, clear          , 0 ),
	MNF_FIELD( 's', "sn",        "Serial number",       0x30,  10, restore_num    , 0 ),
	MNF_FIELD( 'b', "batch",     "Batch number",        0x40,   4, restore_num    , 0 ),
	MNF_FIELD( 'H', "hwver",     "Hardvare version",    0x50,   4, restore_num    , 0 ),

#ifdef CONFIG_FOR_TELTONIKA_RUT2XX
	MNF_FIELD( '1', "sim1",      "SIM 1 PIN",           0x68,  12, clear          , MNF_FIELD_REVERSED ),
#else
	MNF_FIELD( '1', "sim1",      "SIM 1 PIN",           0x70,   8, clear          , 0 ),
#endif

#if defined(CONFIG_FOR_TELTONIKA_RUT9XX) || \
    defined(CONFIG_FOR_TELTONIKA_RUT952) || \
    defined(CONFIG_FOR_TELTONIKA_RUT9M)
	MNF_FIELD( '2', "sim2",      "SIM 2 PIN",           0x78,   8, clear          , 0 ),
#elif defined (CONFIG_FOR_TELTONIKA_TRB24XX)
	MNF_FIELD( '2', "sim2",      "SIM 2 PIN",           0x80,   4, clear          , 0 ),
#endif

	MNF_FIELD( 'W', "wifi_pass", "WiFi password",       0x90,  16, clear          , 0 ),
	MNF_FIELD( 'x', "passwd",    "Linux password",      0xA0, 106, clear          , 0 ),
	{ '\0' }
};

#endif