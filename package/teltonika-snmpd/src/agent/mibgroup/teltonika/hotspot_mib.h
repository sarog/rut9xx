/*
 *  HOTSPOT MIB group interface - hotspot.h
 *
 */

#ifndef _MIBGROUP_HOTSPOT_MIB_H
#define _MIBGROUP_HOTSPOT_MIB_H

config_require(util_funcs)


void init_hotspot_mib(void);
extern FindVarMethod var_hotspot1;
extern FindVarMethod var_hotspot2;
extern FindVarMethod var_hotspot3;
extern FindVarMethod var_hotspot4;

#define HOTSPOTID		1
#define HOTSPOTSSID		2
#define	HOTSPOTENABLED		3
#define	HOTSPOTIP		4
#define	HOTSPOTDOWNLOAD		5
#define	HOTSPOTUPLOAD		6
#define	HOTSPOTUSERS		7
#define	HOTSPOTUSERSPASS		8
#define	HOTSPOTUSERSACTIVE		9
#define HOTSPOTUSERMAC		10
#define HOTSPOTUSERIP		11
#define HOTSPOTUSERSTARTTIME	12
#define HOTSPOTUSERUSETIME	13
#define	HOTSPOTUSERDOWNLOAD	14
#define	HOTSPOTUSERUPLOAD	15
#define HOTSPOTENDTIME	16

#endif	/* _MIBGROUP_HOTSPOT_MIB_H */

