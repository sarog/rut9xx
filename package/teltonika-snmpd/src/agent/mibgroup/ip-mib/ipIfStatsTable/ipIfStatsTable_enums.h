/*
 * Note: this file originally auto-generated by mib2c using
 *  : generic-table-enums.m2c 12526 2005-07-15 22:41:16Z rstory $
 *
 * $Id:$
 */
#ifndef IPIFSTATSTABLE_ENUMS_H
#define IPIFSTATSTABLE_ENUMS_H

#ifdef __cplusplus
extern          "C" {
#endif

    /*
     * NOTES on enums
     * ==============
     *
     * Value Mapping
     * -------------
     * If the values for your data type don't exactly match the
     * possible values defined by the mib, you should map them
     * below. For example, a boolean flag (1/0) is usually represented
     * as a TruthValue in a MIB, which maps to the values (1/2).
     *
     */
/*************************************************************************
 *************************************************************************
 *
 * enum definitions for table ipIfStatsTable
 *
 *************************************************************************
 *************************************************************************/

/*************************************************************
 * constants for enums for the MIB node
 * ipIfStatsIPVersion (InetVersion / ASN_INTEGER)
 *
 * since a Textual Convention may be referenced more than once in a
 * MIB, protect againt redefinitions of the enum values.
 */
#ifndef INETVERSION_ENUMS
#define INETVERSION_ENUMS

#define INETVERSION_UNKNOWN  0
#define INETVERSION_IPV4  1
#define INETVERSION_IPV6  2

#endif                          /* INETVERSION_ENUMS */




#ifdef __cplusplus
}
#endif
#endif                          /* IPIFSTATSTABLE_ENUMS_H */
