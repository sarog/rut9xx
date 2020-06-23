/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>

#if defined(CONFIG_CMD_NET)

extern int do_bootm(cmd_tbl_t *, int, int, char *[]);
static int netboot_common(proto_t, cmd_tbl_t *, int, char *[]);

#if defined(CONFIG_CMD_HTTPD)
int do_httpd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]){
	return NetLoopHttpd();
}
U_BOOT_CMD(httpd, 1, 1, do_httpd, "start www server for firmware recovery\n", NULL);
#endif /* CONFIG_CMD_HTTPD */

int do_tftpb(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]){
	return netboot_common(TFTP, cmdtp, argc, argv);
}
U_BOOT_CMD(tftpboot, 3, 1, do_tftpb, "boot image via network using TFTP protocol\n", "[loadAddress] [bootfilename]\n");

#if defined(CONFIG_CMD_DHCP)
int do_dhcp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]){
	return netboot_common(DHCP, cmdtp, argc, argv);
}
U_BOOT_CMD(dhcp, 3, 1, do_dhcp, "invoke DHCP client to obtain IP/boot params\n", NULL);
#endif /* CONFIG_CMD_DHCP */

#if defined(CONFIG_CMD_NFS)
int do_nfs(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]){
	return netboot_common(NFS, cmdtp, argc, argv);
}
U_BOOT_CMD(nfs, 3, 1, do_nfs, "boot image via network using NFS protocol\n", "[loadAddress] [host ip addr:bootfilename]\n");
#endif /* CONFIG_CMD_NFS */

static void netboot_update_env(void){
	char tmp[22];

	if(NetOurGatewayIP){
		ip_to_string(NetOurGatewayIP, tmp);
		setenv("gatewayip", tmp);
	}

	if(NetOurSubnetMask){
		ip_to_string(NetOurSubnetMask, tmp);
		setenv("netmask", tmp);
	}

	if(NetOurHostName[0]){
		setenv("hostname", NetOurHostName);
	}

	if(NetOurRootPath[0]){
		setenv("rootpath", NetOurRootPath);
	}

	if(NetOurIP){
		ip_to_string(NetOurIP, tmp);
		setenv("ipaddr", tmp);
	}

	if(NetServerIP){
		ip_to_string(NetServerIP, tmp);
		setenv("serverip", tmp);
	}

	if(NetOurDNSIP){
		ip_to_string(NetOurDNSIP, tmp);
		setenv("dnsip", tmp);
	}

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_DNS2)
	if(NetOurDNS2IP){
		ip_to_string (NetOurDNS2IP, tmp);
		setenv("dnsip2", tmp);
	}
#endif

	if(NetOurNISDomain[0]){
		setenv("domain", NetOurNISDomain);
	}

#if defined(CONFIG_CMD_SNTP) && (CONFIG_BOOTP_MASK & CONFIG_BOOTP_TIMEOFFSET)
	if(NetTimeOffset){
		sprintf(tmp, "%d", NetTimeOffset);
		setenv("timeoffset", tmp);
	}
#endif

#if defined(CONFIG_CMD_SNTP) && (CONFIG_BOOTP_MASK & CONFIG_BOOTP_NTPSERVER)
	if (NetNtpServerIP){
		ip_to_string(NetNtpServerIP, tmp);
		setenv("ntpserverip", tmp);
	}
#endif
}

static int netboot_common(proto_t proto, cmd_tbl_t *cmdtp, int argc, char *argv[]){
	char *s;
	int rcode = 0;
	int size;

	/* pre-set load_addr */
	if((s = getenv("loadaddr")) != NULL){
		load_addr = simple_strtoul(s, NULL, 16);
	}

	switch(argc){
		case 1:
			break;

		case 2:
		/* only one arg - accept two forms:
		 * just load address, or just boot file name.
		 * The latter form must be written "filename" here.
		 */
			if(argv[1][0] == '"'){ /* just boot filename */
				copy_filename(BootFile, argv[1], sizeof(BootFile));
			} else { /* load address	*/
				load_addr = simple_strtoul(argv[1], NULL, 16);
			}

			break;

		case 3:
			load_addr = simple_strtoul(argv[1], NULL, 16);
			copy_filename(BootFile, argv[2], sizeof(BootFile));

			break;

		default:

			print_cmd_help(cmdtp);
			return 1;
	}

	if((size = NetLoop(proto)) < 0){
		return(1);
	}

	/* NetLoop ok, update environment */
	netboot_update_env();

	/* done if no file was loaded (no errors though) */
	if(size == 0){
		return(0);
	}

	/* flush cache */
	flush_cache(load_addr, size);

	/* Loading ok, check if we should attempt an auto-start */
	if(((s = getenv("autostart")) != NULL) && (strcmp(s, "yes") == 0)){
		char *local_args[2];
		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf("Automatic boot of image at addr 0x%08lX ...\n", load_addr);
		rcode = do_bootm(cmdtp, 0, 1, local_args);
	}

#ifdef CONFIG_AUTOSCRIPT
	if(((s = getenv("autoscript")) != NULL) && (strcmp(s,"yes") == 0)){
		printf("Running autoscript at addr 0x%08lX ...\n", load_addr);
		rcode = autoscript(load_addr);
	}
#endif
	return rcode;
}

#if defined(CONFIG_CMD_PING)
int do_ping(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]){

	if(argc < 2){
		print_cmd_help(cmdtp);
		return(-1);
	}

	NetPingIP = string_to_ip(argv[1]);

	if (NetPingIP == 0){
		print_cmd_help(cmdtp);
		return(-1);
	}

	if(NetLoop(PING) < 0){
		printf("\n## Error: ping failed, host %s is not alive!\n\n", argv[1]);
		return(1);
	}

	printf("\nPing OK, host %s is alive!\n\n", argv[1]);

	return(0);
}

U_BOOT_CMD(ping, 2, 1, do_ping, "send ICMP ECHO_REQUEST to network host\n", "host IP\n"
		"\t- sends ping to IP 'host IP'\n");
#endif /* CONFIG_CMD_PING */

#if defined(CONFIG_CMD_SNTP)
int do_sntp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]){
	char *toff;

	if(argc < 2){
		print_cmd_help(cmdtp);
		return(-1);
	} else {
		NetNtpServerIP = string_to_ip(argv[1]);
		if(NetNtpServerIP == 0){
			printf("## Error: bad SNTP server IP address\n");
			return(1);
		}
	}

	toff = getenv("timeoffset");

	if(toff == NULL){
		NetTimeOffset = 0;
	} else{
		NetTimeOffset = simple_strtol(toff, NULL, 10);
	}

	if(NetLoop(SNTP) < 0){
		printf("## Error: SNTP host %s not responding\n", argv[1]);
		return(1);
	}

	return(0);
}

U_BOOT_CMD(sntp, 2, 1, do_sntp, "send NTP request to NTP server\n", "ntpserverip\n"
		"\t- sends NTP request to NTP server 'ntpserverip'\n");
#endif /* CONFIG_CMD_SNTP */

#endif /* CONFIG_CMD_NET */
