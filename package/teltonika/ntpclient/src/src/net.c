#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h> /* gethostbyname */
#include "debug.h"
#include "log.h"

static int stuff_net_addr(struct in_addr *p, char *hostname)
{
	int ret = 0;
	struct hostent *ntpserver;
	ntpserver = gethostbyname(hostname);
	if (ntpserver == NULL) {
		ERR("Could not resolve %s\n", hostname);
		ret = -1;
		goto end;
	}
	if (ntpserver->h_length != 4) {
		/* IPv4 only, until I get a chance to test IPv6 */
		ERR("oops %d\n", ntpserver->h_length);
		ret = -1;
		goto end;
	}
	memcpy(&(p->s_addr), ntpserver->h_addr_list[0], 4);
end:
	return ret;
}

static int setup_receive(int usd, unsigned int inaddr, short port)
{
	struct sockaddr_in sa_rcvr;
	int ret = 0;

	memset(&sa_rcvr, 0, sizeof sa_rcvr);
	sa_rcvr.sin_family	= AF_INET;
	sa_rcvr.sin_addr.s_addr = htonl(inaddr);
	sa_rcvr.sin_port	= htons(port);
	if (bind(usd, (struct sockaddr *)&sa_rcvr, sizeof sa_rcvr) == -1) {
		ERR("Could not bind to udp port %d\n", port);
		ret = -1;
		goto end;
	}
end:
	return ret;
}

static int setup_transmit(int usd, char *host, short port)
{
	struct sockaddr_in sa_dest;
	int ret = 0;

	memset(&sa_dest, 0, sizeof sa_dest);
	sa_dest.sin_family = AF_INET;
	if (stuff_net_addr(&(sa_dest.sin_addr), host) != 0) {
		ret = -1;
		goto end;
	}

	sa_dest.sin_port = htons(port);
	if (connect(usd, (struct sockaddr *)&sa_dest, sizeof sa_dest) == -1) {
		ERR("Could not connect to %s\n", host);
		ret = -1;
		goto end;
	}
end:
	return ret;
}

int net_setup(char *host, short port, short local_port)
{
	int usd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (usd  == -1) {
		ERR("Could not create socket\n");
		goto end;
	}
	if (setup_receive(usd, INADDR_ANY, local_port)) {
		usd = -1;
		goto end;
	}
	if (setup_transmit(usd, host, port)) {
		usd = -1;
		goto end;
	}
end:
	return usd;
}
