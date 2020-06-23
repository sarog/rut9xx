/* 
 * Ethernet port status MIB implementation
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include "util_funcs.h"
#include "struct.h"
#include "eth_mib.h"

struct variable1 eth_variables[] = {
	{ PORT1, ASN_OCTET_STR, RONLY, get_eth_port_status, 1, { 4 } },
	{ PORT2, ASN_OCTET_STR, RONLY, get_eth_port_status, 1, { 3 } },
	{ PORT3, ASN_OCTET_STR, RONLY, get_eth_port_status, 1, { 2 } },
	{ PORT4, ASN_OCTET_STR, RONLY, get_eth_port_status, 1, { 1 } }
};

oid eth_variables_oid[] = { 1, 3, 6, 1, 4, 1, 48690, 7 };

void init_eth_mib(void) {
	REGISTER_MIB("teltonika", eth_variables, variable1, eth_variables_oid);
}

int get_status(int port, char *buffer) {
	int bufsize = 0;
	char cmd[256];
	FILE *name = NULL;
	if ( port == 1 ) {
		snprintf(cmd, CMDSIZE, "cat /sys/class/net/eth1/operstate");
	} else {
		snprintf(cmd, CMDSIZE, "/sbin/swconfig dev switch0 show \
			| grep \"port:%d\" \
			| cut -d':' -f4 \
			| cut -d' ' -f1", port);
	}
	if ( (name = popen(cmd, "r")) == NULL ) {
		return errno;
	}
	if ( fgets(buffer, ETH_STRING_LEN, name) == NULL ) {
		pclose(name);
		return errno;
	} else {
		bufsize = strlen(buffer);
		if ( bufsize && buffer[bufsize - 1] == '\n' ) {
			buffer[bufsize - 1] = '\0';
		}
	}
	pclose(name);
	return 0;
}

u_char* get_eth_port_status(struct variable *vp, oid *name, size_t *length, int exact, size_t *var_len, WriteMethod **write_method) {
	char result[ETH_STRING_LEN];
	int port_number;
	if ( header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED ) {
		return NULL;
	}
	port_number = vp->magic;
	if ( get_status(port_number, result) != 0 ) {
		strncpy(result, "unknown", sizeof("unknown"));
	}
	*var_len = strlen(result);
	*write_method = 0;
	return (u_char *) result;
}
