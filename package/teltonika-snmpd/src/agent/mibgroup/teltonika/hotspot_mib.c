/*
 *  HOTSPOT MIB group implementation - hotspot.c
 *
 */

#include <net-snmp/net-snmp-config.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"
#include "hotspot_mib.h"
#include "struct.h"

#define HOTSPOT_STRING_LEN	1024

char search_result[HOTSPOT_STRING_LEN];
int i;
int header_hotspot(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);


/*
 * define the structure we're going to ask the agent to register our
 * information at
 */
struct variable1 hotspot1_variables[] = {
    {HOTSPOTID,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {1}},
    {HOTSPOTSSID,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {2}},
    {HOTSPOTENABLED,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {3}},
    {HOTSPOTIP,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {4}},
    {HOTSPOTDOWNLOAD,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {5}},
    {HOTSPOTUPLOAD,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {6}},
    {HOTSPOTUSERS,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {7}},
    {HOTSPOTUSERSPASS,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {8}},
    {HOTSPOTUSERSACTIVE,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {9}},
    {HOTSPOTUSERMAC,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {10}},
    {HOTSPOTUSERIP,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {11}},
    {HOTSPOTUSERSTARTTIME,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {12}},
    {HOTSPOTUSERUSETIME,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {13}},
    {HOTSPOTUSERDOWNLOAD,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {14}},
    {HOTSPOTUSERUPLOAD,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {15}},
    {HOTSPOTENDTIME,	ASN_OCTET_STR, RONLY, var_hotspot1, 1, {16}}
};
struct variable2 hotspot2_variables[] = {
    {HOTSPOTID,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {1}},
    {HOTSPOTSSID,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {2}},
    {HOTSPOTENABLED,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {3}},
    {HOTSPOTIP,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {4}},
    {HOTSPOTDOWNLOAD,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {5}},
    {HOTSPOTUPLOAD,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {6}},
    {HOTSPOTUSERS,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {7}},
    {HOTSPOTUSERSPASS,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {8}},
    {HOTSPOTUSERSACTIVE,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {9}},
    {HOTSPOTUSERMAC,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {10}},
    {HOTSPOTUSERIP,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {11}},
    {HOTSPOTUSERSTARTTIME,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {12}},
    {HOTSPOTUSERUSETIME,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {13}},
    {HOTSPOTUSERDOWNLOAD,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {14}},
    {HOTSPOTUSERUPLOAD,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {15}},
    {HOTSPOTENDTIME,	ASN_OCTET_STR, RONLY, var_hotspot2, 1, {16}}

};
struct variable3 hotspot3_variables[] = {
    {HOTSPOTID,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {1}},
    {HOTSPOTSSID,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {2}},
    {HOTSPOTENABLED,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {3}},
    {HOTSPOTIP,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {4}},
    {HOTSPOTDOWNLOAD,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {5}},
    {HOTSPOTUPLOAD,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {6}},
    {HOTSPOTUSERS,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {7}},
    {HOTSPOTUSERSPASS,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {8}},
    {HOTSPOTUSERSACTIVE,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {9}},
    {HOTSPOTUSERMAC,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {10}},
    {HOTSPOTUSERIP,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {11}},
    {HOTSPOTUSERSTARTTIME,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {12}},
    {HOTSPOTUSERUSETIME,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {13}},
    {HOTSPOTUSERDOWNLOAD,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {14}},
    {HOTSPOTUSERUPLOAD,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {15}},
    {HOTSPOTENDTIME,	ASN_OCTET_STR, RONLY, var_hotspot3, 1, {16}}
};
struct variable4 hotspot4_variables[] = {
    {HOTSPOTID,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {1}},
    {HOTSPOTSSID,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {2}},
    {HOTSPOTENABLED,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {3}},
    {HOTSPOTIP,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {4}},
    {HOTSPOTDOWNLOAD,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {5}},
    {HOTSPOTUPLOAD,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {6}},
    {HOTSPOTUSERS,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {7}},
    {HOTSPOTUSERSPASS,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {8}},
    {HOTSPOTUSERSACTIVE,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {9}},
    {HOTSPOTUSERMAC,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {10}},
    {HOTSPOTUSERIP,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {11}},
    {HOTSPOTUSERSTARTTIME,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {12}},
    {HOTSPOTUSERUSETIME,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {13}},
    {HOTSPOTUSERDOWNLOAD,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {14}},
    {HOTSPOTUSERUPLOAD,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {15}},
    {HOTSPOTENDTIME,	ASN_OCTET_STR, RONLY, var_hotspot4, 1, {16}}
};

/*
 * Define the OID pointer to the Teltonika stolen ID
 */
oid hotspot1_variables_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 3, 1};
oid hotspot2_variables_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 3, 2};
oid hotspot3_variables_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 3, 3};
oid hotspot4_variables_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 3, 4};
/*
 * Wrapper function to get internal params
 *
 */
int get_params(char *param, char *buffer, int use, int iface_index)
{
	char cmd[HOTSPOT_STRING_LEN];
	FILE* name = NULL;
	int iLength;
	if (use == 0)
		sprintf(cmd, "uci get -q wireless.@wifi-iface[%d].%s 2>/dev/null", iface_index, param);
	else if (use == 1)
		sprintf(cmd, "uci get -q coovachilli.hotspot%d.%s 2>/dev/null", iface_index, param);
	else if (use == 2)
		sprintf(cmd, "awk -F':' '{print $1}' ORS=' ' /etc/chilli/%s/localusers 2>/dev/null", param);
	else if (use == 3)
		sprintf(cmd, "lua /usr/sbin/hotspot_user_info.lua %s %d 2>/dev/null", param, iface_index);
	else if (use == 4)
		sprintf(cmd, "awk -F':' '{print $2}' ORS=' ' /etc/chilli/%s/localusers 2>/dev/null", param);
	else if (use == 5)
		sprintf(cmd, "uci get -q coovachilli.unlimited%d.%s 2>/dev/null", iface_index, param);

	if ((name = popen(cmd, "r")) == NULL)
		return -1;

	if (fgets(buffer, HOTSPOT_STRING_LEN, name) == NULL) {
		pclose(name);
		return -1;
	} else {
		iLength = strlen(buffer);
		// trim 'new line' symbol at the end
		if (iLength && buffer[iLength-1] == '\n')
			buffer[iLength-1] = '\0';
	}
	pclose(name);
	return 0;
}

void init_hotspot_mib(void)
{
	/* We might initialize our vars here */

	/*
	 * register ourselves with the agent to handle our mib tree
	 */
	REGISTER_MIB("teltonika", hotspot1_variables, variable1, hotspot1_variables_oid);
	REGISTER_MIB("teltonika", hotspot2_variables, variable2, hotspot2_variables_oid);
	REGISTER_MIB("teltonika", hotspot3_variables, variable3, hotspot3_variables_oid);
	REGISTER_MIB("teltonika", hotspot4_variables, variable4, hotspot4_variables_oid);
}

u_char* var_hotspot1(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
	if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	switch (vp->magic) {

	case HOTSPOTID:
		if (get_params("hotspotid", search_result, 0, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTSSID:
		if (get_params("ssid", search_result, 0, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTENABLED:
		if (get_params("enabled", search_result, 1, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTIP:
		if (get_params("net", search_result, 1, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTDOWNLOAD:
		if (get_params("downloadbandwidth", search_result, 5, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTUPLOAD:
		if (get_params("uploadbandwidth", search_result, 5, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERS:
		if (get_params("wlan0", search_result, 2, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSPASS:
		if (get_params("wlan0", search_result, 4, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSACTIVE:
		if (get_params("1", search_result, 3, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERMAC:
		if (get_params("3", search_result, 3, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERIP:
		if (get_params("2", search_result, 3, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSTARTTIME:
		if (get_params("4", search_result, 3, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERUSETIME:
		if (get_params("5", search_result, 3, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERDOWNLOAD:
		if (get_params("6", search_result, 3, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERUPLOAD:
		if (get_params("7", search_result, 3, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTENDTIME:
		if (get_params("8", search_result, 3, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	default:
		DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_hotspot\n", vp->magic));
  }

  return NULL;
}
u_char* var_hotspot2(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
	if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	switch (vp->magic) {

	case HOTSPOTID:
		if (get_params("hotspotid", search_result, 0, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTSSID:
		if (get_params("ssid", search_result, 0, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTENABLED:
		if (get_params("enabled", search_result, 1, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTIP:
		if (get_params("net", search_result, 1, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTDOWNLOAD:
		if (get_params("downloadbandwidth", search_result, 1, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTUPLOAD:
		if (get_params("uploadbandwidth", search_result, 1, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERS:
		if (get_params("wlan0-1", search_result, 2, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSPASS:
		if (get_params("wlan0-1", search_result, 4, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSACTIVE:
		if (get_params("1", search_result, 3, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERMAC:
		if (get_params("3", search_result, 3, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERIP:
		if (get_params("2", search_result, 3, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSTARTTIME:
		if (get_params("4", search_result, 3, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERUSETIME:
		if (get_params("5", search_result, 3, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERDOWNLOAD:
		if (get_params("6", search_result, 3, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERUPLOAD:
		if (get_params("7", search_result, 3, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTENDTIME:
		if (get_params("8", search_result, 3, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	default:
		DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_hotspot\n", vp->magic));
  }

  return NULL;
}
u_char* var_hotspot3(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
	if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	switch (vp->magic) {

	case HOTSPOTID:
		if (get_params("hotspotid", search_result, 0, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTSSID:
		if (get_params("ssid", search_result, 0, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTENABLED:
		if (get_params("enabled", search_result, 1, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTIP:
		if (get_params("net", search_result, 1, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTDOWNLOAD:
		if (get_params("downloadbandwidth", search_result, 1, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTUPLOAD:
		if (get_params("uploadbandwidth", search_result, 1, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERS:
		if (get_params("wlan0-2", search_result, 2, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSPASS:
		if (get_params("wlan0-2", search_result, 4, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSACTIVE:
		if (get_params("1", search_result, 3, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERMAC:
		if (get_params("3", search_result, 3, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERIP:
		if (get_params("2", search_result, 3, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSTARTTIME:
		if (get_params("4", search_result, 3, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERUSETIME:
		if (get_params("5", search_result, 3, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERDOWNLOAD:
		if (get_params("6", search_result, 3, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERUPLOAD:
		if (get_params("7", search_result, 3, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTENDTIME:
		if (get_params("8", search_result, 3, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	default:
		DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_hotspot\n", vp->magic));
  }

  return NULL;
}
u_char* var_hotspot4(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
	if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	switch (vp->magic) {

	case HOTSPOTID:
		if (get_params("hotspotid", search_result, 0, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTSSID:
		if (get_params("ssid", search_result, 0, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTENABLED:
		if (get_params("enabled", search_result, 1, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTIP:
		if (get_params("net", search_result, 1, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTDOWNLOAD:
		if (get_params("downloadbandwidth", search_result, 1, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case HOTSPOTUPLOAD:
		if (get_params("uploadbandwidth", search_result, 1, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERS:
		if (get_params("wlan0-3", search_result, 2, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSPASS:
		if (get_params("wlan0-3", search_result, 4, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSACTIVE:
		if (get_params("1", search_result, 3, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERMAC:
		if (get_params("3", search_result, 3, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERIP:
		if (get_params("2", search_result, 3, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERSTARTTIME:
		if (get_params("4", search_result, 3, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERUSETIME:
		if (get_params("5", search_result, 3, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERDOWNLOAD:
		if (get_params("6", search_result, 3, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTUSERUPLOAD:
		if (get_params("7", search_result, 3, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HOTSPOTENDTIME:
		if (get_params("8", search_result, 3, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	default:
		DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_hotspot\n", vp->magic));
  }

  return NULL;
}
