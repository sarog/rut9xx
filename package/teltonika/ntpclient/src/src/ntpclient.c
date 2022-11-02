/*
 * ntpclient.c - NTP client
 *
 * Copyright (C) 1997, 1999, 2000, 2003, 2006, 2007, 2010, 2015  Larry Doolittle  <larry@doolittle.boa.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License (Version 2,
 *  June 1991) as published by the Free Software Foundation.  At the
 *  time of writing, that license was published by the FSF with the URL
 *  http://www.gnu.org/copyleft/gpl.html, and is incorporated herein by
 *  reference.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Possible future improvements:
 *      - Write more documentation  :-(
 *      - Support leap second processing
 *      - Support IPv6
 *      - Support multiple (interleaved) servers
 *
 *  Compile with -DPRECISION_SIOCGSTAMP if your machine really has it.
 *  Older kernels (before the tickless era, pre 3.0?) only give an answer
 *  to the nearest jiffy (1/100 second), not so interesting for us.
 *
 *  If the compile gives you any flak, check below in the section
 *  labelled "XXX fixme - non-automatic build configuration".
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> /* gethostbyname */
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#ifdef PRECISION_SIOCGSTAMP
#include <sys/ioctl.h>
#ifdef __GLIBC__
#include <linux/sockios.h>
#endif
#endif
#ifdef USE_OBSOLETE_GETTIMEOFDAY
#include <sys/time.h>
#endif
#include <libubus.h>
#include <libubox/blob.h>

#include "phaselock.h"
#include "ntpclient.h"
#include "net.h"
#include "debug.h"
#include "log.h"

/* Default to the RFC-4330 specified value */
#ifndef MIN_INTERVAL
#define MIN_INTERVAL (60)
#endif

#define RETRY_INTERVAL	(60)

#ifdef ENABLE_DEBUG
#define DEBUG_OPTION "d"
int g_debug = 0;
#else
#define DEBUG_OPTION
#endif

#ifdef ENABLE_REPLAY
#define REPLAY_OPTION "r"
#else
#define REPLAY_OPTION
#endif

#include <stdint.h>

/* XXX fixme - non-automatic build configuration */
#ifdef __linux__
#include <sys/utsname.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <netdb.h>
#else
extern struct hostent *gethostbyname(const char *name);
extern int h_errno;
#define herror(hostname)                                                       \
	ERR("Error %d looking up hostname %s\n", h_errno, hostname)
#endif
/* end configuration for host systems */

#define JAN_1970 0x83aa7e80 /* 2208988800 1970 - 1900 in seconds */
#define NTP_PORT (123)

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) (4294 * (x) + ((1981 * (x)) >> 11))

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via clock_settime) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC-1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ((x)*15.2587890625)

#define MODEM_SYNC_OBJ		  "modem_sync"
#define MODEM_SYNC_TRIGGER_METHOD "start_sync"

static int trigger_modem_sync(int flag)
{
	int ret	    = -1;
	int timeout = 30;
	uint32_t id = 0;
	struct ubus_context *ubus;
	struct blob_buf b = { 0 };

	ubus = ubus_connect(NULL);
	if (!ubus) {
		ERR("Failed to connect to ubus\n");
		return -1;
	}

	blob_buf_init(&b, 0);

	blobmsg_add_u32(&b, "trigger", flag);

	if (ubus_lookup_id(ubus, MODEM_SYNC_OBJ, &id)) {
		ERR("Failed to find %s ubus object\n", MODEM_SYNC_OBJ);
		goto end;
	}

	if (ubus_invoke(ubus, id, MODEM_SYNC_TRIGGER_METHOD, b.head, NULL, NULL,
			timeout * 1000)) {
		ERR("Failed to invoke a %s object method\n", MODEM_SYNC_TRIGGER_METHOD);
		goto end;
	}

	ret = 0;

end:
	blob_buf_free(&b);
	ubus_free(ubus);

	return ret;
}

static void ntpclient_sigaction(int signo)
{
	if (SIGTERM != signo)
		return;

	if (trigger_modem_sync(1)) {
		ERR("Failed to trigger modem_sync\n");
	}

	exit(0);
}

static int get_current_freq(void)
{
	/* OS dependent routine to get the current value of clock frequency.
	 */
#ifdef __linux__
	struct timex txc;
	txc.modes = 0;
	if (adjtimex(&txc) < 0) {
		perror("adjtimex");
		exit(1);
	}
	return txc.freq;
#else
	return 0;
#endif
}

static int set_freq(int new_freq)
{
	/* OS dependent routine to set a new value of clock frequency.
	 */
#ifdef __linux__
	struct timex txc;
	txc.modes = ADJ_FREQUENCY;
	txc.freq  = new_freq;
	if (adjtimex(&txc) < 0) {
		perror("adjtimex");
		exit(1);
	}
	return txc.freq;
#else
	return 0;
#endif
}

static void set_time(struct ntptime *new)
{
#ifndef USE_OBSOLETE_GETTIMEOFDAY
	/* POSIX 1003.1-2001 way to set the system clock
	 */
	struct timespec tv_set;
	/* it would be even better to subtract half the slop */
	tv_set.tv_sec = new->coarse - JAN_1970;
	/* divide xmttime.fine by 4294.967296 */
	tv_set.tv_nsec = USEC(new->fine) * 1000;
	if (clock_settime(CLOCK_REALTIME, &tv_set) < 0) {
		perror("clock_settime");
		exit(1);
	}
	DBG("set time to %" PRId64 ".%.9" PRId64 "\n",
		       (int64_t)tv_set.tv_sec, (int64_t)tv_set.tv_nsec);
#else
	/* Traditional Linux way to set the system clock
	 */
	struct timeval tv_set;
	/* it would be even better to subtract half the slop */
	tv_set.tv_sec = new->coarse - JAN_1970;
	/* divide xmttime.fine by 4294.967296 */
	tv_set.tv_usec = USEC(new->fine);
	if (settimeofday(&tv_set, NULL) < 0) {
		perror("settimeofday");
		exit(1);
	}
	DBG("set time to %" PRId64 ".%.6" PRId64 "\n",
		       (int64_t)tv_set.tv_sec, (int64_t)tv_set.tv_usec);
#endif
}

static void ntpc_gettime(uint32_t *time_coarse, uint32_t *time_fine)
{
#ifndef USE_OBSOLETE_GETTIMEOFDAY
	/* POSIX 1003.1-2001 way to get the system time
	 */
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	*time_coarse = now.tv_sec + JAN_1970;
	*time_fine   = NTPFRAC(now.tv_nsec / 1000);
#else
	/* Traditional Linux way to get the system time
	 */
	struct timeval now;
	gettimeofday(&now, NULL);
	*time_coarse = now.tv_sec + JAN_1970;
	*time_fine   = NTPFRAC(now.tv_usec);
#endif
}

static void get_packet_timestamp(int usd, struct ntptime *udp_arrival_ntp)
{
#ifdef PRECISION_SIOCGSTAMP
	struct timeval udp_arrival;
	if (ioctl(usd, SIOCGSTAMP, &udp_arrival) < 0) {
		perror("ioctl-SIOCGSTAMP");
		ntpc_gettime(&udp_arrival_ntp->coarse, &udp_arrival_ntp->fine);
	} else {
		udp_arrival_ntp->coarse = udp_arrival.tv_sec + JAN_1970;
		udp_arrival_ntp->fine	= NTPFRAC(udp_arrival.tv_usec);
	}
#else
	(void)usd; /* not used */
	ntpc_gettime(&udp_arrival_ntp->coarse, &udp_arrival_ntp->fine);
#endif
}

static int check_source(int data_len, struct sockaddr_in *sa_in,
			unsigned int sa_len, struct ntp_control *ntpc)
{
	struct sockaddr *sa_source = (struct sockaddr *)sa_in;
	(void)sa_len; /* not used */
	DBG("packet of length %d received\n", data_len);
	if (sa_source->sa_family == AF_INET) {
		DBG("Source: INET Port %d host %s\n",
				ntohs(sa_in->sin_port),
				inet_ntoa(sa_in->sin_addr));
	} else {
		DBG("Source: Address family %d\n",
				sa_source->sa_family);
	}
	/* we could check that the source is the server we expect, but
	 * Denys Vlasenko recommends against it: multihomed hosts get it
	 * wrong too often. */
#if 0
	if (memcmp(ntpc->serv_addr, &(sa_in->sin_addr), 4)!=0) {
		return 1;  /* fault */
	}
#else
	(void)ntpc; /* not used */
#endif
	if (NTP_PORT != ntohs(sa_in->sin_port)) {
		return 1; /* fault */
	}
	return 0;
}

static double ntpdiff(struct ntptime *start, struct ntptime *stop)
{
	int a;
	unsigned int b;
	a = stop->coarse - start->coarse;
	if (stop->fine >= start->fine) {
		b = stop->fine - start->fine;
	} else {
		b = start->fine - stop->fine;
		b = ~b;
		a -= 1;
	}

	return a * 1.e6 + b * (1.e6 / 4294967296.0);
}

/* Does more than print, so this name is bogus.
 * It also makes time adjustments, both sudden (-s)
 * and phase-locking (-l).
 * sets *error to the number of microseconds uncertainty in answer
 * returns 0 normally, 1 if the message fails sanity checks
 */
static int rfc1305print(uint32_t *data, struct ntptime *arrival,
			struct ntp_control *ntpc, int *error)
{
	/* straight out of RFC-1305 Appendix A */
	int li, vn, mode, stratum, poll, prec;
	int delay, disp, refid;
	struct ntptime reftime, orgtime, rectime, xmttime;
	double el_time, st_time, skew1, skew2;
	int freq;
#ifdef ENABLE_DEBUG
	const char *drop_reason = NULL;
#endif

#define Data(i) ntohl(((uint32_t *)data)[i])
	li	= Data(0) >> 30 & 0x03;
	vn	= Data(0) >> 27 & 0x07;
	mode	= Data(0) >> 24 & 0x07;
	stratum = Data(0) >> 16 & 0xff;
	poll	= Data(0) >> 8 & 0xff;
	prec	= Data(0) & 0xff;
	if (prec & 0x80)
		prec |= 0xffffff00;
	delay	       = Data(1);
	disp	       = Data(2);
	refid	       = Data(3);
	reftime.coarse = Data(4);
	reftime.fine   = Data(5);
	orgtime.coarse = Data(6);
	orgtime.fine   = Data(7);
	rectime.coarse = Data(8);
	rectime.fine   = Data(9);
	xmttime.coarse = Data(10);
	xmttime.fine   = Data(11);
#undef Data

	DBG("LI=%d  VN=%d  Mode=%d  Stratum=%d  Poll=%d  Precision=%d\n",
			li, vn, mode, stratum, poll, prec);
	DBG("Delay=%.1f  Dispersion=%.1f  Refid=%u.%u.%u.%u\n",
			sec2u(delay), sec2u(disp), refid >> 24 & 0xff,
			refid >> 16 & 0xff, refid >> 8 & 0xff, refid & 0xff);
	DBG("Reference %u.%.6u\n", reftime.coarse,
			USEC(reftime.fine));
	DBG("(sent)    %u.%.6u\n", ntpc->time_of_send[0],
			USEC(ntpc->time_of_send[1]));
	DBG("Originate %u.%.6u\n", orgtime.coarse,
			USEC(orgtime.fine));
	DBG("Receive   %u.%.6u\n", rectime.coarse,
			USEC(rectime.fine));
	DBG("Transmit  %u.%.6u\n", xmttime.coarse,
			USEC(xmttime.fine));
	DBG("Our recv  %u.%.6u\n", arrival->coarse,
			USEC(arrival->fine));

	el_time = ntpdiff(&orgtime, arrival); /* elapsed */
	st_time = ntpdiff(&rectime, &xmttime); /* stall */
	skew1	= ntpdiff(&orgtime, &rectime);
	skew2	= ntpdiff(&xmttime, arrival);
	freq	= get_current_freq();
	DBG("Total elapsed: %9.2f\n"
			"Server stall:  %9.2f\n"
			"Slop:          %9.2f\n",
			el_time, st_time, el_time - st_time);
	DBG("Skew:          %9.2f\n"
			"Frequency:     %9d\n"
			" day   second     elapsed    stall     skew  dispersion  freq\n",
			(skew1 - skew2) / 2, freq);

	/* error checking, see RFC-4330 section 5 */
#ifdef ENABLE_DEBUG
#define FAIL(x)                                                                \
	do {                                                                   \
		drop_reason = (x);                                             \
		goto fail;                                                     \
	} while (0)
#else
#define FAIL(x) goto fail;
#endif
	if (ntpc->cross_check) {
		if (li == 3)
			FAIL("LI==3"); /* unsynchronized */
		if (vn < 3)
			FAIL("VN<3"); /* RFC-4330 documents SNTP v4, but we interoperate with NTP v3 */
		if (mode != 4)
			FAIL("MODE!=3");
		if (orgtime.coarse != ntpc->time_of_send[0] ||
		    orgtime.fine != ntpc->time_of_send[1])
			FAIL("ORG!=sent");
		if (xmttime.coarse == 0 && xmttime.fine == 0)
			FAIL("XMT==0");
		if (delay > 65536 || delay < -65536)
			FAIL("abs(DELAY)>65536");
		if (disp > 65536 || disp < -65536)
			FAIL("abs(DISP)>65536");
		if (stratum == 0)
			FAIL("STRATUM==0"); /* kiss o' death */
#undef FAIL
	}

	/* XXX should I do this if debug flag is set? */
	if (ntpc->set_clock) { /* you'd better be root, or ntpclient will exit here! */
		set_time(&xmttime);
	}

	/* Not the ideal order for printing, but we want to be sure
	 * to do all the time-sensitive thinking (and time setting)
	 * before we start the output, especially fflush() (which
	 * could be slow).  Of course, if debug is turned on, speed
	 * has gone down the drain anyway. */
	if (ntpc->live) {
		int new_freq;
		new_freq =
			contemplate_data(arrival->coarse, (skew1 - skew2) / 2,
					 el_time + sec2u(disp), freq);
		if (!g_debug && new_freq != freq)
			set_freq(new_freq);
	}
	LOG("%d %.5d.%.3d  %8.1f %8.1f  %8.1f %8.1f %9d\n",
	       arrival->coarse / 86400, arrival->coarse % 86400,
	       arrival->fine / 4294967, el_time, st_time, (skew1 - skew2) / 2,
	       sec2u(disp), freq);
	*error = el_time - st_time;

	return 0;
fail:
#ifdef ENABLE_DEBUG
	LOG("%d %.5d.%.3d  rejected packet: %s\n", arrival->coarse / 86400,
	       arrival->coarse % 86400, arrival->fine / 4294967, drop_reason);
#else
	LOG("%d %.5d.%.3d  rejected packet\n", arrival->coarse / 86400,
	       arrival->coarse % 86400, arrival->fine / 4294967);
#endif
	return 1;
}

static void send_packet(int usd, uint32_t time_sent[2])
{
	uint32_t data[12];
#define LI	0
#define VN	3
#define MODE	3
#define STRATUM 0
#define POLL	4
#define PREC	-6

	DBG("Sending ...\n");
	if (sizeof data != 48) {
		ERR("size error\n");
		return;
	}
	memset(data, 0, sizeof data);
	data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24) |
			(STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
	data[1] = htonl(1 << 16); /* Root Delay (seconds) */
	data[2] = htonl(1 << 16); /* Root Dispersion (seconds) */
	ntpc_gettime(time_sent, time_sent + 1);
	data[10] = htonl(time_sent[0]); /* Transmit Timestamp coarse */
	data[11] = htonl(time_sent[1]); /* Transmit Timestamp fine   */
	send(usd, data, 48, 0);
}

static int find_server(struct ntp_control *ntpc) {
	int socket = -1;

	for (int i = 0; i < ntpc->hosts; i++) {
		socket = net_setup(ntpc->hostnames[i], NTP_PORT, ntpc->udp_local_port);
		if (socket != -1) {
			LOG("Working with server %s\n", ntpc->hostnames[i]);
			break;
		}
	}
	return socket;
}

static void ntpc_sleep(time_t seconds) {
	struct timeval tv;
	int i;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	do {
		i = select(1, NULL, NULL, NULL, &tv);
	} while (i == -1 && errno == EINTR);
}

static int ntpc_handle_timeout(struct ntp_control *ntpc)
{
	int ret = 0;
	DBG("Timeout\n");
	if (ntpc->probes_sent >= ntpc->probe_count && ntpc->probe_count != 0) {
		ret = 1;
		goto end;
	}

	if (ntpc->failover_cnt && !ntpc->probe_count &&
		ntpc->probes_sent >= ntpc->failover_cnt && !ntpc->modem_sync_flg) {
		DBG("Modem time sync is starting\n");
		if (trigger_modem_sync(1)) {
			ERR("Failed to trigger modem_sync\n");
		} else {
			ntpc->probes_sent    = 0;
			ntpc->modem_sync_flg = 1;
		}
	}
end:
	return ret;
}

static void primary_loop(struct ntp_control *ntpc)
{
	fd_set fds;
	struct sockaddr_in sa_xmit_in;
	int i, pack_len, error;
	socklen_t sa_xmit_len;
	struct timeval to;
	struct ntptime udp_arrival_ntp;
	static uint32_t incoming_word[325];
#define incoming	((char *)incoming_word)
#define sizeof_incoming (sizeof incoming_word)

	if (ntpc->failover_cnt && !ntpc->probe_count && trigger_modem_sync(0)) {
		ERR("Failed to trigger modem_sync\n");
	}

	ntpc->modem_sync_flg = 0;
	ntpc->probes_sent = 0;
	sa_xmit_len = sizeof sa_xmit_in;
	to.tv_sec   = 0;
	to.tv_usec  = 0;

	int usd = -1;
	int sleep_time = ntpc->retry_interval;

	for (;;) {
		DBG("sleep_time = %d\n", sleep_time);
		DBG("cycle_time = %d\n", ntpc->cycle_time);
		DBG("retry_interval = %d\n", ntpc->retry_interval);
		DBG("Finding server...\n");
		usd = find_server(ntpc);
		if (usd < 0) {
			ntpc->probes_sent++;
			if (ntpc_handle_timeout(ntpc)) {
				break;
			}
			sleep_time = ntpc->retry_interval;
			goto loopend;
		}

		//We have socket here
		send_packet(usd, ntpc->time_of_send);
		++ntpc->probes_sent;

		//Wait for reply
		to.tv_sec  = sleep_time;
		to.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(usd, &fds);
		do {
			i = select(usd + 1, &fds, NULL, NULL, &to); // Wait on read or error
		} while (i == -1 && errno == EINTR);

		if (i == -1) {
			//real error here
			sleep_time = ntpc->retry_interval;
			goto loopend;
		}

		if (i == 0){
			//timeout
			if (ntpc_handle_timeout(ntpc)) {
				break;
			}
			sleep_time = ntpc->retry_interval;
			goto loopend;
		}

		// We have something to read here.
		DBG("Receiving...\n");
		pack_len = recvfrom(usd, incoming, sizeof_incoming, 0,
				 (struct sockaddr *)&sa_xmit_in, &sa_xmit_len);
		error = ntpc->goodness;
		sleep_time = ntpc->retry_interval;
		sleep_time = 0;
		if (pack_len < 0) {
			ERR("recvfrom error");
		} else if (pack_len > 0 && (unsigned)pack_len < sizeof_incoming) {
			if (ntpc->failover_cnt) {
				ntpc->probes_sent = 0;
			}
			get_packet_timestamp(usd, &udp_arrival_ntp);

			if (check_source(pack_len, &sa_xmit_in, sa_xmit_len, ntpc) != 0) {
				goto loopend;
			}

			if (rfc1305print(incoming_word, &udp_arrival_ntp, ntpc, &error) != 0) {
				goto loopend;
			} else {
				sleep_time = ntpc->cycle_time;
				ntpc->retry_interval = RETRY_INTERVAL;
				if (ntpc->modem_sync_flg) {
					DBG("Modem time sync is stopping\n");
					if (trigger_modem_sync(0)) {
						ERR("Failed to stop modem_sync\n");
					} else {
						ntpc->modem_sync_flg = 0;
					}
				}
			}
		} else {
			LOG("Ooops.  pack_len=%d\n", pack_len);
		}
		/* best rollover option: specify -g, -s, and -l.
		 * simpler rollover option: specify -s and -l, which
		 * triggers a magic -c 1 */
		if ((error < ntpc->goodness && ntpc->goodness != 0) ||
		    (ntpc->probes_sent >= ntpc->probe_count && ntpc->probe_count != 0)) {
			if (ntpc->failover_cnt) {
				ntpc->probes_sent = 0;
			}
			ntpc->set_clock = 0;
			if (!ntpc->live) {
				break;
			}
		}
loopend:
		close(usd);
		DBG("Probes sent: %d\n", ntpc->probes_sent);
		DBG("Going to sleep %d sec.\n", sleep_time);
		ntpc_sleep(sleep_time);
		if (sleep_time < ntpc->cycle_time) {
			ntpc->retry_interval <<= 1;
			if (ntpc->retry_interval > ntpc->cycle_time) {
				ntpc->retry_interval = ntpc->cycle_time;
			}
		}
	} //for(;;)
	close(usd);
#undef incoming
#undef sizeof_incoming
}

#ifdef ENABLE_REPLAY
static void do_replay(void)
{
	char line[100];
	int n, day, freq, absolute;
	float sec, el_time, st_time, disp;
	double skew, errorbar;
	int simulated_freq	    = 0;
	unsigned int last_fake_time = 0;
	double fake_delta_time	    = 0.0;

	while (fgets(line, sizeof line, stdin)) {
		n = sscanf(line, "%d %f %f %f %lf %f %d", &day, &sec, &el_time,
			   &st_time, &skew, &disp, &freq);
		if (n == 7) {
			fputs(line, stdout);
			absolute = day * 86400 + (int)sec;
			errorbar = el_time + disp;
			DBG("contemplate %u %.1f %.1f %d\n",
				       absolute, skew, errorbar, freq);
			if (last_fake_time == 0)
				simulated_freq = freq;
			fake_delta_time += (absolute - last_fake_time) *
					   ((double)(freq - simulated_freq)) /
					   65536;
			DBG("fake %f %d \n", fake_delta_time,
				       simulated_freq);
			skew += fake_delta_time;
			freq	       = simulated_freq;
			last_fake_time = absolute;
			simulated_freq = contemplate_data(absolute, skew,
							  errorbar, freq);
		} else {
			ERR("Replay input error\n");
			exit(2);
		}
	}
}
#endif

static void usage(char *argv0)
{
	LOG("Usage: %s [-c count]"
#ifdef ENABLE_DEBUG
		" [-d]"
#endif
		" [-f frequency] [-g goodness] -h hostname\n"
		"\t[-i interval] [-l] [-p port] [-q min_delay]"
#ifdef ENABLE_REPLAY
		" [-r]"
#endif
		" [-s] [-t]\n",
		argv0);
}

int main(int argc, char *argv[])
{
	int c;
	int ret	  = EXIT_FAILURE;
	/* These parameters are settable from the command line
	   the initializations here provide default behavior */
	int initial_freq; /* initial freq value to use */
	struct ntp_control ntpc;
	ntpc.udp_local_port = 0; /* default of 0 means kernel chooses */
	ntpc.live	 = 0;
	ntpc.set_clock	 = 0;
	ntpc.probe_count = 0; /* default of 0 means loop forever */
	ntpc.failover_cnt = 0;
	ntpc.cycle_time	 = 600; /* seconds */
	ntpc.retry_interval = RETRY_INTERVAL;
	ntpc.goodness	 = 0;
	ntpc.cross_check = 1;
	ntpc.hosts = 0;
	memset(ntpc.hostnames, 0, sizeof(ntpc.hostnames));

	for (;;) {
		c = getopt(argc, argv,
			   "c:" DEBUG_OPTION "f:g:h:i:lp:q:k:" REPLAY_OPTION
			   "stD");
		if (c == EOF)
			break;
		switch (c) {
		case 'c':
			ntpc.probe_count = atoi(optarg);
			break;
#ifdef ENABLE_DEBUG
		case 'd':
			++g_debug;
			break;
#endif
		case 'f':
			initial_freq = atoi(optarg);
			DBG("initial frequency %d\n", initial_freq);
			set_freq(initial_freq);
			break;
		case 'g':
			ntpc.goodness = atoi(optarg);
			break;
		case 'h':
			if (ntpc.hosts < NTP_MAX_SERVERS) {
				ntpc.hostnames[ntpc.hosts++] = optarg;
			}
			break;
		case 'k':
			ntpc.failover_cnt = strtol(optarg, NULL, 10);
			break;
		case 'i':
			ntpc.cycle_time = atoi(optarg);
			break;
		case 'l':
			(ntpc.live)++;
			break;
		case 'p':
			ntpc.udp_local_port = atoi(optarg);
			break;
		case 'q':
			min_delay = atof(optarg);
			break;
#ifdef ENABLE_REPLAY
		case 'r':
			do_replay();
			exit(0);
			break;
#endif
		case 's':
			ntpc.set_clock++;
			break;

		case 't':
			ntpc.cross_check = 0;
			break;

		case 'D':
			daemon(0, 0);
			break;

		default:
			usage(argv[0]);
			ret = 1;
			goto end;
		}
	}
	if (ntpc.hosts == 0) {
		usage(argv[0]);
		ret = 1;
		goto end;
	}

	if (ntpc.set_clock && !ntpc.live && !ntpc.goodness &&
	    !ntpc.probe_count) {
		ntpc.probe_count = 1;
	}

	/* respect only applicable MUST of RFC-4330 */
	if (ntpc.probe_count != 1 && ntpc.cycle_time < MIN_INTERVAL) {
		ntpc.cycle_time = MIN_INTERVAL;
	}

	DBG("Configuration:\n"
			"  -c probe_count %d\n"
			"  -d (debug)     %d\n"
			"  -g goodness    %d\n"
			"  -k failover_count %d\n"
			"  -i interval    %d\n"
			"  -l live        %d\n"
			"  -p local_port  %d\n"
			"  -q min_delay   %f\n"
			"  -s set_clock   %d\n"
			"  -x cross_check %d\n",
			ntpc.probe_count, g_debug, ntpc.goodness, ntpc.failover_cnt,
			ntpc.cycle_time, ntpc.live, ntpc.udp_local_port, min_delay,
			ntpc.set_clock, ntpc.cross_check);
	DBG("Hosts:\n");
	for (int i = 0;  i < ntpc.hosts && ntpc.hostnames[i]; i++) {
		DBG("  %d. %s\n", i + 1, ntpc.hostnames[i]);
	}
	DBG("Compiled with NTP_MAX_SERVERS = %d\n", NTP_MAX_SERVERS);

	signal(SIGTERM, ntpclient_sigaction);

	primary_loop(&ntpc);

	ret = EXIT_SUCCESS;
end:
	if (ret != EXIT_SUCCESS && ntpc.probe_count && ntpc.failover_cnt &&
	    trigger_modem_sync(1)) {
		ERR("Failed to trigger modem_sync\n");
	}

	return ret;
}
