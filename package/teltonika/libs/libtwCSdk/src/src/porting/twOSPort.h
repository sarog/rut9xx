/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twOSPort.h
 * \brief Wrappers for OS-specific functionality
*/

#ifndef TW_OS_PORT_H
#define TW_OS_PORT_H

#include <stdint.h>
#include <stdlib.h>
#include <twPasswds.h>


#include "twConfig.h"
#include "twDefaultSettings.h"
#include "twDefinitions.h"
#include TW_OS_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

/* Thingworx integration - add axtls certificate file size limit to avoid integer overflows*/
#define TW_AXTLS_MAX_CERT_SIZE 16384

/**
 * \name Logging functions
*/
/**
 * \brief See twLogger.h.
*/
char * levelString(enum LogLevel level);
/**
 * \brief See twLogger.h.
*/
void LOGGING_FUNCTION( enum LogLevel level, const char * timestamp, const char * message );

/**
 * \name Time Functions
 *
 * \brief Time is represented as a 64 bit value representing milliseconds since
 * the epoch.
*/
/**
 * \brief Compares two #DATETIME variables to see if one is greater.
 *
 * \param[in]     t1        The #DATETIME to compare against.
 * \param[in]     t1        The #DATETIME to compare with.
 *
 * \return #TRUE if t1 > t2, #FALSE otherwise.
*/ 
char twTimeGreaterThan(DATETIME t1, DATETIME t2);

/**
 * \brief Compares two #DATETIME variables to see if one is smaller.
 *
 * \param[in]     t1        The #DATETIME to compare against.
 * \param[in]     t1        The #DATETIME to compare with.
 *
 * \return #TRUE if t1 < t2, #FALSE otherwise.
*/ 
char twTimeLessThan(DATETIME t1, DATETIME t2);

/**
 * \brief Adds milliseconds to a #DATETIME.
 *
 * \param[in]     t1        The #DATETIME to add to.
 * \param[in]     msec      The number of milliseconds to add.
 *
 * \return \p t1 advanced by \p msec milliseconds.
*/
DATETIME twAddMilliseconds(DATETIME t1, int32_t msec);

/**
 * \brief Gets the current system time.
 *
 * \param[in]     utc       Currently unused.
 *
 * \return The current system time.
*/
DATETIME twGetSystemTime(char utc);

/**
 * \brief Gets the current system time in milliseconds.
 *
 * \return The current system time in milliseconds.
*/
uint64_t twGetSystemMillisecondCount();

/**
 * \brief Gets the current system time as a string.
 *
 * \param[out,in] s         A pointer to the string to write the time to.
 * \param[in]     format    A string describing how to format the time (via
 *                          sprintf()).
 * \param[in]     length    The length of the string.
 * \param[in]     msec      If #TRUE, get time in milliseconds.
 * \param[in]     utc       If #TRUE, get coordinated universal time.
 *
 * \return Nothing.
*/
void twGetSystemTimeString(char * s, const char * format, int length, char msec, char utc);

/**
 * \brief Converts a #DATETIME to a string.
 *
 * \param[in]     time      The #DATETIME to convert.
 * \param[in,out] s         A pointer to the string to write the time to.
 * \param[in]     format    A string describing how to format the time (via
 *                          sprintf()).
 * \param[in]     length    The length of the string.
 * \param[in]     msec      If #TRUE, get time in milliseconds.
 * \param[in]     utc       If #TRUE, get coordinated universal time.
 *
 * \return Nothing.
*/
void twGetTimeString(DATETIME time, char * s, const char * format, int length, char msec, char utc);
void twSleepMsec(int milliseconds);

/**
 * \name Mutex Functions
*/
/**
 * \brief Creates a new ::twMutex.
 *
 * \return A pointer to the newly allocated ::twMutex.
 *
 * \note The calling function will gain ownership of the returned ::twMutex and
 * is responsible for freeing it via twMutex_Delete().
*/
TW_MUTEX twMutex_Create();

/**
 * \brief Frees all memory associated with a ::twMutex and all of its owned
 * substructures.
 *
 * \param[in]     m      A pointer to the ::twMutex to delete.
 *
 * \return Nothing.
*/
void twMutex_Delete(TW_MUTEX m);

/**
 * \brief Locks a ::twMutex.
 *
 * \param[in]     m     A pointer to the ::twMutex to lock.
 *
 * \return Nothing.
*/
void twMutex_Lock(TW_MUTEX m);

/**
 * \brief Unlocks a ::twMutex.
 *
 * \param[in]     m     A pointer to the ::twMutex to unlock.
 *
 * \return Nothing.
*/
void twMutex_Unlock(TW_MUTEX m);

/**
 * \name Sockets
*/
#define CLOSED (char)0
#define OPEN (char)1

/**
 * \brief ::twSocket base type definition.
*/
typedef struct twSocket {
	TW_SOCKET_TYPE sock;     /**< Socket descriptor. **/
	TW_ADDR_INFO addr;       /**< The address to use **/
	TW_ADDR_INFO * addrInfo; /**< #TW_ADDR_INFO struct head - use to free. **/
	char state;              /**< The current state of the ::twSocket. **/
	char * host;             /**< The host name of the server. **/
	uint16_t port;           /**< The port the server is listening on. **/
	char * proxyHost;        /**< The host name of the proxy. **/
	uint16_t proxyPort;      /**< The port of the proxy. **/
	char * proxyUser;        /**< The username to use to authenticate with the proxy. **/
	twPasswdCallbackFunction proxyPassCallback; /**< The password callback to use to obtain the proxy password from the
 										 * client authenticate with the proxy. **/
} twSocket;

#ifndef MSG_NOSIGNAL
/**
 * \brief MSG_NOSIGNAL is not defined on some implementations.
*/
#define MSG_NOSIGNAL 0
#endif

/**
 * \brief Creates a new ::twSocket.
 *
 * \param[in]     host      The host name of the server.
 * \param[in]     port      The port the server is listening on.
 * \param[in]     options   Currently unused.
 *
 * \return A pointer to the newly allocated ::twSocket.
 *
 * \note The calling function will gain ownership of the returned ::twSocket
 * and is responsible for freeing it via twSocket_Delete().
*/
twSocket * twSocket_Create(const char * host, int16_t port, uint32_t options);

/**
 * \brief Connects a ::twSocket.
 *
 * \param[in]     s         A pointer to the ::twSocket to connect.
 *
 * \return Upon successful completion, a value of 0 is returned.  Otherwise, a
 * value of -1 is returned.
*/ 
int twSocket_Connect(twSocket * s);

/**
 * \brief Reconnects a ::twSocket.
 *
 * \param[in]     s         A pointer to the ::twSocket to reconnect.
 *
 * \return Upon successful completion, a value of 0 is returned.  Otherwise, a
 * value of -1 is returned.
*/
int twSocket_Reconnect(twSocket * s);

/**
 * \brief Closes a ::twSocket.
 *
 * \param[in]     s         A pointer to the ::twSocket to close.
 *
 * \return Upon successful completion, a value of 0 is returned.  Otherwise, a
 * value of -1 is returned.
*/
int twSocket_Close(twSocket * s);

/**
 * \brief Checks to see if a ::twSocket is ready for I/O.
 *
 * \param[in]     s         A pointer to the ::twSocket to check.
 * \param[in]     timeout   The amount of time (in milliseconds) to wait.
 *
 * \return 0 if ready, 1 if not ready, -1 if an error was encountered.
*/
int twSocket_WaitFor(twSocket * s, int timeout);

/**
 * \brief Reads data from a ::twSocket.
 *
 * \param[in]     s         A pointer to the ::twSocket to read from.
 * \param[out]    buf       A buffer to store the read data.
 * \param[in]     len       The length of data to be read.
 * \param[in]     timeout   The amount of time (in milliseconds) to wait for
 *                          I/O.
 *
 * \return The number of bytes read or -1 if an error was encountered.
*/ 
int twSocket_Read(twSocket * s, char * buf, int len, int timeout);

/**
 * \brief Writes data to a ::twSocket.
 *
 * \param[in]     s         A pointer to the ::twSocket to write to.
 * \param[in]     buf       A buffer to of data to write.
 * \param[in]     len       The length of data to be written.
 * \param[in]     timeout   The amount of time (in milliseconds) to wait for
 *                          I/O.
 *
 * \return The number of bytes written or -1 if an error was encountered.
*/ 
int twSocket_Write(twSocket * s, char * buf, int len, int timeout);

/**
 * \brief Frees all memory associated with a ::twSocket.
 *
 * \param[in]     s         A pointer to the ::twSocket to be deleted.
 *
 * \return 0 on success, -1 if an error was encountered.
*/
int twSocket_Delete(twSocket * s);

/**
 * \brief Gets the errno of the last operation.
 *
 * \return The errno of the last operation.
*/
int twSocket_GetLastError();

/* Enabling proxy support adds ~5KB to the code footprint, #define USE_NTLM_PROXY support adds another ~40KB */
#ifdef ENABLE_HTTP_PROXY_SUPPORT
/**
 * \brief Sets the proxy information of a ::twSocket.
 *
 * \param[in]     s         A pointer to the ::twSocket to be modified.
 * \param[in]     proxyHost The host name of the proxy server.
 * \param[in]     proxyPort The port the proxy server is listening on.
 * \param[in]     proxyUser The user name used to authenticate with the proxy.
 * \param[in]     passwdCallbackFunction The password used to authenticate with the proxy.
 *
 * \return 0 if successful, positive integral on error code (see twErrors.h) if
 * an error was encountered.
*/
int twSocket_SetProxyInfo(twSocket * s,char * proxyHost, uint16_t proxyPort, char * proxyUser, twPasswdCallbackFunction passwdCallbackFunction);

/**
 * \brief Clears the proxy information of a ::twSocket.
 *
 * \param[in]     s         A pointer to the ::twSocket to be modified.
 *
 * \return 0 if successful, positive integral on error code (see twErrors.h) if
 * an error was encountered.
*/
int twSocket_ClearProxyInfo(twSocket * s);
#endif

/**
 * \name Thread/Task Functions
*/
/**
 * \brief Starts the ::twTasker.
*/
void twTasker_Start();
/**
 * \brief Stops the ::twTasker.
**/
void twTasker_Stop();

/**
 * \name File Transfer Functions
*/
/**
 * \brief Gets information about a file.
 *
 * \param[in]     filename      The full path of the file or directory to get
 *                              the information of.
 * \param[out]    size          A pointer to an integer to store the size of
 *                              the file in.
 * \param[out]    lastModified  A pointer to a #DATETIME to store the date/time
 *                              of the last modification to the file in.
 * \param[out]    isDirectory   A pointer to a char to store #TRUE if the file
 *                              is a directory or #FALSE if it isn't.
 * \param[out]    isReadOnly    A pointer to a char to store #TRUE if the file
 *                              is read-only or #FALSE if it isn't.
 *
 * \return 0 on success or an appropriate errno if an error was encountered.
 *
 * \note The calling function will retain ownership of all pointers passed to
 * twDirectory_GetFileInfo().
*/
int twDirectory_GetFileInfo(char * filename, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly);

/**
 * \brief Checks to see if a file exists.
 *
 * \param[in]     name          The full path of the file to check.
 *
 * \return #TRUE if the file exists, #FALSE otherwise.
*/
char twDirectory_FileExists(char * name);

/**
 * \brief Creates a new file.
 *
 * \param[in]     name          The full path of the file to create.
 *
 * \return 0 on success or an appropriate errno if an error was encountered.
*/
int twDirectory_CreateFile(char * name);

/**
 * \brief Moves an existing file.
 *
 * \param[in]     fromName      The full path of the file to move.
 * \param[in]     toName        The full path to move the file to.
 *
 * \return 0 on success or an appropriate errno if an error was encountered.
*/
int twDirectory_MoveFile(char * fromName, char * toName);
/**
 * \brief Copies an existing file to a new location.
 *
 * \param[in]     fromName      The full path of the file to copy.
 * \param[in]     toName        The full path to move the file to.
 *
 * \return 0 on success or an appropriate errno if an error was encountered.
*/
int twDirectory_CopyFile(char * fromName, char * toName);
/**
 * \brief Deletes a file.
 *
 * \param[in]     name          The full path of the file to delete.
 *
 * \return 0 on success or an appropriate errno if an error was encountered.
*/
int twDirectory_DeleteFile(char * name);

/**
 * \brief Creates a directory.
 *
 * \param[in]     name          The full path of the directory to create.
 *
 * \return 0 on success or an appropriate errno if an error was encountered.
*/
int twDirectory_CreateDirectory(char * name);

/**
 * \brief Deletes a directory.
 *
 * \param[in]     name          The full path of the directory to delete.
 *
 * \return 0 on success or an appropriate errno if an error was encountered.
*/
int twDirectory_DeleteDirectory(char * name);

/**
 * \param[in]     dirName       The full path of the directory to iterate
 *                              through.
 * \param[out]    dir           A handle for the directory.
 * \param[out]    name          A pointer to a string to store the name of the
 *                              current entry in.
 * \param[out]    size          A pointer to an integer to store the size of
 *                              the current entry in.
 * \param[out]    lastModified  A pointer to a #DATETIME to store the date/time
 *                              of the last modification to the current entry in.
 * \param[out]    isDirectory   A pointer to a char to store #TRUE if the
 *                              current entry is a directory or #FALSE if it isn't.
 * \param[out]    isReadOnly    A pointer to a char to store #TRUE if the
 *                              current entry is read-only or #FALSE if it isn't.
 *
 * \return A handle to the directory or 0 if either there are no more entries
 * in the directory or an error was encountered.
 *
 * \note If twDirectory_IterateEntries() is called on \p dirName for the first
 * time, the function will open the directory and return a handle to the opened
 * directory.  If there are no more entries in the directory the directory will
 * be closed and the function will return 0.
 * \note The calling function will retain ownership of all pointers passed to
 * twDirectory_IterateEntries().
*/
TW_DIR twDirectory_IterateEntries(char * dirName, TW_DIR dir, char ** name, uint64_t * size, 
								  DATETIME * lastModified, char * isDirectory, char * isReadOnly);

/**
 * \brief Gets the errno of the last operation.
 *
 * \return The errno of the last operation.
*/
int twDirectory_GetLastError();


#ifdef __cplusplus
}
#endif

#endif

