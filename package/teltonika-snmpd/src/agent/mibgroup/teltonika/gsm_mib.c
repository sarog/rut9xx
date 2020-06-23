/*
 *  GSM MIB group implementation - gsm.c
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
#include "gsm_mib.h"
#include "struct.h"

#define GSM_STRING_LEN	256

char search_result[GSM_STRING_LEN];

int header_gsm(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);

/*
 * define the structure we're going to ask the agent to register our
 * information at
 */

struct variable1 gsm_variables1[] = {
	{IMEI,		ASN_OCTET_STR, RONLY, var_gsm, 1, {1}},
	{MODEL,		ASN_OCTET_STR, RONLY, var_gsm, 1, {2}},
	{MANUF,		ASN_OCTET_STR, RONLY, var_gsm, 1, {3}},
	{REVISION,	ASN_OCTET_STR, RONLY, var_gsm, 1, {4}},
	{SERIAL,	ASN_OCTET_STR, RONLY, var_gsm, 1, {5}},
	{IMSI,		ASN_OCTET_STR, RONLY, var_gsm, 1, {6}},
	{ROUTERNAME,ASN_OCTET_STR, RONLY, var_gsm, 1, {7}},
	{PRODUCTCODE,		ASN_OCTET_STR, RONLY, var_gsm, 1, {8}},
	{BATCHNUMBER,		ASN_OCTET_STR, RONLY, var_gsm, 1, {9}},
	{HARDWAREREVISION,	ASN_OCTET_STR, RONLY, var_gsm, 1, {10}}
};
struct variable1 gsm_variables2[] = {
	{SIMSTATE,	ASN_OCTET_STR, RONLY, var_gsm, 1, {1}},
	{PINSTATE,	ASN_OCTET_STR, RONLY, var_gsm, 1, {2}},
	{NETSTATE,	ASN_OCTET_STR, RONLY, var_gsm, 1, {3}},
	{SIGNAL,	ASN_INTEGER, RONLY, var_gsm, 1, {4}},
	{OPERATOR,	ASN_OCTET_STR, RONLY, var_gsm, 1, {5}},
	{OPERNUM,	ASN_OCTET_STR, RONLY, var_gsm, 1, {6}},
	{CONNSTATE,	ASN_OCTET_STR, RONLY, var_gsm, 1, {7}},
	{CONNTYPE,	ASN_OCTET_STR, RONLY, var_gsm, 1, {8}},
	{TEMP,		ASN_INTEGER, RONLY, var_gsm, 1, {9}},
	{RXCOUNTT,	ASN_INTEGER, RONLY, var_gsm, 1, {10}},
	{TXCOUNTT,	ASN_INTEGER, RONLY, var_gsm, 1, {11}},
	{RXCOUNTY,	ASN_INTEGER, RONLY, var_gsm, 1, {12}},
	{TXCOUNTY,	ASN_INTEGER, RONLY, var_gsm, 1, {13}},
	{FWVERSION,	ASN_OCTET_STR, RONLY, var_gsm, 1, {14}},
	{SIMSLOT,	ASN_OCTET_STR, RONLY, var_gsm, 1, {15}},
	{ROUTERUPTIME,		ASN_INTEGER, RONLY, var_gsm, 1, {16}},
	{CONNECTIONUPTIME,	ASN_INTEGER, RONLY, var_gsm, 1, {17}},
	{MOBILEIP,	ASN_OCTET_STR, RONLY, var_gsm, 1, {18}},
	{SENT,		ASN_INTEGER, RONLY, var_gsm, 1, {19}},
	{RECEIVED,	ASN_INTEGER, RONLY, var_gsm, 1, {20}},
	{CELLID,	ASN_OCTET_STR, RONLY, var_gsm, 1, {21}},
	{SINR,	ASN_OCTET_STR, RONLY, var_gsm, 1, {22}},
	{RSRP,	ASN_OCTET_STR, RONLY, var_gsm, 1, {23}},
	{RSRQ,	ASN_OCTET_STR, RONLY, var_gsm, 1, {24}}
};

struct variable1 gsm_variables3[] = {
	{DIGITALINPUT,			ASN_OCTET_STR, RONLY, var_gsm, 1, {1}},
	{DIGITALISOLATEDINPUT,	ASN_OCTET_STR, RONLY, var_gsm, 1, {2}},
	{ANALOGINPUT,			ASN_OCTET_STR, RONLY, var_gsm, 1, {3}},
	{DIGITALOCOUTPUT,		ASN_OCTET_STR, RONLY, var_gsm, 1, {4}},
	{DIGITALRELAYOUTPUT,	ASN_OCTET_STR, RONLY, var_gsm, 1, {5}},
	{ANALOGINPUTCALC,			ASN_OCTET_STR, RONLY, var_gsm, 1, {6}}
};
/*
 * Define the OID pointer to the Teltonika ID
 */
oid gsm_variables_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 1};
oid gsm_variables_oid2[] = { 1, 3, 6, 1, 4, 1, 48690, 2};
oid gsm_variables_oid3[] = { 1, 3, 6, 1, 4, 1, 48690, 5};
/*
 * Wrapper function to get internal params
 *
 */
int get_param(char *param, char *buffer, int use)
{
	char cmd[GSM_STRING_LEN];
	FILE* name = NULL;
	int iLength;

	if (use == 0)
		sprintf(cmd, "/usr/sbin/sysget %s 2>/dev/null", param);
	else if (use == 1)
		sprintf(cmd, "/usr/bin/mdcget %s 2>/dev/null", param);
	else if (use == 2)
		sprintf(cmd, "/bin/cat %s 2>/dev/null", param);
	else if (use == 3)
		sprintf(cmd, "/sbin/gpio.sh get  %s 2>/dev/null", param);
	else if (use == 4)
		sprintf(cmd, "ubus %s 2>/dev/null", param);
	else if (use == 5)
		sprintf(cmd, "gsmctl -n %s 2>/dev/null", param);
	else if (use == 6)
		sprintf(cmd, "gsmctl %s 2>/dev/null", param);
	else if (use == 7)
		sprintf(cmd, "analog_calc 2>/dev/null");
	else if (use == 8)
		sprintf(cmd, "/sbin/mnf_info %s 2>/dev/null", param);

	if ((name = popen(cmd, "r")) == NULL)
		return -1;

	if (fgets(buffer, GSM_STRING_LEN, name) == NULL) {
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

void init_gsm_mib(void)
{
	/* We might initialize our vars here */

	/*
	 * register ourselves with the agent to handle our mib tree
	 */
	REGISTER_MIB("tlt", gsm_variables1, variable1, gsm_variables_oid);
	REGISTER_MIB("tlt", gsm_variables2, variable1, gsm_variables_oid2);
	REGISTER_MIB("tlt", gsm_variables3, variable1, gsm_variables_oid3);
}

u_char* var_gsm(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
	static long count=0;

	if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	switch (vp->magic) {

	case IMEI:
		if (get_param("--imei", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case MODEL:
		if (get_param("--model", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case MANUF:
		if (get_param("--manuf", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case REVISION:
		if (get_param("--revision", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case SERIAL:
		if (get_param("sn", search_result, 8) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
	case SIMSTATE:
		if (get_param("--simstate", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case PINSTATE:
		if (get_param("--pinstate", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case IMSI:
		if (get_param("--imsi", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
	case NETSTATE:
		if (get_param("--netstate", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case SIGNAL:
		if (get_param("--signal", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//*var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case OPERATOR:
		if (get_param("--operator", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case OPERNUM:
		if (get_param("--opernum", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case CONNSTATE:
		if (get_param("--connstate", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case CONNTYPE:
		if (get_param("--conntype", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case TEMP:
		if (get_param("--temp", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//*var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case RXCOUNTT:
		if (get_param("rx", search_result, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//~ *var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case TXCOUNTT:
		if (get_param("tx", search_result, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//~ *var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case RXCOUNTY:
		if (get_param("-y rx", search_result, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//~ *var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case TXCOUNTY:
		if (get_param("-y tx", search_result, 1) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//~ *var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case FWVERSION:
		if (get_param("/etc/version", search_result, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case DIGITALINPUT:
		if (get_param("DIN1", search_result, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		if (strncmp(search_result, "0", sizeof("0")) == 0)
			sprintf(search_result, "Inactive");
		else
			sprintf(search_result, "Active");
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case DIGITALISOLATEDINPUT:
		if (get_param("DIN2", search_result, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		if (strncmp(search_result, "0", sizeof("0")) == 0)
			sprintf(search_result, "Inactive");
		else
			sprintf(search_result, "Active");
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case ANALOGINPUT:
		if (get_param("/sys/class/hwmon/hwmon0/device/in0_input", search_result, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		double result;
		result = atof(search_result)/1000;
		sprintf(search_result, "%2.2f", result);

		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case DIGITALOCOUTPUT:
		if (get_param("DOUT1", search_result, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		if (strncmp(search_result, "0", sizeof("0")) == 0)
			sprintf(search_result, "Inactive");
		else
			sprintf(search_result, "Active");
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case DIGITALRELAYOUTPUT:
		if (get_param("DOUT2", search_result, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		if (strncmp(search_result, "0", sizeof("0")) == 0)
			sprintf(search_result, "Inactive");
		else
			sprintf(search_result, "Active");
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case SIMSLOT:
		if (get_param("SIM", search_result, 3) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case ROUTERNAME:
		if (get_param("/etc/config/system | grep -r routername | awk -F \"[']\" '{print $2}'", search_result, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case PRODUCTCODE:
		if (get_param("/etc/config/hwinfo | grep -r 'mnf_code' | awk -F \"[']\" '{print $2}'", search_result, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case BATCHNUMBER:
		if (get_param("/etc/config/hwinfo | grep -r 'batch' | awk -F \"[']\" '{print $2}'", search_result, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case HARDWAREREVISION:
		if (get_param("/etc/config/hwinfo | grep -r 'hwver' | awk -F \"[']\" '{print $2}'", search_result, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case ROUTERUPTIME:
		if (get_param("/proc/uptime | awk -F'.' '{print $1}'", search_result, 2) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//*var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case CONNECTIONUPTIME:
		if (get_param("call network.interface.wan status | grep uptime | awk '{print $2}' | tr ',' ' '", search_result, 4) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//*var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case MOBILEIP:
		if (get_param("-p $(uci get network.ppp.ifname 2>&1)", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case SENT:
		if (get_param("-e $(uci get network.ppp.ifname 2>&1)", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//~ *var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	case RECEIVED:
		if (get_param("-r $(uci get network.ppp.ifname 2>&1)", search_result, 5) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//~ *var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) & count;

	case CELLID:
		if (get_param("--cellid", search_result, 6) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case SINR:
		if (get_param("--sinr", search_result, 6) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case RSRP:
		if (get_param("--rsrp", search_result, 6) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case RSRQ:
		if (get_param("--rsrq", search_result, 6) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	case ANALOGINPUTCALC:
		if (get_param("", search_result, 7) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;

	default:
		DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_gsm\n", vp->magic));
  }

  return NULL;
}
