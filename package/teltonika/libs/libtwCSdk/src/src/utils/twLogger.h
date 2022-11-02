/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twLogger.h
 *
 * \brief Structure definitions and function prototypes for the ThingWorx
 * logging facility.
*/

#ifndef twLogger_H /** Prevent multiple inclusions **/
#define twLogger_H

#include "twOSPort.h"
#include "twErrors.h"

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief For log message level enumeration see twDefinitions.h
**/

/**
 * \brief Signature for logging function to be called when twLog() is invoked.
 *
 * \param[in]     level         The ::LogLevel of the log message.
 * \param[in]     timestamp     The timestamp of the log message.
 * \param[in]     message       The log message text.
 *
 * \return Nothing.
*/
typedef void (*log_function) ( enum LogLevel level, const char * timestamp, const char * message);

/**
 * \brief ThingWorx Logger singleton structure definition.
*/
typedef struct twLogger {
    enum LogLevel level; /**< The ::LogLevel of the ::twLogger. **/
    log_function f;      /**< The log_function() associated with the ::twLogger. **/
	char isVerbose;      /**< If TRUE, verbose logging is enabled. **/
	char * buffer;       /**< A pointer to the ::twLogger's message buffer. **/
	TW_MUTEX mtx;        /**< A mutex associated with the ::twLogger. **/
} twLogger;

/**
 * \brief Gets a pointer to the ::twLogger singleton and creates a new one if
 * it hasn't been created already.
 *
 * \return A pointer to the new or existing ::twLogger singleton.
*/
twLogger * twLogger_Instance();

/**
 * \brief Frees all memory associated with the ::twLogger singleton.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twLogger_Delete();

/**
 * \brief Sets the ::twLogger#LogLevel of the ::twLogger singleton.
 *
 * \param[in]     level     The ::LogLevel to set the ::twLogger to.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twLogger_SetLevel(enum  LogLevel level);

/**
 * \brief Sets the ::twLogger#log_function of the ::twLogger singleton.
 *
 * \param[in]     f         The log_function() to use.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twLogger_SetFunction(log_function f);

/**
 * \brief Sets the ::twLogger#isVerbose flag of the ::twLogger singleton.
 *
 * \param[in]     val       TRUE enables verbose logging, FALSE disables
 *                          verbose logging.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twLogger_SetIsVerbose(char val);

/**
 * \brief Logs a message.
 *
 * \param[in]     level     The ::LogLevel of the message.
 * \param[in]     format    The format of the message.
 * \param[in]     ...       A va_list of messages to log.
 *
 * \return Nothing.
*/
#ifndef __GNUC__
void twLog(enum LogLevel level, const char * format, ... );
#else
/* Add checking of the var-args with the format string, a GNU CC compiler only option */
void twLog(enum LogLevel level, const char *format, ...) __attribute__((format(printf, 2, 3)));
#endif

/**
 * \brief Logs a hex string message.
 *
 * \param[in]     msg       The message to log.
 * \param[in]     preamble  #TRUE enables message preamble, #FALSE disables
 *                          message preamble.
 * \param[in]     length    The length of the message.
 *
 * \return Nothing.
*/
void twLogHexString(const char * msg, char * preamble, size_t length);

/**
 * \brief Logs a generic string message.
 *
 * \param[in]     m         The message to log.
 * \param[in]     preamble  #TRUE enables message preamble, #FALSE disables
 *                          message preamble.
 *
 * \return Nothing.
*/
void twLogMessage(void * m, char * preamble);

/* Helper functions for creating log messages */

/**
 * \brief Helper function to convert a ::msgCodeEnum to a string.
 *
 * \param[in]     m     The ::msgCodeEnum to convert.
 *
 * \return A string corresponding to \p m.
 *
 * \note The returned string is allocated on the stack.
*/
char * twCodeToString(enum msgCodeEnum m);

/**
 * \brief Helper function to convert a ::entityTypeEnum to a string.
 *
 * \param[in]     m     The ::entityTypeEnum to convert.
 *
 * \return A string corresponding to \p m.
 *
 * \note The returned string is allocated on the stack.
*/
char * twEntityToString(enum entityTypeEnum m);

/**
 * \brief Helper function to convert a ::characteristicEnum to a string.
 *
 * \param[in]     m     The ::characteristicEnum to convert.
 *
 * \return A string corresponding to \p m.
 *
 * \note The returned string is allocated on the stack.
*/
char * twCharacteristicToString(enum characteristicEnum m);

#ifdef __cplusplus
}
#endif

#endif
