/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/


#ifndef TW_C_SDK_TWCONSTANTS_H
#define TW_C_SDK_TWCONSTANTS_H

#define TW_NO_DESCRIPTION (char*)NULL
#define TW_NO_DATA_SHAPE NULL
#define TW_NO_USER_DATA NULL
#define TW_NO_TIMESTAMP NULL

#define TW_QUALITY_GOOD "GOOD"
#define TW_QUALITY_UNKNOWN "UNKNOWN"
#define TW_QUALITY_BAD "BAD"
#define TW_QUALITY_OUT_OF_RANGE "OUT_OF_RANGE"
#define TW_QUALITY_UNVERIFIED_SOURCE "UNVERIFIED_SOURCE"
#define TW_NO_NAMESPACE NULL
#define TW_NO_SHAPE_NAME NULL

#define TW_THREAD_AUTOSTART TRUE

/**
 * \defgroup Aspects Constants for PropertyDefinition
 * @{
 */
#define TW_ASPECT_ISPERSISTENT "isPersistent"
#define TW_ASPECT_ISBUILTIN "isBuiltIn"
#define TW_ASPECT_ISREADONLY "isReadOnly"
#define TW_ASPECT_DATASHAPE "dataShape"
#define TW_ASPECT_ISLOGGED "isLogged"
#define TW_ASPECT_PUSHTYPE  "pushType"
#define TW_ASPECT_DATACHANGETYPE  "dataChangeType"
#define TW_ASPECT_DATACHANGETHRESHOLD "dataChangeThreshold"
#define TW_ASPECT_ISFOLDED "isFolded"
#define TW_ASPECT_DEFAULT_VALUE "defaultValue"
#define TW_OBSERVE_ALL_PROPERTIES NULL
/**@}*/

/**
 * \defgroup Property Push Constants
 * @{
 */
#define TW_PUSH_CONNECT_FORCE TRUE
#define TW_PUSH_CONNECT_LATER FALSE
#define TW_PUSH_LATER FALSE
#define TW_PUSH_NOW TRUE
#define TW_PUSH_ALL_THINGS NULL
/**@}*/

/**
 * \defgroup Push Types and values for TW_ASPECT_PUSH_TYPE
 * @{
 */
#define TW_PUSH_TYPE_ALWAYS "ALWAYS"
#define TW_PUSH_TYPE_VALUE "VALUE"
#define TW_PUSH_TYPE_NEVER "NEVER"
#define TW_PUSH_THRESHOLD_NONE 0
#define TW_FOLD_TYPE_NO FALSE
#define TW_FOLD_TYPE_YES TRUE
#define REPORT_ALL_CHANGES 0
#define TW_SHAPE_NAME_NONE NULL
/**@}*/


#define TW_THING_TEMPLATE_GENERIC "GenericEdgeThingTemplate"
#define TW_THING_TEMPLATE_EMPTY "EmptyTemplate"
#define TW_NO_RETURN_DATASHAPE NULL
#define TW_NO_PARAMETERS NULL

#endif //TW_C_SDK_TWCONSTANTS_H
