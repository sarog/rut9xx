#ifndef NTPCLIENT_H
#define NTPCLIENT_H

#ifndef NTP_MAX_SERVERS
#define NTP_MAX_SERVERS (1)
#endif

struct ntptime {
	unsigned int coarse;
	unsigned int fine;
};

struct ntp_control {
	uint32_t time_of_send[2];
	short int udp_local_port;
    int probes_sent;
    int modem_sync_flg;
	int live;
	int set_clock; /* non-zero presumably needs root privs */
	int probe_count;
	int failover_cnt;
	int cycle_time;
	int retry_interval;
	int goodness;
	int cross_check;
	int hosts;
	char *hostnames[NTP_MAX_SERVERS];
};

/* global tuning parameter */
extern double min_delay;

#endif
