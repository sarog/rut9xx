/*
 *  GPS MIB group implementation - gps.c
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
#include "gps_mib.h"
#include "struct.h"


#define GPS_STRING_LEN	256

char search_result[GPS_STRING_LEN];

int header_gps(struct variable *, oid *, size_t *, int, size_t *, WriteMethod **);

/*
 * define the structure we're going to ask the agent to register our
 * information at
 */

struct variable1 gps_variables[] = {
	{LONGITUDE,		ASN_OCTET_STR, RONLY, var_gps, 1, {1}},
	{LATITUDE,		ASN_OCTET_STR, RONLY, var_gps, 1, {2}},
	{ACCURACY,		ASN_OCTET_STR, RONLY, var_gps, 1, {3}},
	{DATETIME,		ASN_OCTET_STR, RONLY, var_gps, 1, {4}},
	{NUMSAT,		ASN_INTEGER, RONLY, var_gps, 1, {5}}
};

/*
 * Define the OID pointer to the Teltonika stolen ID
 */
oid gps_variables_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 6};
/*
 * Wrapper function to get internal params
 *
 */
int get_parameter(char *param, char *buffer, int use)
{
	char cmd[GPS_STRING_LEN];
	FILE* name = NULL;
	int iLength;

	if (use == 0)
		sprintf(cmd, "gpsctl %s 2>/dev/null", param);

	if ((name = popen(cmd, "r")) == NULL)
		return -1;

	if (fgets(buffer, GPS_STRING_LEN, name) == NULL) {
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

void init_gps_mib(void)
{
	/* We might initialize our vars here */

	/*
	 * register ourselves with the agent to handle our mib tree
	 */
	REGISTER_MIB("teltonika", gps_variables, variable1, gps_variables_oid);
}

u_char* var_gps(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method)
{
	static long count=0;

	if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	switch (vp->magic) {
		
	case LONGITUDE:
		if (get_parameter("--longitude", search_result, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case LATITUDE:
		if (get_parameter("--latitude", search_result, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case ACCURACY:
		if (get_parameter("--accuracy", search_result, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
		
	case DATETIME:
		if (get_parameter("--datetime", search_result, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		*var_len = strlen(search_result);
		*write_method = 0;
		return (u_char *) search_result;
	
	case NUMSAT:
		if (get_parameter("--satellites", search_result, 0) != 0)
			strncpy(search_result, "unknown", sizeof("unknown"));
		//*var_len = strlen(search_result);
		*write_method = 0;
		count=atol(search_result);
		return (u_char *) &count;

	default:
		DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_gps\n", vp->magic));
  }

  return NULL;
}
