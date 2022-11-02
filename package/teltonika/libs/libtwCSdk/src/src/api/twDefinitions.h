/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twDefinitions.h
 * \brief Common definitions for C SDK
 *
 * Contains definitions for macros and enumerations relating to various message, return value,
 * type, characteristic, and service encodings.
*/

#ifndef TW_DEFINITIONS_H
#define TW_DEFINITIONS_H

#include "twErrors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name General
*/

#define TRUE 1
#define FALSE 0

/**
 * \name Messaging
*/

#define TW_MSG_VERSION 0x01

/**
 * \name File Transfer Service
*/

#define TW_VIRTUAL_STAGING_DIR "__staging__"
#define LIST_ALL 0 
#define LIST_FILES 1 
#define LIST_DIRS 2 

/**
 * \name Service arrays
*/

extern char * fileXferServices[];
extern char * tunnelServices[];

/**
 * \name Basic Message Structure Components
*/

/**
 * \brief Enumeration of HTTP message types.
*/
enum msgType {
	TW_UNKNOWN = 0,   /**< 0x00 - Unknown message type **/
	TW_REQUEST,       /**< 0x01 - Request message type **/
	TW_RESPONSE,      /**< 0x02 - Response message type **/
	TW_AUTH,          /**< 0x03 - Authorization message type **/
	TW_BIND,          /**< 0x04 - Bind message type **/
	TW_MULTIPART_REQ, /**< 0x05 - Multi-part request message type **/
	TW_MULTIPART_RESP /**< 0x06 - Multi-part response message type **/
};

/**
 * \brief Enumeration of HTTP message codes.
 * \note 0x00 - 0x1F = HTTP request methods.
 * \note 0x40 - 0x8F = HTTP client error status codes.
 * \note 0xA0 - 0xA4 = HTTP server error status codes.
*/
enum msgCodeEnum {
    /* HTTP Request Methods */
	TWX_UNKNOWN = 0,                       /**< 000 00000 - 0x00 Unknown method **/
	TWX_GET,                               /**< 000 00001 - 0x01 GET method **/
	TWX_PUT,                               /**< 000 00010 - 0x02 PUT method **/
	TWX_POST,                              /**< 000 00011 - 0x03 POST method **/
	TWX_DEL,                               /**< 000 00100 - 0x04 DELETE method **/
	TWX_BIND = 0x0A,                       /**< 000 01010 - 0x0A BIND method **/
	TWX_UNBIND,                            /**< 000 01011 - 0x0B UNBIND method **/
	TWX_AUTH = 0x14,                       /**< 000 01100 - 0x0D AUTH method **/
	TWX_KEEP_ALIVE = 0x1F,                 /**< 000 11111 - 0x1F KEEP_ALIVE method **/
    /* HTTP Client Error Status Codes */
	TWX_SUCCESS = 0x40,                    /**< 010 00000 - 0x40 (2.00) Success **/
	TWX_BAD_REQUEST = 0x80,                /**< 100 00000 - 0x80 (4.00) Bad request **/
	TWX_UNAUTHORIZED,                      /**< 100 00001 - 0x81 (4.01) Unauthorized **/
	TWX_BAD_OPTION,                        /**< 100 00010 - 0x82 (4.02) Bad option **/
	TWX_FORBIDDEN,                         /**< 100 00011 - 0x83 (4.03) Forbidden **/
	TWX_NOT_FOUND,                         /**< 100 00100 - 0x84 (4.04) Not found **/
	TWX_METHOD_NOT_ALLOWED,                /**< 100 00101 - 0x85 (4.05) Method not allowed **/
	TWX_NOT_ACCEPTABLE,                    /**< 100 00110 - 0x86 (4.06) Not acceptable **/
	TWX_PRECONDITION_FAILED = 0x8C,        /**< 100 01100 - 0x8C (4.12) Precondition failed **/
	TWX_ENTITY_TOO_LARGE,                  /**< 100 01101 - 0x8D (4.13) Entity too large **/
	TWX_UNSUPPORTED_CONTENT_FORMAT = 0x8F, /**< 100 01111 - 0x8F (4.15) Unsupported content format **/
    /* HTTP Server Error Status Codes */
	TWX_INTERNAL_SERVER_ERROR = 0xA0,      /**< 101 00000 - 0xA0 (5.00) Internal server error **/
	TWX_NOT_IMPLEMENTED,                   /**< 101 00001 - 0xA1 (5.01) Not implemented **/
	TWX_BAD_GATEWAY,                       /**< 101 00010 - 0xA2 (5.02) Bad gateway **/
	TWX_SERVICE_UNAVAILABLE,               /**< 101 00011 - 0xA3 (5.03) Service unavailable **/
	TWX_GATEWAY_TIMEOUT,                   /**< 101 00100 - 0xA4 (5.04) Gateway timeout **/
	TWX_WROTE_TO_OFFLINE_MSG_STORE,         /**< Wrote to offline message store **/
	TWX_OFFLINE_MSG_STORE_FULL				/**< Wrote to offline message store **/
};

/**
 * \name Types and Characteristics
*/

/**
 * Enumeration of entity types.
*/
enum entityTypeEnum {
	TW_UNDEFINED = 0,                  /**< 0x00 - Undefined entity **/
	TW_THING = 0x0A,                   /**< 0x0A - Thing type **/
	TW_THINGSHAPES = 0x0B,             /**< 0x0B - Thing shape type. **/
	TW_THINGTEMPLATES = 0x0C,          /**< 0x0C - Thing template type **/
	TW_THINGPACKAGES = 0x0D,           /**< 0x0D - Thing package type **/
	TW_NETWORKS = 0x0E,                /**< 0x0E - Network type **/
	TW_DATASHAPES = 0x0F,              /**< 0x0F - Data shape type **/
	TW_MODELTAGS = 0x14,               /**< 0x14 - Model tag type **/
	TW_DATATAGS = 0x15,                /**< 0x15 - Data tag type **/
	TW_MASHUPS = 0x1E,                 /**< 0x1E - Mashup type **/
	TW_WIDGETS = 0x1F,                 /**< 0x1F - Widget type **/
	TW_STYLEDEFINITIONS = 0x20,        /**< 0x20 - Style definition type **/
	TW_STATEDEFINITIONS = 0x21,        /**< 0x21 - State definition type **/
	TW_MENUS = 0x22,                   /**< 0x22 - Menu type **/
	TW_MEDIAENTITIES = 0x23,           /**< 0x23 - Media entity type **/
	TW_LOCALIZATIONTABLES = 0x24,      /**< 0x24 - Localization table type **/
	TW_DASHBOARDS = 0x27,              /**< 0x27 - Dashboard type **/
	TW_LOGS = 0x28,                    /**< 0x28 - Log type **/
	TW_USERS = 0x32,                   /**< 0x32 - User type **/
	TW_GROUPS = 0x33,                  /**< 0x33 - Group type **/
	TW_ORGANIZATIONS = 0x34,           /**< 0x34 - Organization type **/
	TW_APPLICATIONKEYS = 0x35,         /**< 0x35 - Application key type **/
	TW_DIRECTORYSERVICES = 0x36,       /**< 0x36 - Directory service type **/
	TW_RESOURCE = 0x3C,                /**< 0x3C - Resource type **/
	TW_SCRIPTFUNCTIONLIBRARIES = 0x3D, /**< 0x3D - Script function library type **/
	TW_EXTENSIONPACKAGES = 0x46,       /**< 0x46 - Extension package type **/
	TW_SUBSYSTEM = 0x50                /**< 0x50 - Subsystem type **/
};

/**
 * Enumeration of characteristics.
*/
enum characteristicEnum {
	TW_PROPERTIES = 1, /**< 1 - Property characteristic **/
	TW_SERVICES,       /**< 2 - Service characteristic **/
	TW_EVENTS          /**< 3 - Event characteristic **/
};

/**
 * Enumeration of base types.
*/
enum BaseType {
	TW_NOTHING = -1,           /**< -001 - Nothing type **/
	TW_STRING = 0,             /**< 000 - String type **/
	TW_NUMBER,                 /**< 001 - Number type **/
	TW_BOOLEAN,                /**< 002 - Boolean type **/
	TW_DATETIME,               /**< 003 - Date time type **/
	TW_TIMESPAN,               /**< 004 - Time span type **/
	TW_INFOTABLE,              /**< 005 - Info table type **/
	TW_LOCATION,               /**< 006 - Location type **/
	TW_XML,                    /**< 007 - XML type **/
	TW_JSON,                   /**< 008 - JSON type **/
	TW_QUERY,                  /**< 009 - Query type **/
	TW_IMAGE,                  /**< 010 - Image type **/
	TW_HYPERLINK,              /**< 011 - Hyperlink type **/
	TW_IMAGELINK,              /**< 012 - Imagelink type **/
	TW_PASSWORD,               /**< 013 - Password type **/
	TW_HTML,                   /**< 014 - HTML type **/
	TW_TEXT,                   /**< 015 - Text type **/
	TW_TAGS,                   /**< 016 - Tags type **/
	TW_SCHEDULE,               /**< 017 - Schedule type **/
	TW_VARIANT,                /**< 018 - Variant type **/
	TW_GUID = 20,              /**< 020 - GUID type **/
	TW_BLOB,                   /**< 021 - Blob type **/
	TW_INTEGER,                /**< 022 - Integer type **/
    TW_PROPERTYNAME = 50,      /**< 050 - Property name type **/
    TW_SERVICENAME,            /**< 051 - Service name type **/
    TW_EVENTNAME,              /**< 052 - Event name type **/
	TW_THINGNAME = 100,        /**< 100 - Thing name type **/
	TW_THINGSHAPENAME,         /**< 101 - Thing shape name type **/
	TW_THINGTEMPLATENAME,      /**< 102 - Thing template name type **/
	TW_DATASHAPENAME = 104,    /**< 104 - Data shape name type **/
	TW_MASHUPNAME,             /**< 105 - Mashup name type **/
	TW_MENUNAME,               /**< 106 - Menu name type **/
	TW_BASETYPENAME,           /**< 107 - Base type name type **/
	TW_USERNAME,               /**< 108 - User name type **/
	TW_GROUPNAME,              /**< 109 - Group name type **/
	TW_CATEGORYNAME,           /**< 110 - Category name type **/
	TW_STATEDEFINITIONNAME,    /**< 111 - State definition name type **/
	TW_STYLEDEFINITIONNAME,    /**< 112 - Style definition name type **/
	TW_MODELTAGVOCABULARYNAME, /**< 113 - Model tag vocabulary name type **/
	TW_DATATAGVOCABULARYNAME,  /**< 114 - Data tag vocabulary name type **/
	TW_NETWORKNAME,            /**< 115 - Network name type **/
	TW_MEDIAENTITYNAME,        /**< 116 - Media entity name type **/
	TW_APPLICATIONKEYNAME,     /**< 117 - Application key name type **/
	TW_LOCALIZATIONTABLENAME,  /**< 118 - Localization table name type **/
	TW_ORGANIZATIONNAME,       /**< 119 - Organization name type **/
    TW_DASHBOARDNAME,           /**< 120 - Dashboard name type **/
	TW_UNKNOWN_TYPE
};

/**
* Enumeration of Log message levels.
**/
enum LogLevel {
	TW_TRACE,	/**< Application execution tracing. **/
	TW_DEBUG,	/**< Debug messages for development. **/
	TW_INFO,	/**< General information messages. **/
	TW_WARN,	/**< Warning messages. **/
	TW_ERROR,	/**< Error messages. **/
	TW_FORCE,	/**< TBD **/
	TW_AUDIT	/**< TBD **/
};

#ifdef __cplusplus
}
#endif

#endif
