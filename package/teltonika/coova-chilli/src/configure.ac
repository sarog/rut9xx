#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([coova-chilli],[1.4],[https://github.com/coova/coova-chilli/issues])
AC_CONFIG_SRCDIR([src/chilli.c])

AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

#JJAKO Check for libtool
LT_INIT
AC_ARG_PROGRAM

# Checks for libraries.

if test -r /dev/bpf -o -h /dev/bpf ; then
  V_DEV=bpf
  AC_DEFINE(HAVE_CLONING_BPF,1,[define if you have a cloning BPF device])
elif test -r /dev/bpf0 ; then
  V_DEV=bpf
elif test -r /usr/include/net/pfilt.h ; then
  V_DEV=pf
elif test -r /dev/enet ; then
  V_DEV=enet
elif test -r /dev/nit ; then
  V_DEV=snit
elif test -r /usr/include/sys/net/nit.h ; then
  V_DEV=nit
elif test -r /usr/include/linux/socket.h ; then
  V_DEV=linux
elif test -r /usr/include/net/raw.h ; then
  V_DEV=snoop
elif test -r /usr/include/odmi.h ; then
  V_DEV=bpf
elif test -c /dev/bpf0 ; then # check again in case not readable
  V_DEV=bpf
elif test -r /usr/include/sys/dlpi.h ; then
  V_DEV=dlpi
elif test -c /dev/enet ; then # check again in case not readable
  V_DEV=enet
elif test -c /dev/nit ; then # check again in case not readable
  V_DEV=snit
else
  V_DEV=null
fi
AC_MSG_RESULT($V_DEV)

# Do capture-mechanism-dependent tests.
case "$V_DEV" in
linux)
	AC_LBL_TPACKET_STATS
	AC_LBL_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI
	;;
esac

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_RESOLV
AC_CHECK_HEADERS([arpa/inet.h errno.h fcntl.h dirent.h \
		  inttypes.h limits.h \
		  netdb.h netinet/in.h netinet/tcp.h \
		  stdint.h stddef.h stdlib.h string.h strings.h \
		  sys/ioctl.h sys/socket.h linux/sysinfo.h sys/sysinfo.h \
		  sys/param.h sys/time.h time.h \
		  sys/ipc.h sys/msg.h signal.h \
		  sys/wait.h sys/un.h ifaddrs.h \
		  sys/stat.h sys/types.h regex.h \
		  syslog.h poll.h sys/epoll.h \
		  unistd.h endian.h libgen.h \
		  asm/types.h pwd.h grp.h wchar.h stdarg.h])

AC_CHECK_HEADERS([resolv.h net/route.h net/if.h net/if_arp.h net/if_tun.h net/ethernet.h], [], [],
[
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
])

AC_CHECK_HEADER(inttypes.h,[AC_DEFINE([JSON_C_HAVE_INTTYPES_H],[1],[Public define for json_inttypes.h])])
AC_CONFIG_HEADER(json/json_config.h)
AC_CHECK_DECLS([INFINITY], [], [], [[#include <math.h>]])
AC_CHECK_DECLS([nan], [], [], [[#include <math.h>]])
AC_CHECK_DECLS([isnan], [], [], [[#include <math.h>]])
AC_CHECK_DECLS([isinf], [], [], [[#include <math.h>]])
AC_CHECK_DECLS([_isnan], [], [], [[#include <float.h>]])
AC_CHECK_DECLS([_finite], [], [], [[#include <float.h>]])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])


# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_CHECK_FUNCS([bzero clock_gettime dup2 gethostbyname getprotoent gettimeofday inet_ntoa \
memchr memmove memset mkdir munmap regcomp select setenv socket strcasecmp \
strchr strcspn strdup strerror strncasecmp strndup strrchr strspn strstr strtol getline dirname \
glob getaddrinfo getnameinfo getifaddrs sysinfo strlcpy tzset snprintf vsnprintf vasprintf])
AC_CHECK_LIB(resolv, res_init)

AC_ARG_ENABLE(chilliquery, [AS_HELP_STRING([--disable-chilliquery],[Disable chilli_query])], 
  enable_chilliquery=$enableval, enable_chilliquery=yes)

if test x"$enable_chilliquery" = xyes; then
   AC_DEFINE(ENABLE_CHILLIQUERY,1,[Define to enable chilli_query])
fi

AC_ARG_ENABLE(ubus, [AS_HELP_STRING([--disable-ubus],[Disable ubus])],
  enable_ubus=$enableval, enable_ubus=yes)

if test x"$enable_ubus" = xyes; then
   AC_DEFINE(ENABLE_UBUS,1,[Define to enable ubus])
fi

AM_CONDITIONAL(WITH_CHILLIQUERY, [test x"$enable_chilliquery" != xno])

AM_CONDITIONAL(WITH_UBUS, [test x"$enable_ubus" != xno])

AC_ARG_ENABLE(leakybucket, [AS_HELP_STRING([--disable-leakybucket],[disable use of leaky bucket shaping])], 
  enable_leakybucket=$enableval, enable_leakybucket=yes)

if test x"$enable_leakybucket" = xyes; then
   AC_DEFINE(ENABLE_LEAKYBUCKET,1,[Define to enable Chilli Leaky Bucket shaping])
fi

AC_ARG_ENABLE(uamanyip, [AS_HELP_STRING([--disable-uamanyip],[disable use of uamanyip])], 
  enable_uamanyip=$enableval, enable_uamanyip=yes)

if test x"$enable_uamanyip" = xyes; then
   AC_DEFINE(ENABLE_UAMANYIP,1,[Define to enable uamanyip])
fi

AC_ARG_ENABLE(uamuiport, [AS_HELP_STRING([--disable-uamuiport],[disable use of uamuiport])], 
  enable_uamuiport=$enableval, enable_uamuiport=yes)

if test x"$enable_uamuiport" = xyes; then
   AC_DEFINE(ENABLE_UAMUIPORT,1,[Define to enable uamuiport])
fi

AC_ARG_ENABLE(accounting-onoff, [AS_HELP_STRING([--disable-accounting-onoff],[disable use of Accounting-On and Accounting-Off])], 
  enable_accounting_onoff=$enableval, enable_accounting_onoff=yes)

if test x"$enable_accounting_onoff" = xyes; then
   AC_DEFINE(ENABLE_ACCOUNTING_ONOFF,1,[Define to enable Accounting-On and Accounting-Off])
fi

AC_ARG_ENABLE(tap, [AS_HELP_STRING([--disable-tap],[Disable support for tap interface (tun only)])], 
  enable_tap=$enableval, enable_tap=yes)

if test x"$enable_tap" = xyes; then
   AC_DEFINE(ENABLE_TAP,1,[Define to enable Chilli tap support])
fi

AC_ARG_ENABLE(tcpreset, [AS_HELP_STRING([--disable-tcpreset],[Disable support for TCP reset of filtered connections])], 
  enable_tcpreset=$enableval, enable_tcpreset=yes)

if test x"$enable_tcpreset" = xyes; then
   AC_DEFINE(ENABLE_TCPRESET,1,[Define to enable TCP reset support])
fi

AC_ARG_ENABLE(radproxy, [AS_HELP_STRING([--disable-radproxy],[Disable support RADIUS (EAP) Proxy])], 
  enable_radproxy=$enableval, enable_radproxy=yes)

if test x"$enable_radproxy" = xyes; then
   AC_DEFINE(ENABLE_RADPROXY,1,[Define to enable Chilli RADIUS (EAP) Proxy support])
fi

AC_ARG_ENABLE(json, [AS_HELP_STRING([--enable-json],[Enable support for JSON])], 
  enable_json=$enableval, enable_json=yes)

if test x"$enable_json" = xyes; then
   AC_DEFINE(ENABLE_JSON,1,[Define to enable Chilli JSON])
   AC_CHECK_LIB([json-c], [json_object_new_object],
	      [AC_SUBST([LIBJSON], ["-ljson-c"])],
				   [AC_CHECK_LIB([json], [json_object_new_object],
				   [AC_SUBST([LIBJSON], ["-ljson"])],
				   [AC_MSG_FAILURE(
				   [--enable-json was given, but test for libjson failed])],)])
fi

AC_ARG_ENABLE(debug, [AS_HELP_STRING([--disable-debug],[Disable debugging messages])], 
  enable_debug=$enableval, enable_debug=yes)

if test x"$enable_debug" = xyes; then
   AC_DEFINE(ENABLE_DEBUG,1,[Define to enable debugging])
fi

AC_ARG_ENABLE(dhcpradius, [AS_HELP_STRING([--disable-dhcpradius],[Disable support DHCP/RADIUS integration])], 
  enable_dhcpradius=$enableval, enable_dhcpradius=yes)

if test x"$enable_dhcpradius" = xyes; then
   AC_DEFINE(ENABLE_DHCPRADIUS,1,[Define to enable DHCP/RADIUS integration])
fi

AC_ARG_ENABLE(wpad, [AS_HELP_STRING([--enable-wpad],[Enable support WPAD])], 
  enable_wpad=$enableval, enable_wpad=no)

if test x"$enable_wpad" = xyes; then
   AC_DEFINE(ENABLE_WPAD,1,[Define to enable WPAD])
fi

AC_ARG_ENABLE(gardenaccounting, [AS_HELP_STRING([--enable-gardenaccounting],[Enable walled garden accounting])], 
  enable_gardenaccounting=$enableval, enable_gardenaccounting=no)

if test x"$enable_gardenaccounting" = xyes; then
   AC_DEFINE(ENABLE_GARDENACCOUNTING,1,[Define to enable walled garden accounting])
fi

AC_ARG_ENABLE(gardenext, [AS_HELP_STRING([--enable-gardenext],[Enable extended walled garden features])], 
  enable_gardenext=$enableval, enable_gardenext=no)

if test x"$enable_gardenext" = xyes; then
   AC_DEFINE(ENABLE_GARDENEXT,1,[Define to enable extended walled garden features])
fi

AC_ARG_ENABLE(inspect, [AS_HELP_STRING([--enable-inspect],[Enable inspect feature in cmdsock])], 
  enable_inspect=$enableval, enable_inspect=no)

if test x"$enable_inspect" = xyes; then
   AC_DEFINE(ENABLE_INSPECT,1,[Define to enable inspect feature in cmdsock])
fi

AC_ARG_ENABLE(coa, [AS_HELP_STRING([--disable-coa],[Disable CoA RADIUS support])], 
  enable_coa=$enableval, enable_coa=yes)

if test x"$enable_coa" = xyes; then
   AC_DEFINE(ENABLE_COA,1,[Define for CoA RADIUS support])
fi

AC_ARG_ENABLE(dhcpopt, [AS_HELP_STRING([--enable-dhcpopt],[Enable support for DHCP option setting])], 
  enable_dhcpopt=$enableval, enable_dhcpopt=no)

if test x"$enable_dhcpopt" = xyes; then
   AC_DEFINE(ENABLE_DHCPOPT,1,[Define to enable DHCP option setting])
fi


AC_ARG_ENABLE(debug2, [AS_HELP_STRING([--enable-debug2],[Enable verbose debugging])], 
  enable_debug2=$enableval, enable_debug2=no)

if test x"$enable_debug2" = xyes; then
   AC_DEFINE(ENABLE_DEBUG2,1,[Define to enable verbose debugging])
fi

AC_ARG_ENABLE(sessgarden, [AS_HELP_STRING([--enable-sessgarden],[Enable support for session-based walled garden])], 
  enable_sessgarden=$enableval, enable_sessgarden=no)

if test x"$enable_sessgarden" = xyes; then
   AC_DEFINE(ENABLE_SESSGARDEN,1,[Define to enable Chilli session walled garden])
fi

AC_ARG_ENABLE(sessproxy, [AS_HELP_STRING([--enable-sessproxy],[Enable support for per session postauth proxy])], 
  enable_sessproxy=$enableval, enable_sessproxy=no)

if test x"$enable_sessproxy" = xyes; then
   AC_DEFINE(ENABLE_SESSPROXY,1,[Define to enable per session postauth proxy])
fi

AC_ARG_ENABLE(sessdhcp, [AS_HELP_STRING([--enable-sessdhcp],[Enable support for per session DHCP relay])], 
  enable_sessdhcp=$enableval, enable_sessdhcp=no)

if test x"$enable_sessdhcp" = xyes; then
   AC_DEFINE(ENABLE_SESSDHCP,1,[Define to enable per session DHCP relay])
fi

AC_ARG_ENABLE(sessdns, [AS_HELP_STRING([--enable-sessdns],[Enable support for per session DNS enforcement])], 
  enable_sessdns=$enableval, enable_sessdns=no)

if test x"$enable_sessdns" = xyes; then
   AC_DEFINE(ENABLE_SESSDNS,1,[Define to enable per session DNS enforcement])
fi

AC_ARG_ENABLE(chillixml, [AS_HELP_STRING([--enable-chillixml],[Enable use of chillixml])], 
  enable_chillixml=$enableval, enable_chillixml=no)

if test x"$enable_chillixml" = xyes; then
   AC_DEFINE(ENABLE_CHILLIXML,1,[Define to enable Chilli XML])
fi

AC_ARG_ENABLE(proxyvsa, [AS_HELP_STRING([--enable-proxyvsa],[Enable support for VSA attribute proxy])], 
  enable_proxyvsa=$enableval, enable_proxyvsa=no)

if test x"$enable_proxyvsa" = xyes; then
   AC_DEFINE(ENABLE_PROXYVSA,1,[Define to enable VSA proxy])
fi

AC_ARG_ENABLE(ipwhitelist, [AS_HELP_STRING([--enable-ipwhitelist],[Enable file based IP white list])], 
  enable_ipwhitelist=$enableval, enable_ipwhitelist=no)

if test x"$enable_ipwhitelist" = xyes; then
   AC_DEFINE(ENABLE_IPWHITELIST,1,[Define to support file based whitelists])
fi

AC_ARG_ENABLE(uamdomainfile, [AS_HELP_STRING([--enable-uamdomainfile],[Enable loading of mass uamdomains from file])], 
  enable_uamdomainfile=$enableval, enable_uamdomainfile=no)

if test x"$enable_uamdomainfile" = xyes; then
   AC_DEFINE(ENABLE_UAMDOMAINFILE,1,[Define to support loading of uamdomains (with regex) from file])
fi

AC_ARG_ENABLE(redirdnsreq, [AS_HELP_STRING([--enable-redirdnsreq],[Enable the sending of a DNS query on redirect])], 
  enable_redirdnsreq=$enableval, enable_redirdnsreq=no)

if test x"$enable_redirdnsreq" = xyes; then
   AC_DEFINE(ENABLE_REDIRDNSREQ,1,[Define to DNS query on redirect to pick up dynamic walled garden])
fi

AC_ARG_ENABLE(ieee8021q, [AS_HELP_STRING([--disable-ieee8021q],[disable support for IEEE 802.1Q])], 
  enable_ieee8021q=$enableval, enable_ieee8021q=yes)

if test x"$enable_ieee8021q" = xyes; then
   AC_DEFINE(ENABLE_IEEE8021Q,1,[Define to enable Chilli IEEE 802.1Q])
fi

AC_ARG_ENABLE(largelimits, [AS_HELP_STRING([--enable-largelimits],[Enable larger limits for use with non-embedded systems])], 
  enable_largelimits=$enableval, enable_largelimits=no)

if test x"$enable_largelimits" = xyes; then
   AC_DEFINE(ENABLE_LARGELIMITS,1,[Enable larger limits for use with non-embedded systems])
fi

AC_ARG_WITH([openssl],
 [AS_HELP_STRING([--with-openssl], [enable support for openssl])],[],[with_openssl=no])

OPENSSL=
AS_IF([test x"$with_openssl" != xno],
  [AC_CHECK_LIB([crypto], [CRYPTO_malloc],
              [AC_SUBST([LIBSSL], ["-lssl -lcrypto"])
               AC_DEFINE([HAVE_OPENSSL], [1],
                         [Define if you have openssl])
              ],
              [AC_MSG_FAILURE(
                 [--with-openssl was given, but test for openssl failed])],
              [-lssl -lcrypto])])

AM_CONDITIONAL(WITH_OPENSSL, [test x"$with_openssl" != xno])

AC_ARG_WITH([matrixssl],
 [AS_HELP_STRING([--with-matrixssl], [enable support for matrixssl])],[],[with_matrixssl=no])

AS_IF([test x"$with_matrixssl" != xno],
  [AC_CHECK_LIB([matrixssl], [matrixSslOpen],
              [AC_SUBST([LIBSSL], ["-lmatrixssl"])
               AC_DEFINE([HAVE_MATRIXSSL], [1],
                         [Define if you have matrixssl])
              ],
              [AC_MSG_FAILURE(
                 [--with-matrixssl was given, but test for matrixssl failed])],
              [-lmatrixssl])])

AS_IF([test x"$with_matrixssl" != xno],
  [AC_CHECK_HEADERS([matrixSsl.h matrixSsl/matrixSsl.h])])

AM_CONDITIONAL(WITH_MATRIXSSL, [test x"$with_matrixssl" != xno])

AC_ARG_WITH([cyassl],
 [AS_HELP_STRING([--with-cyassl], [enable support for cyassl])],[],[with_cyassl=no])

AC_ARG_WITH([cyassl],
 [AS_HELP_STRING([--with-cyassl], [enable support for cyassl])],[],[with_cyassl=no])

AS_IF([test x"$with_cyassl" != xno],
  [AC_CHECK_LIB([cyassl], [CyaSSL_Init],
              [AC_SUBST([LIBSSL], ["-lcyassl"])
               AC_DEFINE([HAVE_CYASSL], [1],
                         [Define if you have cyassl])
              ],
              [AC_MSG_FAILURE(
                 [--with-cyassl was given, but test for cyassl failed])],
              [-lcyassl])])

AS_IF([test x"$with_cyassl" != xno],
  [AC_CHECK_HEADERS([cyassl/ssl.h ssh.h])])

AM_CONDITIONAL(WITH_CYASSL, [test x"$with_cyassl" != xno])

AM_CONDITIONAL(WITH_SSL, [test x"$with_openssl" != xno || test x"$with_matrixssl" != xno || test x"$with_cyassl" != xno])

AC_ARG_WITH([matrixssl-cli],
 [AS_HELP_STRING([--with-matrixssl-cli], [enable matrixssl client use])],[],[with_matrixssl_cli=no])

AM_CONDITIONAL(WITH_MATRIXSSL_CLI, [test x"$with_matrixssl_cli" != xno])

AC_ARG_WITH([nfqueue],
 [AS_HELP_STRING([--with-nfqueue], [enable support for netfilter_queue])],[],[with_nfqueue=no])

AS_IF([test x"$with_nfqueue" != xno],
  [AC_CHECK_LIB([netfilter_queue], [nfq_open],
              [AC_SUBST([LIBNETFILTER_QUEUE], ["-lnetfilter_queue  -lnfnetlink -lmnl"])
               AC_DEFINE([HAVE_NETFILTER_QUEUE], [1],
                         [Define if you have netfilter_queue])
              ],
              [AC_MSG_FAILURE(
                 [--with-nfqueue was given, but test for netfilter_queue failed])],
              [-lnetfilter_queue -lnfnetlink -lmnl])])

AM_CONDITIONAL(WITH_NETFILTER_QUEUE, [test x"$with_nfqueue" != xno])

AC_ARG_WITH([avl],
 [AS_HELP_STRING([--with-avl], [enable support for avl library])],[],[with_avl=no])

AS_IF([test x"$with_avl" != xno],
  [AC_DEFINE([HAVE_AVL], [1], [Define to use avl library])])

AM_CONDITIONAL(WITH_AVL, [test x"$with_avl" != xno])

AC_ARG_WITH([nfcoova],
 [AS_HELP_STRING([--with-nfcoova], [enable support for coova netfilter module])],[],[with_nfcoova=no])

AS_IF([test x"$with_nfcoova" != xno],
  [AC_DEFINE([HAVE_NETFILTER_COOVA], [1], [Define to use coova kernel module])])

AM_CONDITIONAL(WITH_NETFILTER_COOVA, [test x"$with_nfcoova" != xno])

AC_ARG_WITH([sfhash],
 [AS_HELP_STRING([--without-sfhash], [disable SuperFastHash use])],[],[with_sfhash=yes])

AS_IF([test x"$with_sfhash" != xno],
  [AC_DEFINE([HAVE_SFHASH], [1], [Define to use SuperFastHash])])

AM_CONDITIONAL(WITH_SFHASH, [test x"$with_sfhash" != xno])

AC_ARG_WITH([lookup3],
 [AS_HELP_STRING([--with-lookup3], [enable Jenkins lookup3 use])],[],[with_lookup3=no])

AS_IF([test x"$with_lookup3" != xno],
  [AC_DEFINE([HAVE_LOOKUP3], [1], [Define to use Jenkins lookup3])])

AM_CONDITIONAL(WITH_LOOKUP3, [test x"$with_lookup3" != xno])

AC_ARG_WITH([patricia],
 [AS_HELP_STRING([--with-patricia], [enable Patricia use])],[],[with_patricia=no])

AS_IF([test x"$with_patricia" != xno],
  [AC_DEFINE([HAVE_PATRICIA], [1], [Define to include Patricia])])

AM_CONDITIONAL(WITH_PATRICIA, [test x"$with_patricia" != xno])


AC_ARG_ENABLE([authedallowed],
 [AS_HELP_STRING([--enable-authedallowed], [enable Authurized Garden])],[],[with_authedallowed=no])

AS_IF([test x"$with_authedallowed" != xno],
  [AC_DEFINE([ENABLE_AUTHEDALLOWED], [1], [Define to include Authenticated Garden])])

AM_CONDITIONAL(WITH_AUTHEDALLOWED, [test x"$with_authedallowed" != xno])


AC_ARG_WITH([ipv6],
 [AS_HELP_STRING([--without-ipv6], [enable IPv6])],[],[with_ipv6=yes])

AS_IF([test x"$with_ipv6" != xno],
  [AC_DEFINE([ENABLE_IPV6], [1], [Define to use IPv6])])

AM_CONDITIONAL(WITH_IPV6, [test x"$with_ipv6" != xno])

AC_ARG_WITH([pcap],
 [AS_HELP_STRING([--with-pcap], [enable support for pcap])],[],[with_pcap=no])

AS_IF([test x"$with_pcap" != xno],
  [AC_CHECK_LIB([pcap], [pcap_open_live],
              [AC_SUBST([LIBPCAP], ["-lpcap"])
               AC_DEFINE([USING_PCAP], [1],
                         [Define if you have pcap enabled])
              ],
              [AC_MSG_FAILURE(
                 [--with-pcap was given, but test for pcap failed])],
              [-lpcap])])

AM_CONDITIONAL(WITH_PCAP, [test x"$with_pcap" != xno])

AC_ARG_WITH([curl],
 [AS_HELP_STRING([--with-curl], [enable support for curl])],[],[with_curl=no])

AS_IF([test x"$with_curl" != xno],
  [AC_CHECK_LIB([curl], [curl_global_init],
              [AC_SUBST([LIBCURL], ["-lcurl -lz -lssl -lcrypto -lcares"])
               AC_DEFINE([USING_CURL], [1],
                         [Define if you have curl enabled])
              ],
              [AC_MSG_FAILURE(
                 [--with-curl was given, but test for curl failed])],
              [-lz -lssl -lcrypto -lcares])])

AM_CONDITIONAL(WITH_CURL, [test x"$with_curl" != xno])

AC_ARG_WITH([mmap],
 [AS_HELP_STRING([--with-mmap], [enable support for mmap])],[],[with_mmap=no])

AS_IF([test x"$with_mmap" != xno],
  [AC_DEFINE([USING_MMAP], [1], [Define if you have mmap enabled])])

AC_ARG_WITH([poll],
 [AS_HELP_STRING([--with-poll], [enable support for poll])],[],[with_poll=no])

AS_IF([test x"$with_poll" != xno],
  [AC_DEFINE([USING_POLL], [1], [Define if you have poll() enabled])])

AC_ARG_WITH([ipc-msg],
 [AS_HELP_STRING([--with-ipc-msg], [enable support for msgsnd/msgrcv SV IPC])],[],[with_ipcmsg=no])

AS_IF([test x"$with_ipcmsg" != xno],
  [AC_DEFINE([USING_IPC_MSG], [1], [Define to use SV IPC message queue])])

AC_ARG_ENABLE(binstatusfile, [AS_HELP_STRING([--enable-binstatusfile],[Enable support for binary status file])], 
  enable_binstatfile=$enableval, enable_binstatfile=no)

if test x"$enable_binstatfile" = xyes; then
   AC_DEFINE(ENABLE_BINSTATFILE,1,[Define to enable binary status file])
fi

AC_ARG_ENABLE(statusfile, [AS_HELP_STRING([--enable-statusfile],[Enable support for status file])], 
  enable_statfile=$enableval, enable_statfile=no)

if test x"$enable_statfile" = xyes || test x"$enable_binstatfile" = xyes; then
   AC_DEFINE(ENABLE_STATFILE,1,[Define to enable status file])
fi

AC_ARG_ENABLE(chilliproxy, [AS_HELP_STRING([--enable-chilliproxy],[Enable support for HTTP AAA Proxy])], 
  enable_chilliproxy=$enableval, enable_chilliproxy=no)

if test x"$enable_chilliproxy" = xyes; then
   AC_DEFINE(ENABLE_CHILLIPROXY,1,[Define to enable HTTP AAA Proxy])
fi

AM_CONDITIONAL(WITH_CHILLIPROXY, [test x"$enable_chilliproxy" = xyes])

AC_ARG_ENABLE(multiroute, [AS_HELP_STRING([--enable-multiroute],[Enable support for multiple routes])], 
  enable_multiroute=$enableval, enable_multiroute=no)

if test x"$enable_multiroute" = xyes; then
   AC_DEFINE(ENABLE_MULTIROUTE,1,[Define to enable multiple routes])
fi

AM_CONDITIONAL(WITH_MULTIROUTE, [test x"$enable_multiroute" = xyes])

AC_ARG_ENABLE(multilan, [AS_HELP_STRING([--enable-multilan],[Enable support for multiple LANs])], 
  enable_multilan=$enableval, enable_multilan=no)

if test x"$enable_multilan" = xyes; then
   AC_DEFINE(ENABLE_MULTILAN,1,[Define to enable multiple LANs])
fi

AM_CONDITIONAL(WITH_MULTILAN, [test x"$enable_multilan" = xyes])

AC_ARG_ENABLE(chilliradsec, [AS_HELP_STRING([--enable-chilliradsec],[Enable support for RadSec AAA Proxy])], 
  enable_chilliradsec=$enableval, enable_chilliradsec=no)

if test x"$enable_chilliradsec" = xyes; then
   AC_DEFINE(ENABLE_CHILLIRADSEC,1,[Define to enable RadSec AAA Proxy])
fi

AM_CONDITIONAL(WITH_CHILLIRADSEC, [test x"$enable_chilliradsec" = xyes])

AC_ARG_ENABLE(chilliredir, [AS_HELP_STRING([--enable-chilliredir],[Enable support for Redir server])], 
  enable_chilliredir=$enableval, enable_chilliredir=no)

if test x"$enable_chilliredir" = xyes; then
   AC_DEFINE(ENABLE_CHILLIREDIR,1,[Define to enable Redir server])
fi

AM_CONDITIONAL(WITH_CHILLIREDIR, [test x"$enable_chilliredir" = xyes])

AC_ARG_ENABLE(database, [AS_HELP_STRING([--enable-database],[Enable support sqlite database])],
  enable_database=$enableval, enable_database=no)

if test x"$enable_database" = xyes; then
   AC_DEFINE(ENABLE_DATABASE,1,[Define to enable sqlite database])
fi

AM_CONDITIONAL(WITH_DATABASE, [test x"$enable_database" = xyes])

AC_ARG_ENABLE(gsm, [AS_HELP_STRING([--enable-gsm],[Enable gsm support])],
  enable_gsm=$enableval, enable_gsm=no)

if test x"$enable_gsm" = xyes; then
   AC_DEFINE(ENABLE_GSM,1,[Define to enable gsm support])
fi

AM_CONDITIONAL(WITH_GSM, [test x"$enable_gsm" = xyes])

AC_ARG_ENABLE(chilliscript, [AS_HELP_STRING([--enable-chilliscript],[Enable support for chilli_script helper])], 
  enable_chilliscript=$enableval, enable_chilliscript=no)

if test x"$enable_chilliscript" = xyes; then
   AC_DEFINE(ENABLE_CHILLISCRIPT,1,[Define to enable chilli_script helper])
fi

AM_CONDITIONAL(WITH_CHILLISCRIPT, [test x"$enable_chilliscript" = xyes])

AC_ARG_ENABLE(cluster, [AS_HELP_STRING([--enable-cluster],[Enable support for clustering])], 
  enable_cluster=$enableval, enable_cluster=no)

if test x"$enable_cluster" = xyes; then
   AC_DEFINE(ENABLE_CLUSTER,1,[Define to enable cluster])
fi

AM_CONDITIONAL(WITH_CLUSTER, [test x"$enable_cluster" = xyes])

AC_ARG_ENABLE(sessionstate, [AS_HELP_STRING([--enable-sessionstate],[Enable extended use of the CoovaChilli-Session-State attribute])], 
  enable_sessionstate=$enableval, enable_sessionstate=no)

if test x"$enable_sessionstate" = xyes; then
   AC_DEFINE(ENABLE_SESSIONSTATE,1,[Define to enable extended use of the CoovaChilli-Session-State attribute])
fi

AC_ARG_ENABLE(sessionid, [AS_HELP_STRING([--enable-sessionid],[Enable the use of the CoovaChilli-Session-Id attribute])], 
  enable_sessionid=$enableval, enable_sessionid=no)

if test x"$enable_sessionid" = xyes; then
   AC_DEFINE(ENABLE_SESSIONID,1,[Define to enable the use of the CoovaChilli-Session-Id attribute])
fi

AC_ARG_ENABLE(apsessionid, [AS_HELP_STRING([--enable-apsessionid],[Enable the use of the CoovaChilli-AP-Session-Id attribute])], 
  enable_apsessionid=$enableval, enable_apsessionid=no)

if test x"$enable_apsessionid" = xyes; then
   AC_DEFINE(ENABLE_APSESSIONID,1,[Define to enable the use of the CoovaChilli-AP-Session-Id attribute])
fi

AC_ARG_ENABLE(coovachilliconfig, [AS_HELP_STRING([--disable-coovachilliconfig],[Disable the use of the CoovaChilli-Config attribute])], 
  enable_coovachilliconfig=$enableval, enable_coovachilliconfig=yes)

if test x"$enable_coovachilliconfig" = xyes; then
   AC_DEFINE(ENABLE_COOVACHILLICONFIG,1,[Define to enable the use of the CoovaChilli-Config attribute])
fi

AC_ARG_ENABLE(mdns, [AS_HELP_STRING([--enable-mdns],[Enable support for MDNS])], 
  enable_mdns=$enableval, enable_mdns=no)

if test x"$enable_mdns" = xyes; then
   AC_DEFINE(ENABLE_MDNS,1,[Define to enable mDNS])
fi

AM_CONDITIONAL(WITH_MDNS, [test x"$enable_mdns" = xyes])

AC_ARG_ENABLE(netbios, [AS_HELP_STRING([--enable-netbios],[Enable support for NetBIOS])], 
  enable_netbios=$enableval, enable_netbios=no)

if test x"$enable_netbios" = xyes; then
   AC_DEFINE(ENABLE_NETBIOS,1,[Define to enable NetBIOS])
fi

AM_CONDITIONAL(WITH_NETBIOS, [test x"$enable_netbios" = xyes])

AC_ARG_ENABLE(ieee8023, [AS_HELP_STRING([--enable-ieee8023],[Enable support for IEEE 802.3 Ethernet])], 
  enable_ieee8023=$enableval, enable_ieee8023=no)

if test x"$enable_ieee8023" = xyes; then
   AC_DEFINE(ENABLE_IEEE8023,1,[Define to enable IEEE 802.3])
fi

AM_CONDITIONAL(WITH_IEEE8023, [test x"$enable_ieee8023" = xyes])

AC_ARG_ENABLE(pppoe, [AS_HELP_STRING([--enable-pppoe],[Enable support for PPPoE])], 
  enable_pppoe=$enableval, enable_pppoe=no)

if test x"$enable_pppoe" = xyes; then
   AC_DEFINE(ENABLE_PPPOE,1,[Define to enable PPPoE])
fi

AM_CONDITIONAL(WITH_PPPOE, [test x"$enable_pppoe" = xyes])

AC_ARG_ENABLE(l2tpppp, [AS_HELP_STRING([--enable-l2tpppp],[Enable support for L2TP/PPP Tunneling])], 
  enable_l2tpppp=$enableval, enable_l2tpppp=no)

if test x"$enable_l2tpppp" = xyes; then
   AC_DEFINE(ENABLE_L2TP_PPP,1,[Define to enable L2TP/PPP])
fi

AM_CONDITIONAL(WITH_L2TP_PPP, [test x"$enable_l2tpppp" = xyes])

AC_ARG_ENABLE(eapol, [AS_HELP_STRING([--enable-eapol],[Enable support for EAPOL])], 
  enable_eapol=$enableval, enable_eapol=no)

if test x"$enable_eapol" = xyes; then
   AC_DEFINE(ENABLE_EAPOL,1,[Define to enable EAPOL])
fi

AM_CONDITIONAL(WITH_EAPOL, [test x"$enable_eapol" = xyes])

AC_ARG_ENABLE(miniportal, [AS_HELP_STRING([--enable-miniportal],[Enable support Coova miniportal])], 
  enable_miniportal=$enableval, enable_miniportal=no)

if test x"$enable_miniportal" = xyes ; then
   AC_DEFINE(ENABLE_MINIPORTAL,1,[Define to enable Coova miniportal])
fi

AM_CONDITIONAL(WITH_MINIPORTAL, [test x"$enable_miniportal" = xyes])

AC_ARG_ENABLE(miniconfig, [AS_HELP_STRING([--enable-miniconfig],[Enable support minimal cmdline config])], 
  enable_miniconfig=$enableval, enable_miniconfig=no)

if test x"$enable_miniconfig" = xyes ; then
   AC_DEFINE(ENABLE_MINICONFIG,1,[Define to enable minimal cmdline config])
fi

AM_CONDITIONAL(WITH_MINICONFIG, [test x"$enable_miniconfig" = xyes])

AC_ARG_ENABLE(ewtapi, [AS_HELP_STRING([--enable-ewtapi],[Enable support for CoovaEWT API])], 
  enable_ewtapi=$enableval, enable_ewtapi=no)

if test x"$enable_ewtapi" = xyes; then
   AC_DEFINE(ENABLE_EWTAPI,1,[Define to enable CoovaEWT API])
fi

AC_ARG_ENABLE(libjson, [AS_HELP_STRING([--enable-libjson],[Enable support for libjson])], 
  enable_libjson=$enableval, enable_libjson=no)

AM_CONDITIONAL(WITH_EWTAPI, [test x"$enable_ewtapi" = xyes])
AM_CONDITIONAL(WITH_JSONLIB, [test x"$enable_ewtapi" = xyes || test x"$enable_libjson" = xyes])

AC_ARG_ENABLE(ssdp, [AS_HELP_STRING([--enable-ssdp],[Enable support for Simple Service Discovery Protocol])], 
  enable_ssdp=$enableval, enable_ssdp=no)

if test x"$enable_ssdp" = xyes ; then
   AC_DEFINE(ENABLE_SSDP,1,[Define to enable Simple Service Discovery Protocol])
fi

AC_ARG_ENABLE(layer3, [AS_HELP_STRING([--enable-layer3],[Enable support for Layer3 only operation])], 
  enable_layer3=$enableval, enable_layer3=no)

if test x"$enable_layer3" = xyes ; then
   AC_DEFINE(ENABLE_LAYER3,1,[Define to enable Layer3 only support])
fi

AC_ARG_ENABLE(modules, [AS_HELP_STRING([--enable-modules],[Enable dynamically loadable modules (experimental)])], 
  enable_modules=$enableval, enable_modules=no)

if test x"$enable_modules" = xyes ; then
   AC_DEFINE(ENABLE_MODULES,1,[Define to enable dynamically loadable modules])
fi

AM_CONDITIONAL(WITH_MODULES, [test x"$enable_modules" = xyes])

AC_ARG_ENABLE(extadmvsa, [AS_HELP_STRING([--enable-extadmvsa],[Enable admin-user VSA attribute support])], 
  enable_extadmvsa=$enableval, enable_extadmvsa=no)

if test x"$enable_extadmvsa" = xyes; then
   AC_DEFINE(ENABLE_EXTADMVSA,1,[Define to enable admin-user VSA support])
fi

AC_ARG_ENABLE(redirinject, [AS_HELP_STRING([--enable-redirinject],[Enable Redir content-injection support])], 
  enable_redirinject=$enableval, enable_redirinject=no)

if test x"$enable_redirinject" = xyes; then
   AC_DEFINE(ENABLE_REDIRINJECT,1,[Define to Redir content-injection support])
fi

AM_CONDITIONAL(WITH_REDIRINJECT, [test x"$enable_redirinject" = xyes])

AC_ARG_ENABLE(netnat, [AS_HELP_STRING([--enable-netnat],[Enable net interface nat (experimental)])], 
  enable_netnat=$enableval, enable_netnat=no)

if test x"$enable_netnat" = xyes ; then
   AC_DEFINE(ENABLE_NETNAT,1,[Define to enable network interface nat])
fi

AM_CONDITIONAL(WITH_NETNAT, [test x"$enable_netnat" = xyes])

AC_ARG_ENABLE(useragent, [AS_HELP_STRING([--enable-useragent],[Enable recording user-agent])], 
  enable_useragent=$enableval, enable_useragent=no)

if test x"$enable_useragent" = xyes ; then
   AC_DEFINE(ENABLE_USERAGENT,1,[Define enable the recording of user-agent])
fi

AC_ARG_ENABLE(acceptlanguage, [AS_HELP_STRING([--enable-acceptlanguage],[Enable recording Accept-Language])], 
  enable_acceptlanguage=$enableval, enable_acceptlanguage=no)

if test x"$enable_acceptlanguage" = xyes ; then
   AC_DEFINE(ENABLE_ACCEPTLANGUAGE,1,[Define enable the recording of accept-language])
fi

AC_ARG_ENABLE(location, [AS_HELP_STRING([--enable-location],[Enable Location Awareness])], 
  enable_location=$enableval, enable_location=no)

if test x"$enable_location" = xyes ; then
   AC_DEFINE(ENABLE_LOCATION,1,[Define enable Location Awareness])
fi

AM_CONDITIONAL(WITH_LOCATION, [test x"$enable_location" = xyes])


AC_ARG_ENABLE(forcedns, [AS_HELP_STRING([--enable-forcedns],[Enable the forcing (NAT) of DNS])], 
  enable_forcedns=$enableval, enable_forcedns=no)

if test x"$enable_forcedns" = xyes ; then
   AC_DEFINE(ENABLE_FORCEDNS,1,[Enable the forcing (NAT) of DNS])
fi


AC_CHECK_LIB(rt, clock_gettime)

AM_CONDITIONAL([HAVE_STRLCPY], [test "x$ac_cv_func_strlcpy" = xyes])

AC_ARG_ENABLE(config,
 [  --enable-config=file],
 [ if  test "x$enableval" != "x" ; then
      config_file=$enableval
   else
      echo "Error! Needs an argument"
      exit -1
   fi
 ])

if test x"$config_file" != x ; then
   AC_DEFINE_UNQUOTED(ENABLE_CONFIG,"$config_file",[none])
fi

AM_CONDITIONAL(WITH_CONFIG, [test x"$enable_config" = xyes])

AC_SUBST(sysconfdir)
AC_CONFIG_FILES([Makefile
		 lchilli/Makefile 
		 bstring/Makefile 
		 conf/Makefile
		 debian/Makefile
		 distro/Makefile
		 doc/Makefile
		 json/Makefile
		 miniportal/Makefile
		 src/Makefile 
		 src/mssl/Makefile 
		 www/Makefile 
		 distro/suse/coova-chilli.spec
		 distro/redhat/coova-chilli.spec])

AC_OUTPUT
