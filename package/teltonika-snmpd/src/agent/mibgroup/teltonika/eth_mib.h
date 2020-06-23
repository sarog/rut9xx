#ifndef _MIBGROUP_ETH_MIB_H
#define _MIBGROUP_ETH_MIB_H

config_require(util_funcs)

void init_eth_mib(void);
extern FindVarMethod get_eth_port_status;

#define PORT1 1
#define PORT2 2
#define PORT3 3
#define PORT4 4
#define ETH_STRING_LEN 256
#define CMDSIZE 256

#endif
