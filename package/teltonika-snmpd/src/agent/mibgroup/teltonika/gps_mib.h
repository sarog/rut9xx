/*
 *  GPS MIB group interface - gps.h
 *
 */

#ifndef _MIBGROUP_GPS_MIB_H
#define _MIBGROUP_GPS_MIB_H

config_require(util_funcs)

void init_gps_mib(void);
extern FindVarMethod var_gps;

#define LONGITUDE		1
#define LATITUDE		2
#define ACCURACY		3
#define DATETIME		4
#define NUMSAT			5


#endif	/* _MIBGROUP_GPS_MIB_H */
