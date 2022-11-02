#ifndef __FIREWALL_H
#define __FIREWALL_H

#define FW_PREROUTING                                                          \
	"PREROUTING -t mangle -i %s -m quota2 --name rx_%s --quota 0 --grow"
#define FW_POSTROUTING                                                         \
	"POSTROUTING -t mangle -o %s -m quota2 --name tx_%s --quota 0 --grow"

void set_firewall_rules(const char *iface);
void del_firewall_rules(const char *iface);

#endif //__FIREWALL_H