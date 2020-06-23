/*************************************
 * Copyright 2017, PTC, Inc.
 *************************************/

/**
 * \file twFileManager.h
 *
 * \brief ThingWorx file transfer structure definitions and functions
 *
 * Contains structure type definitions and function prototypes for ThingWorx
 * API file transfer operations.
*/

#ifndef FILEMANAGER_H /* Prevent multiple inclusions. */
#define FILEMANAGER_H

/* #defines for the fake thingnames and tid's used for file transfer idle timeout checks */
#define TW_FAKE_TID "XXXXXX"
#define TW_FAKE_TID_LEN 6
#define TW_FAKE_THINGNAME "XXXXX"
#define TW_FAKE_THINGNAME_LEN 5

#include "twOSPort.h"
#include "twDefinitions.h"
#include "twDefaultSettings.h"
#include "twList.h"
#include "twInfoTable.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief File Manager singleton structure definition
 */
typedef struct twFileManager {
	TW_MUTEX mtx;
	twList * virtualDirs;
	twList * openFiles;
	twList * callbacks;
	TW_MUTEX fileTransferMtx;
} twFileManager;


/**
 * \brief File Transfer Information structure definition.
*/
typedef struct twFileTransferInfo {
	char * sourceRepository;  /**< The source entityName to use when sending the file and looking up the virtual path. **/
	char * sourcePath;        /**< The <b>virtual</b> path to the file to be sent. **/
	char * sourceFile;        /**< The name of the file to be sent **/
	char * sourceChecksum;    /**< The source file's checksum. **/
	char * targetRepository;  /**< The target repository of the file. **/
	char * targetPath;        /**< The path of the resulting file in the target repository. **/
	char * targetFile;        /**< The name of the resulting file in the target directory. **/
	char * targetChecksum;    /**< The target file's checksum. **/
	DATETIME startTime;       /**< Start time of the file transfer. **/
	DATETIME endTime;         /**< End time of the file transfer. **/
	int32_t duration;         /**< The duration of the file transfer. **/
	char * state;             /**< The current state of the file transfer. **/
	char isComplete;          /**< File transfer completion flag. **/
	double size;              /**< The size of the file. **/
	char * transferId;        /**< Unique file transfer ID. **/
	char * user;              /**< A username associated with the file transfer. **/
	char * message;           /**< A message associated with the file transfer. **/
} twFileTransferInfo;

/**
 * \brief Creates a new ::twFileTransferInfo structure.
 *
 * \param[in]     it    A ::twInfoTable containing all file transfer
 *                      information for the new ::twFileTransferInfo structure.
 *
 * \return A pointer to the newly allocated ::twFileTransferInfo structure.
 *
 * \note The calling function retains ownership of the \p it pointer.
 * \note The calling function gains ownership of the returned
 * ::twFileTransferInfo structure and is responsible for freeing it via
 * twFileTransferInfo_Delete().
*/
twFileTransferInfo * twFileTransferInfo_Create(twInfoTable * it);

/**
 * \brief Frees all memory associated with a ::twFileTransferInfo structure and
 * all its owned substructures.
 *
 * \param[in]     transferInfo  A pointer to the ::twFileTransferInfo structure
 *                              to delete.
 *
 * \return Nothing.
*/
void twFileTransferInfo_Delete(void * transferInfo);

/**
 * \brief Signature of a callback function that is registered to be called when
 * a file transfer completes or fails.
 *
 * \param[in]     fileRcvd      #TRUE if the file was received, #FALSE if it
 *                              was sent.
 * \param[in]     info          A pointer to the ::twFileTransferInfo
 *                              associated with the callback.
 * \param[in]     userdata      An opaque pointer that is passed in when the
 *                              callback is registered.
 *
 * \return Nothing.
*/
typedef void (*file_cb) (char fileRcvd, twFileTransferInfo * info, void * userdata);


/********************************/
/*      twFile Functions        */
/********************************/

/**
 * \brief File/directory data structure.
*/
typedef struct twFile {
	char * name;                    /**< The name of the file. **/
	char * realPath;                /**< The real path of the file. **/
	char * virtualPath;             /**< The virtual path of the file. **/
	char * repository;              /**< The repository of the file. **/
	DATETIME  lastModified;         /**< The date/time of the last modification to the file. **/
	char readOnly;                  /**< Read-only flag (TRUE if read-only, FALSE otherwise). **/
	uint64_t size;                  /**< The size of the file. **/
	TW_FILE_HANDLE handle;          /**< A handle to the file. **/
	char isDir;                     /**< Is directory flag (TRUE if the file is a directory, FALSE otherwise. **/
	uint64_t lastFileXferActivity;  /**< The time (in milliseconds) of the last file transfer activity. **/
	char * tid;                     /**< The unique TID associated with the file. **/
	char openForRead;               /**< #TRUE if the file is open for reading, #FALSE otherwise. **/
	char isInUse;					/**< #TRUE if the file is open for use, #FALSE otherwise. **/
	TW_MUTEX inUseMutex;				/**< Lock for if file is open for use **/
} twFile;

/**
 * \brief Creates the ::twFileManager singleton.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The ::twFileManager singleton must be freed via
 * twFileManager_Delete().
*/ 
int twFileManager_Create();

/**
 * \brief Deletes the ::twFileManager singleton and all its owned
 * substructures.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/ 
int twFileManager_Delete();

/**
* \brief returns whether the ::twFileManager singleton is enabled based off of its pointer value
*
* \return TRUE is enabled, FALSE if not enabled
*/
char twFileManager_IsEnabled();

/**
 * \brief Adds a virtual directory to the ::twFileManager singleton.
 *
 * \param[in]     thingName     The name of the ::twThing that this virtual
 *                              directory is associated with.
 * \param[in]     dirName       A name to apply to this virtual directory.
 * \param[in]     path          The absolute path to the underlying directory
 *                              in the filesystem.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note A named virtual directory is associated with a specific ::twThing and
 * has a specific path in the underlying filesystem.  There can be more than one
 * virtual directory assigned to a single ::twThing.
*/
int twFileManager_AddVirtualDir(const char * thingName, char * dirName, char * path);

/**
 * \brief Removes a virtual directory from the ::twFileManager singleton.
 *
 * \param[in]     thingName     The name of the ::twThing that this virtual
 *                              directory is associated with.
 * \param[in]     dirName       The name of the virtual directory to remove
 *                              ("*" removes all associated with \p thingName).
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twFileManager_RemoveVirtualDir(const char * thingName, char * dirName);

/**
 * \brief Returns a list of all virtual directories from the ::twFileManager
 * singleton.
 *
 * \param[in]     entityName    A filter on virtual directories registered with
 *                              a given entity name (NULL returns all virtual
 *                              directories).
 *
 * \return A pointer to a ::twList of ::twFile pointers representing the
 * virtual directories.  Returns 0 if an error was encountered.
 *
 * \note The calling function will gain ownership of the returned ::twList and
 * is responsible for deleting it via twList_Delete().
*/
twList * twFileManager_ListVirtualDirs(const char * entityName);

/**
 * \brief Creates a new ::twFile structure and populates it with information
 * obtained from a file/directory.
 *
 * \param[in]     thingName     The name of the ::twThing this file is
 *                              associated with.
 * \param[in]     path          Virtual path to the file or directory.
 * \param[in]     filename      The name of the file or directory.
 * \param[in]     mode          The mode to open the file in (see Cfopen()
 *                              documentation).
 *
 * \return A pointer to the newly allocated ::twFile structure.  Returns NULL if
 * an error was encountered.
 *
 * \note The function will check to make sure the path is valid (either in the
 * root or a virtual directory).
 * \note The function will open the file to provide an OS specific reference or
 * handle for any I/O.
 * \note The calling function gains ownership of the returned ::twFile
 * structure and is responsible for deleting it via twFileManager_CloseFile().
*/
twFile * twFileManager_OpenFile(const char * thingName,  const char * path, const char * filename, char * mode);

/**
 * \brief Closes a file and frees all memory associated with the ::twFile
 * structure.
 *
 * \param[in]     f             A pointer to the ::twFile to close.
 *
 * \return Nothing.
*/
void twFileManager_CloseFile(void * file);


/**
 * \brief Get the MD5 checksum of a file.
 *
 * \param[in]     entityName     The entity name associated with the target file.  
  * \param[in]     content          Data to create the result infotable around.
  * \param[in]     realPath          The native filesystem path to this file.
  * \param[in]     adjustedPath          Cleaned up path to the file (see adjustPath()) to match the mask.
 *
 * \return 	#TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
enum msgCodeEnum twFileManager_GetFileChecksum(const char * entityName, twInfoTable ** content, const char * realPath, const char * adjustedPath);

/**
 * \brief Checks to see if the specified file is already open.
 *
 * \param[in]     thingName     The name of the ::twThing this file is
 *                              associated with.
 * \param[in]     path          The full path to the file or directory.  May be
 *                              passed as NULL if tid is specified.
 * \param[in]     filename      The name of the file or directory.
 * \param[in]     tid           The tid of an open transaction.  May be passed
 *                              as NULL if path is specified.
 * \param[out]   isTimedOut		pointer to boolean which will signal if the specified file has timed out, 
 *								NULL can be passed in if the calling funciton does not care about this value
 *
 * \return A pointer to the ::twFile associated with the file or directory if
 * it is already open.  NULL if the file is not open or an error was
 * encountered.
*/
twFile * twFileManager_GetOpenFile(const char * thingName, const char * path, const char * filename, const char * tid, char * isTimedOut);

/**
 * \brief Gets the native file system path for a file.
 *
 * \param[in]     thingName     The name of the ::twThing this file is
 *                              associated with.
 * \param[in]     path          The virtual path of the file.
 * \param[in]     filename      The name of the file.  Passing NULL will assume
 *                              file name is already appended to path.
 *
 * \return The native filesystem path to this file.
 *
 * \note The calling function gains ownership of the returned pointer and is
 * responsible for freeing it.
*/
char * twFileManager_GetRealPath(const char * thingName, const char * path, const char * filename);

/**
 * \brief Forces the file to complete a callback.
 *
 * \param[in]     rcvd          TRUE if the file was received, FALSE if it was
 *                              sent.
 * \param[in]     fti           A pointer to the ::twFileTransferInfo
 *                              structure.
 *
 * \return Nothing.
*/
void twFileManager_MakeFileCallback(char rcvd, twFileTransferInfo * fti);

/**
 * \brief Registers a function to be called when a file transfer completes.
 *
 * \param[in]     cb            The function to be registered.
 * \param[in]     filter        A wildcard-supporting filter to apply to the
 *                              filename of transfers to be associated with the
 *                              callback.
 * \param[in]     onceOnly      If #TRUE, this registration will be deleted
 *                              after it is called once.
 * \param[in]     userdata      An opaque pointer that is passed in when the
 *                              callback is registered.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twFileManager_RegisterFileCallback(file_cb cb, char * filter, char onceOnly, void * userdata);

/**
 * \brief Unregisters a file callback function registered via
 * twFileManager_RegisterFileCallback().
 *
 * \param[in]     cb            The function to be unregistered.
 * \param[in]     filter        A wildcard-supporting filter to apply to the
 *                              filename of transfers to be unregistered.
 * \param[in]     userdata      An opaque pointer that is passed in when the
 *                              callback is registered.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twFileManager_UnregisterFileCallback(file_cb cb, char * filter, void * userdata);


/**
 * \param Sends a file to the server.
 *
 * \param[in]      sourceRepo   The entity name to use when sending the file
 *                              (and looking up the virtual path).
 * \param[in]      sourcePath   The <b>virtual</b> path of the file to send
 *                              (not including the file name).
 * \param[in]      sourceFile   The name of the file to send.
 * \param[in]      targetRepo   The target repository of the file.
 * \param[in]      targetPath   The path of the resulting file in the target
 *                              repository (not including the file name).
 * \param[in]      targetFile   The name of the resulting file in the target
 *                              directory.
 * \param[in]      timeout      Time (in milliseconds) to wait for a
 *                              response from the ThingWorx server.  0 uses
 *                              #DEFAULT_MESSAGE_TIMEOUT.
 * \param[in]      asynch       If #TRUE return immediately and call the
 *                              callback function when the transfer completes.
 *                              If #FALSE block until the transfer is complete.
 * \param[out]     tid          A pointer to the transfer ID used for this
 *                              transfer.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The callback will be called for synchronous or asynchronous
 * transactions.
 * \note The calling function is responsible for registering/unregistering
 * callback functions (see twFileManager_RegisterFilecallback() and
 * twFileManager_UnregisterFileCallback()).
 * \note In a single-threaded environment, this function must be invoked asynchronously.
*/
int twFileManager_SendFile(const char * sourceRepo, const char * sourcePath, const char * sourceFile,
						   const char * targetRepo, const char * targetPath, const char * targetFile,
						   uint32_t timeout, char asynch, char ** tid);

/**
 * \brief Gets a file from the server.
 *
 * \param[in]      sourceRepo   The entity name to get the file from.
 * \param[in]      sourcePath   The <b>virtual</b> path of the file to get
 *                              (not including the file name).
 * \param[in]      sourceFile   The name of the file to get.
 * \param[in]      targetRepo   The target repository of the file.
 * \param[in]      targetPath   The path of the resulting file in the target
 *                              repository (not including the file name).
 * \param[in]      targetFile   The name of the resulting file in the target
 *                              directory.
 * \param[in]      timeout      Time (in milliseconds) to wait for a
 *                              response from the ThingWorx server.  0 uses
 *                              #DEFAULT_MESSAGE_TIMEOUT.
 * \param[in]      asynch       If #TRUE return immediately and call the
 *                              callback function when the transfer completes.
 *                              If #FALSE block until the transfer is complete.
 * \param[out]     tid          A pointer to the transfer ID used for this
 *                              transfer.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The callback will be called for synchronous or asynchronous
 * transactions.
 * \note The calling function is responsible for registering/unregistering
 * callback functions (see twFileManager_RegisterFilecallback() and
 * twFileManager_UnregisterFileCallback()).
*/
int twFileManager_GetFile(const char * sourceRepo, const char * sourcePath, const char * sourceFile,
						   const char * targetRepo, const char * targetPath, const char * targetFile,
						   uint32_t timeout, char asynch, char ** tid);

/**
 * \brief Checks for any stalled file transfers and deletes them.
 *
 * \return Nothing.
*/
void twFileManager_CheckStalledTransfers();

/**
 * \brief Lists the files or subdirectories in a directory \p path.
 *
 * \param[in]     entityName    The entity name to use when looking up a virtual
 *                              directory path.
 * \param[in]     path          The <b>virtual</b> path to the directory to
 *                              investigate.
 * \param[in]     namemask      A wildcard enabled mask to list only files that
 *                              match the mask.
 * \param[in]     returnType    #LIST_ALL returns all files and directories,
 *                              #LIST_FILES returns files only, #LIST_DIRS
 *                              returns directories only.
 *
 * \return A ::twList of ::twFile pointers representing all requested items
 * within the given \p path.  Returns  if an error was encountered.
 *
 * \note The calling function gains ownership of the returned ::twList and is
 * responsible for freeing it via twList_Delete().
*/
twList * twFileManager_ListEntities(const char * entityName, const char * path, const char * namemask, char returnType);


/**
* \brief Completes the specified file transfer based on the content of job,
*		invoking any file callbacks and closing the open file.
*
* \param[in]     entityName    The entity name to use when looking up a virtual
*                              directory path.
* \param[in]     ft			   Populated twFileTransferInfo structure
* \param[in]     path          Path to the file
* \param[in]     adjustedPath  Cleaned up path to the file (see adjustPath())
*                              match the mask.
*
* \return TWX_SUCCESS if files are cleaned up, TWX_INTERNAL_SERVER_ERROR if
*			no matching open files can be found.
*/
enum msgCodeEnum twFileManager_FinishFileTransfer(
	const char * entityName,
	twFileTransferInfo * ft,
	const char * path,
	const char * adjustedPath
);


/******************************************************************************/
/*     Private methods which should only be called by Thingworx functions     */
/******************************************************************************/
/**
 * \brief						uses the file->inUseMutex to safely set file->isInUse 
 *								to the value of the isInUse parameter
 *
 * \param[in]     file		    The twFile to be modified
 * \param[in]     path          boolean value to use for file->isInUse 
 *
 *
*/
void twFile_SetIsInUse(twFile * file, const char isInUse);

/**
 * \brief						allocates memory for the twFile and initializes the twFile->inUseMutex
 *
 * \return a valid pointer to a twFile struct or NULL if there was a failure
*/
twFile * twFile_Create();

/**
 * \brief						Deletes mutex's, file handles, and Free's memory realted to the twFile struct
 *
 * \param[in]     file		    The twFile to be deleted
*/
void twFile_Delete(void * f);

/* creating a function in order to support stubbing of fopen macro*/
TW_FILE_HANDLE twFile_FOpen(const char * name, const char * mode);

#ifdef __cplusplus
}
#endif

#endif
