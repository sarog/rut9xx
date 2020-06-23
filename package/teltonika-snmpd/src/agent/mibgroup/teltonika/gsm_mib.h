/*
 *  GSM MIB group interface - gsm.h
 *
 */

#ifndef _MIBGROUP_GSM_MIB_H
#define _MIBGROUP_GSM_MIB_H

config_require(util_funcs)

void init_gsm_mib(void);
extern FindVarMethod var_gsm;

#define IMEI		1
#define MODEL		2
#define MANUF		3
#define REVISION	4
#define SERIAL		5
#define SIMSTATE	6
#define PINSTATE	7
#define IMSI		8
#define NETSTATE	9
#define SIGNAL		10
#define OPERATOR	11
#define OPERNUM		12
#define CONNSTATE	13
#define CONNTYPE	14
#define TEMP		15
#define RXCOUNTT	16
#define TXCOUNTT	17
#define RXCOUNTY	18
#define TXCOUNTY	19
#define FWVERSION	20
#define DIGITALINPUT			21
#define DIGITALISOLATEDINPUT	22
#define ANALOGINPUT				23
#define DIGITALOCOUTPUT			24
#define DIGITALRELAYOUTPUT		25
#define SIMSLOT					26
#define ROUTERNAME				27
#define PRODUCTCODE				28
#define BATCHNUMBER				29
#define HARDWAREREVISION		30
#define ROUTERUPTIME			31
#define CONNECTIONUPTIME		32
#define MOBILEIP				33
#define SENT					34
#define RECEIVED				35
#define CELLID					36
#define	SINR					37
#define	RSRP					38
#define	RSRQ					39
#define ANALOGINPUTCALC 40

#endif	/* _MIBGROUP_GSM_MIB_H */
