/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx File Transfer
 */

#include "twFileManager.h"
#include "twFileTransferCallbacks.h"
#include "twLogger.h"
#include "twInfoTable.h"
#include "twDefinitions.h"
#include "twApi.h"
#include "twApiStubs.h"
#include "stringUtils.h"
#include "wildcard.h"

#define MAX_PATH_LEN 4096

twFileManager * fm = NULL;

void * fileXferCallback = NULL;

/***************************************/
/*     File Transfer Info Functions    */
/***************************************/
twFileTransferInfo * twFileTransferInfo_Create(twInfoTable * it) {
	twFileTransferInfo * ft = NULL;
	if (!it) {
		TW_LOG(TW_ERROR, "twFileTransferInfo_Create: NULL input parameter");
		return NULL;
	}
	ft = (twFileTransferInfo *)TW_CALLOC(sizeof(twFileTransferInfo), 1);
	if (!ft) {
		TW_LOG(TW_ERROR, "twFileTransferInfo_Create: Error allocating storage");
		return NULL;
	}
	twInfoTable_GetString(it, "sourceRepository",0,&ft->sourceRepository);
	twInfoTable_GetString(it, "sourcePath",0,&ft->sourcePath);
	twInfoTable_GetString(it, "sourceFile",0,&ft->sourceFile);
	twInfoTable_GetString(it, "sourceChecksum",0,&ft->sourceChecksum);
	twInfoTable_GetString(it, "targetRepository",0,&ft->targetRepository);
	twInfoTable_GetString(it, "targetPath",0,&ft->targetPath);
	twInfoTable_GetString(it, "targetFile",0,&ft->targetFile);
	twInfoTable_GetString(it, "targetChecksum",0,&ft->targetChecksum);
	twInfoTable_GetDatetime(it, "startTime",0,&ft->startTime);
	twInfoTable_GetDatetime(it, "endTime",0,&ft->endTime);
	twInfoTable_GetInteger(it, "duration",0,&ft->duration);
	twInfoTable_GetString(it, "state",0,&ft->state);
	twInfoTable_GetBoolean(it, "isComplete",0,&ft->isComplete);
	twInfoTable_GetNumber(it, "size",0, &ft->size);
	twInfoTable_GetString(it, "transferId",0,&ft->transferId);
	twInfoTable_GetString(it, "user",0,&ft->user);
	twInfoTable_GetString(it, "message",0,&ft->message);
	return ft;
}

void twFileTransferInfo_Delete(void * transferInfo) {
	twFileTransferInfo * ft = (twFileTransferInfo *)transferInfo;
	if (!transferInfo) return;
	if (ft->sourceRepository) TW_FREE(ft->sourceRepository);
	if (ft->sourcePath) TW_FREE(ft->sourcePath);
	if (ft->sourceFile) TW_FREE(ft->sourceFile);
	if (ft->sourceChecksum) TW_FREE(ft->sourceChecksum);
	if (ft->targetRepository) TW_FREE(ft->targetRepository);
	if (ft->targetPath) TW_FREE(ft->targetPath);
	if (ft->targetFile) TW_FREE(ft->targetFile);
	if (ft->targetChecksum) TW_FREE(ft->targetChecksum);
	if (ft->state) TW_FREE(ft->state);
	if (ft->transferId) TW_FREE(ft->transferId);
	if (ft->user) TW_FREE(ft->user);
	if (ft->message) TW_FREE(ft->message);
	TW_FREE(ft);
}

/********************************/
/*      twFile Functions        */
/********************************/

void twFile_Delete(void * f) {
	twFile * tmp = NULL;
	if (f) {
	tmp = (twFile *) f;
		if (tmp->handle) {
			TW_FCLOSE(tmp->handle);
			tmp->handle = NULL;
		}
		if (tmp->name) {
			TW_FREE(tmp->name);
			tmp->name = NULL;
		}
		if (tmp->realPath) {
			TW_FREE(tmp->realPath);
			tmp->realPath = NULL;
		}
		if (tmp->virtualPath) {
			TW_FREE(tmp->virtualPath);
			tmp->virtualPath = NULL;
		}
		if (tmp->repository) {
			TW_FREE(tmp->repository);
			tmp->repository = NULL;
		}
		if (tmp->tid) {
			TW_FREE(tmp->tid);
			tmp->tid = NULL;
		}
		if (tmp->inUseMutex) {
			twMutex_Delete(tmp->inUseMutex);
			tmp->inUseMutex = NULL;
		}
		TW_FREE(f);
	}
}

twFile * twFile_Create(){
	twFile * f = (twFile *)TW_CALLOC(sizeof(twFile), 1);
	if (f) {
		f->inUseMutex = twMutex_Create();
		if (!f->inUseMutex) {
			twFile_Delete(f);
			f = NULL;
		}
	}
	return f;
}

void twFile_SetIsInUse(twFile * file, const char isInUse){
	if (!file) return;
	twMutex_Lock(file->inUseMutex);
	file->isInUse = isInUse;
	twMutex_Unlock(file->inUseMutex);
}

/********************************/
/*     Virtual Directories      */
/********************************/
typedef struct twVirtualDir {
	char * thingName;
	char * dirName;
	char * path;
} twVirtualDir;

void twVirtualDir_Delete(void * vdir) {
	twVirtualDir * tmp = (twVirtualDir *)vdir;
	if (!vdir) return;
	TW_FREE(tmp->thingName);
	TW_FREE(tmp->dirName);
	TW_FREE(tmp->path);
	TW_FREE(tmp);
}

char * fixPath(const char * path);

twVirtualDir * twVirtualDir_Create(const char * thingName, char * dirName, char * path) {
	twVirtualDir * vdir = NULL;
	char * fixedPath = fixPath(path);
	if (!thingName || !dirName || !path) {
		TW_LOG(TW_ERROR, "twVirtualDir_Create: NULL input parameter");
		if (fixedPath) TW_FREE(fixedPath);
		return NULL;
	}
	vdir = (twVirtualDir *)TW_CALLOC(sizeof(twVirtualDir), 1); 
	if (!vdir) {
		TW_LOG(TW_ERROR,"twVirtualDir_Create: Error allocating virtualDir");
		TW_FREE(fixedPath);
		return NULL;
	}
	vdir->thingName = duplicateString(thingName);
	vdir->dirName = duplicateString(dirName);
	/* Make sure our path is absolute */
	if (fixedPath[0] != TW_FILE_DELIM) {
		/* Do we have a disk specifier? */
		size_t fixedPathLen = strnlen(fixedPath, MAX_PATH_LEN);
		if (fixedPathLen > 1 && fixedPath[1] != ':') {
			vdir->path = (char *)TW_CALLOC(fixedPathLen + 2, 1);
			if (vdir->path) {
				vdir->path[0] = TW_FILE_DELIM;
				strcpy(&(vdir->path[1]), path);
			}
			TW_FREE(fixedPath);
		} else vdir->path = fixedPath;
	} else vdir->path = fixedPath;
	if (!vdir->thingName || !vdir->dirName || ! vdir->path) {
		TW_LOG(TW_ERROR,"twVirtualDir_Create: Error allocating virtualDir entries");
		twVirtualDir_Delete(vdir); 
		return NULL;
	}
	return vdir;
}

/********************************/
/*      Helper Functions        */
/********************************/
char * fixPath(const char * path) {
	size_t i = 0;
	size_t len = 0;
	char * tmp = duplicateString(path);
	if (!tmp) return NULL;
	len = strnlen(tmp, MAX_PATH_LEN);
	for (i = 0; i < len; i++) {
		if (tmp[i] == '\\' || tmp[i] == '/') tmp[i] = TW_FILE_DELIM;
	}
	return tmp;
}

ListEntry * getVirtualDirEntry(const char * thingName, char * vdir) {
	ListEntry * le = NULL;
	twVirtualDir * tmp = NULL;
	if (!fm || !fm->virtualDirs || !fm->mtx) {
		TW_LOG(TW_ERROR,"getVirtualDir: FileTransferManager not initialized");
		return NULL;
	}
	le = twList_Next(fm->virtualDirs, NULL);
	while (le && le->value) {
		tmp = (twVirtualDir *)le->value;
		if ((!strcmp(tmp->dirName, vdir) && !strcmp(tmp->thingName, thingName)) ||
			(!strcmp(TW_VIRTUAL_STAGING_DIR, vdir) && !strcmp(tmp->dirName, vdir)) ||
			(!strcmp(tmp->dirName, vdir) && !strcmp("*", thingName)) ||
			(!strcmp(tmp->dirName, "*") && !strcmp(tmp->thingName, thingName))) break;
		le = twList_Next(fm->virtualDirs, le);
	}
	return le;
}

char * getVirtualDirPath(const char * thingName, char * vdir) {
	ListEntry * le = NULL;
	if (!fm || !fm->mtx) {
		TW_LOG(TW_ERROR,"getVirtualDirPath: FileTransferManager not initialized");
		return NULL;
	}	
	twMutex_Lock(fm->mtx);	
	le = getVirtualDirEntry(thingName, vdir);
	twMutex_Unlock(fm->mtx);	
	if (!le || !le->value) return NULL;
	return ((twVirtualDir *)(le->value))->path;
}

/********************************/
/*    File Transfer Callbacks   */
/********************************/
typedef struct twFileXferCallback {
	file_cb cb;
	char * filter;
	char onceOnly;
	void * userdata;
	char unregistered;
	int refCount;
	TW_MUTEX mtx;
} twFileXferCallback;

twFileXferCallback * twFileXferCallback_Create(file_cb cb, const char * filter, char onceOnly, void * userdata) {
	twFileXferCallback * res = NULL;
	if (!cb) return NULL;
	res = (twFileXferCallback *) TW_CALLOC(sizeof(twFileXferCallback), 1);
	if (!res) return NULL;
	res->cb = cb;
	res->onceOnly = onceOnly;
	res->userdata = userdata;
	if (filter) res->filter = duplicateString(filter);
	res->unregistered = FALSE;
	res->refCount = 1;
	res->mtx = twMutex_Create();
	return res;
}

void twFileXferCallback_Delete(void * cb) {
	twFileXferCallback * tmp = (twFileXferCallback *) cb;
	if (!cb) return;
	if (tmp->filter) TW_FREE(tmp->filter);
	if (tmp->mtx) twMutex_Delete(tmp->mtx);
	TW_FREE(tmp);
}

/********************************/
/*        API Functions         */
/********************************/
/*extern enum msgCodeEnum fileTranferCallback(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content);*/

twFileManager *twFileManager_GetFileManager() {
	return fm;
}

int twFileManager_Create() {
	fm = (twFileManager *) TW_CALLOC(sizeof(twFileManager), 1);
	if (!fm) {
		TW_LOG(TW_ERROR,"twFileManager_Create: Error allocating FileManager singleton");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	fm->mtx = twMutex_Create();
    fm->fileTransferMtx = twMutex_Create();
	fm->virtualDirs = twList_Create(twVirtualDir_Delete);
	fm->openFiles = twList_Create(twFile_Delete);
	fm->callbacks = twList_Create(twFileXferCallback_Delete);
	if (!fm->mtx || !fm->virtualDirs || !fm->openFiles || !fm->callbacks) {
		TW_LOG(TW_ERROR,"twFileManager_Create: Error allocating mutex or tracking list");
		twFileManager_Delete();
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	fileXferCallback = &fileTransferCallback;
	/* Create our staging directory if it doesn't exist */
	{
	uint64_t size = 0;
	DATETIME lastModified = 0;
	char isDirectory = FALSE;
	char isReadOnly = FALSE;
	if (twDirectory_GetFileInfo(twcfg.file_xfer_staging_dir, &size, &lastModified, &isDirectory, &isReadOnly) || !isDirectory) {
		int res = 0;
		if (!isDirectory) {
			if(lastModified) TW_LOG(TW_WARN, "Overwriting file at path = %s", twcfg.file_xfer_staging_dir ? twcfg.file_xfer_staging_dir : "NULL");
			twDirectory_DeleteFile(twcfg.file_xfer_staging_dir);
		}
		TW_LOG(TW_WARN, "Creating Staging Directory at path = %s", twcfg.file_xfer_staging_dir ? twcfg.file_xfer_staging_dir : "NULL");
		res = twDirectory_CreateDirectory(twcfg.file_xfer_staging_dir);
		if (res) {
			TW_LOG(TW_ERROR,"twFileManager_Create: Error creating staging directory %s.  Error Code: %d", 
				twcfg.file_xfer_staging_dir ? twcfg.file_xfer_staging_dir : "NULL",res);
			twFileManager_Delete();
			return TW_ERROR_CREATING_STAGING_DIR;
		}
	}
	/* Setup our staging directory as a virtual dir */
	twFileManager_AddVirtualDir("*",TW_VIRTUAL_STAGING_DIR, twcfg.file_xfer_staging_dir);
	}
	return TW_OK;
}

int twFileManager_Delete() {
	if (!fm) return TW_INVALID_PARAM;
	if (fm->mtx) twMutex_Lock(fm->mtx);
    if (fm->fileTransferMtx) twMutex_Lock(fm->fileTransferMtx);
	if (fm->virtualDirs) twList_Delete(fm->virtualDirs);
	if (fm->openFiles) twList_Delete(fm->openFiles);
	if (fm->callbacks) twList_Delete(fm->callbacks);
	if (fm->mtx) twMutex_Unlock(fm->mtx);
    if (fm->fileTransferMtx) twMutex_Unlock(fm->fileTransferMtx);
    twMutex_Delete(fm->fileTransferMtx);
	twMutex_Delete(fm->mtx);
	TW_FREE(fm);
	fm = NULL;
	return TW_OK;
}

char twFileManager_IsEnabled() {
	return fm ? TRUE : FALSE;
}

char * twFileManager_GetRealPath(const char * thingName, const char * path, const char * filename) {

	char * realPath = NULL;
	char * vdirName = NULL;
	char * vdirPath = NULL;
	char * adjustedPath = fixPath(path);
	char * tmp = NULL;
	size_t len = 0;
	size_t realPathLen = 0;
	size_t vdirPathLen = 0;
	size_t thingNameLen = 0;
	size_t tmpLen = 0;
	size_t filenameLen = 0;

	if (!thingName || !adjustedPath || !strnlen(adjustedPath, MAX_PATH_LEN)) {
		TW_LOG(TW_ERROR,"twFileManager_GetRealPath: Invalid parameters");
		if (adjustedPath) TW_FREE(adjustedPath);
		return 0;
	}
	len = strnlen(adjustedPath, MAX_PATH_LEN);
	/* Remove any trailing delimiter */
	if (adjustedPath[len - 1] == TW_FILE_DELIM) adjustedPath[len - 1] = 0;
	/* Skip any leading delimiter. */
	vdirName = adjustedPath;
	if( adjustedPath[0] == TW_FILE_DELIM) vdirName = &adjustedPath[1]; 
	/* Extract just the name of the vdir */
	tmp = strchr(vdirName, TW_FILE_DELIM);
	if (tmp) {
		/* tmp now contains the rest of the path after the vdir */
		*tmp = 0;
		tmp++;
	}
	tmpLen = (tmp ? strnlen(tmp, MAX_PATH_LEN) : 0);
	thingNameLen = strnlen(thingName, MAX_PATH_LEN);
	
	/* Get the real vdir path */
	vdirPath = getVirtualDirPath(thingName, vdirName);
	if (!vdirPath) {
		/* Log error if vdirName is not empty string */
		if (strnlen(vdirName, MAX_PATH_LEN) > 0) {
			TW_LOG(TW_ERROR,"twFileManager_GetRealPath: Error getting vdirPath, %s", vdirName);
		}
		TW_FREE(adjustedPath);
		return NULL;
	}
	vdirPathLen = strnlen(vdirPath, MAX_PATH_LEN);
	filenameLen = (filename ? strnlen(filename, MAX_PATH_LEN) : 0);
	
	realPathLen = vdirPathLen+ 1 + thingNameLen + 1 + tmpLen + 1 + filenameLen + 1;
	
	if(realPathLen > MAX_PATH_LEN) {
		TW_LOG(TW_ERROR,"twFileManager_GetRealPath: realPath exceeded maximum length");
		TW_FREE(adjustedPath);
		return NULL;
	}
	
	/* Allocate enough memory for our return string */
	realPath = (char *)TW_CALLOC(realPathLen, 1);

	if (!realPath) {
		TW_LOG(TW_ERROR,"twFileManager_GetRealPath: Error allocating memory");
		TW_FREE(adjustedPath);
		return NULL;
	}
	
	/* Copy the vdir path to our return value */
	strcpy(realPath, vdirPath);
	
	realPathLen = strnlen(realPath, MAX_PATH_LEN);
	
	/* Add a delimiter if we need to */
	if ((tmp || filename) && realPath[realPathLen - 1] != TW_FILE_DELIM) {
		realPath[realPathLen] = TW_FILE_DELIM;
	}

	if (tmp) strncat(realPath, tmp, tmpLen);
	/* Add a delimiter and filename if needed */
	if (filename) {
		size_t delimLen = strnlen(TW_FILE_DELIM_STR, MAX_PATH_LEN);
		if (realPath[realPathLen - 1] != TW_FILE_DELIM_STR[0]) strncat(realPath, TW_FILE_DELIM_STR, delimLen);
		strncat(realPath, filename, filenameLen);
	}
	TW_LOG(TW_DEBUG,"getRealPath: Real path for %s:%s%c%s = %s", thingName, path, TW_FILE_DELIM, filename ? filename : "", realPath);
	TW_FREE(adjustedPath);
	return realPath;
}

int twFileManager_AddVirtualDir(const char * thingName, char * dirName, char * path) {
	twVirtualDir * vdir = NULL;
	if (!fm || !fm->virtualDirs || !fm->mtx) {
		TW_LOG(TW_ERROR,"twFileManager_AddVirtualDir: FileTransferManager not initialized");
		return TW_FILE_XFER_MANAGER_NOT_INITIALIZED;
	}
	/* Delete any existing entry for this vdir	*/
	twFileManager_RemoveVirtualDir(thingName, dirName);
	/* Now create the new entry */
	vdir = twVirtualDir_Create(thingName, dirName, path);
	if (!vdir) {
		TW_LOG(TW_ERROR,"twFileManager_AddVirtualDir: Error allocating vdir");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	twMutex_Lock(fm->mtx);
	TW_LOG(TW_TRACE, "twFileManager_AddVirtualDir: Adding %s to vdir list", dirName);
	twList_Add(fm->virtualDirs, vdir);
	twMutex_Unlock(fm->mtx);
	return TW_OK;
}

int twFileManager_RemoveVirtualDir(const char * thingName, char * dirName) {
	ListEntry * le = NULL;
	if (!fm || !fm->virtualDirs || !fm->mtx) {
		TW_LOG(TW_ERROR,"twFileManager_RemoveVirtualDir: FileTransferManager not initialized");
		return TW_FILE_XFER_MANAGER_NOT_INITIALIZED;
	}
	twMutex_Lock(fm->mtx);
	if (!strcmp(dirName, "*")) {
		/* Remove all entries for this thing */
		le = getVirtualDirEntry(thingName, dirName);
		while (le) {
			if (le) twList_Remove(fm->virtualDirs, le, TRUE);
			le = getVirtualDirEntry(thingName, dirName);
		}
	} else {
		le = getVirtualDirEntry(thingName, dirName);
		if (le) twList_Remove(fm->virtualDirs, le, TRUE);
	}
	twMutex_Unlock(fm->mtx);
	return TW_OK;
}

twList * twFileManager_ListVirtualDirs(const char * entityName) {
	ListEntry * le = NULL;
	twList * tmp = NULL;
	if (!fm || !fm->virtualDirs || !fm->mtx) {
		TW_LOG(TW_ERROR,"twFileManager_ListVirtualDirs: FileTransferManager not initialized");
		return NULL;
	}
	tmp = twList_Create(twFile_Delete);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twFileManager_ListVirtualDirs: Error allocating twList result");
		return NULL;
	}
	twMutex_Lock(fm->mtx);
	le = twList_Next(fm->virtualDirs, NULL);
	while (le && le->value) {
		int result = 0;
		twVirtualDir * d = (twVirtualDir *)(le->value);
		if ( !entityName || !strcmp(entityName, d->thingName) ) {
			twFile * f = twFile_Create();
			if (!f) {
				TW_LOG(TW_ERROR,"twFileManager_ListVirtualDirs: Error allocating twFile structure");
				twList_Delete(tmp);
				twMutex_Unlock(fm->mtx);
				return NULL;
			}
			f->name = duplicateString(d->dirName);
			f->realPath = duplicateString(d->path);
			f->virtualPath = duplicateString(d->dirName);
			result = twDirectory_GetFileInfo(d->path, &(f->size), &(f->lastModified), &(f->isDir), &(f->readOnly));
			if (result) {
				TW_LOG(TW_WARN,"twFileManager_ListVirtualDirs: Error getting file info for %s", d->path);
				twFile_Delete(f);
			} else twList_Add(tmp, f);
		}
		le = twList_Next(fm->virtualDirs, le);
	}
	twMutex_Unlock(fm->mtx);
	return tmp;
}

twFile * twFileManager_GetOpenFile(const char * thingName, const char * path, const char * filename, const char * tid, char * isTimedOut) {
	twFile * f = NULL;
	ListEntry * le = NULL;
	char * realpath = 0;
	char fileFound = FALSE;
	twFile * tmp = NULL;
	uint64_t now = twGetSystemMillisecondCount();

	/* if isTimedOut is valid, make sure the value is false*/
	if (isTimedOut) {
		*isTimedOut = FALSE;
	}
	if (!fm || !fm->mtx || !fm->openFiles) {
		TW_LOG(TW_ERROR,"twFileManager_GetOpenFile: FileTransferManager not initialized");
		return NULL;
	}
	if (!thingName) {
		TW_LOG(TW_ERROR,"twFileManager_GetOpenFile: Missing thingName or path");
		return NULL;
	}
	/* 
		Check to see if this file is already open. Comparison should be either if tid's match or
		thingName and realpath match then the file is already open.
	*/
	if (path) realpath = twFileManager_GetRealPath(thingName, path, filename);
	if (!realpath && !tid) {
		TW_LOG(TW_ERROR,"twFileManager_GetOpenFile: Missing tid and Error getting real path for %s:%s", thingName, path);
		return NULL;
	}

	/* if this is an idle check, ignore TRACE messages for non-existant TID */
	if (NULL==tid || strncmp(TW_FAKE_TID, tid, TW_FAKE_TID_LEN) !=0){
		TW_LOG(TW_TRACE, "twFileManager_GetOpenFile: Looking for tid: %s, realpath: %s.", tid ? tid : "NA", realpath ? realpath : "NA");
	}
	/* Get the requested file and also check files for idle timeouts */
	if (fm->mtx) twMutex_Lock(fm->mtx);
	le = twList_Next(fm->openFiles, NULL);
	while (le && le->value) {
		tmp = (twFile *)(le->value);
		if ((tid && tmp->tid && !strcmp(tid,tmp->tid)) || (realpath && tmp->repository && (!strcmp(tmp->repository, thingName) && tmp->realPath && !strcmp(tmp->realPath, realpath)))) {
			fileFound = TRUE;
		}
		TW_LOG(TW_TRACE, "twFileManager_GetOpenFile: Compare to tid: %s, realpath: %s.", tmp->tid, tmp->realPath);

		/* Check to see if we have been idle */
		if (twTimeGreaterThan(now, twAddMilliseconds(tmp->lastFileXferActivity, twcfg.file_xfer_timeout))) {
			ListEntry * entry = le;
			le = twList_Next(fm->openFiles, le);
			if (!tmp->isInUse) {
				twMutex_Lock(tmp->inUseMutex);
				tmp->isInUse = TRUE;

				/* if the file has timed out, check if this file is the one we are looking for and set the outparam to true if necessary */
				if (isTimedOut && fileFound) {
					TW_LOG(TW_TRACE,"twFileManager_GetOpenFile: Specified file transfer %s:%s has timed out. Closing: %s", (tid ? "tid":"realpath"), (tid ? tid:realpath), tmp->realPath);
					*isTimedOut = TRUE;
				} else {
					TW_LOG(TW_TRACE,"twFileManager_GetOpenFile: Cleaning up timed out transfer. Closing: %s", tmp->realPath);
				}

				s_twList_Remove(fm->openFiles, entry, FALSE);
				s_twFile_Delete(tmp);
			}
			continue;
		}
		if (fileFound) {
			/* This file already is open */
			TW_LOG(TW_TRACE,"twFileManager_GetOpenFile: Found Open file %s%c%s", tmp->realPath, TW_FILE_DELIM, filename ? filename : "");
			twFile_SetIsInUse(tmp, TRUE); 
			tmp->lastFileXferActivity = twGetSystemMillisecondCount();
			f = tmp;
			break;
		}
		le = twList_Next(fm->openFiles, le);
	}
	if (fm->mtx) twMutex_Unlock(fm->mtx);
	if (realpath) TW_FREE(realpath);

	if (NULL==tid || strncmp(TW_FAKE_TID, tid, TW_FAKE_TID_LEN) != 0) {
		TW_LOG(TW_TRACE, "twFileManager_GetOpenFile: Returning %s", f ? f->realPath : "NULL");
	}
	return f;
}


enum msgCodeEnum twFileManager_GetFileChecksum(const char * entityName, twInfoTable ** content, const char * realPath, const char * adjustedPath) {
	TW_FILE_HANDLE f = 0;
	twDataShape * ds = NULL;
	twInfoTableRow * row = NULL;
	twFile * file = NULL;

	/* Outputs */
	ds = twDataShape_Create(twDataShapeEntry_Create("result","",TW_STRING));
	if (!ds) {
		TW_LOG(TW_ERROR, "twGetFileChecksum: Error creating output datashape");
		return TWX_INTERNAL_SERVER_ERROR;
	}
    /* Create the output infotable */
	*content = twInfoTable_Create(ds);
	if (!*content) {
		TW_LOG(TW_ERROR, "twGetFileChecksum: Error creating output infotable");
		twDataShape_Delete(ds);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	
	file = twFileManager_GetOpenFile(entityName, adjustedPath, NULL, NULL, NULL);
	
	if (fm->mtx) twMutex_Lock(fm->mtx);
	
	if (file && file->handle) {
		f = file->handle;
	}
	else if (twDirectory_FileExists(realPath)) {
		f = TW_FOPEN(realPath,"rb");
	}
	
	if (f) {
		TW_MD5_CTX md5;
		char hexHash[48];
		char hex[4];
		int i = 0;
		int bytesRead = 0;
		char * buffer = (char *)TW_CALLOC(twcfg.file_xfer_md5_block_size,1);
		char digest[16];
		memset(digest, 0, sizeof (digest));
		memset(&md5, 0, sizeof(TW_MD5_CTX));
		if (!buffer) {
			TW_LOG(TW_ERROR, "twGetFileChecksum: Error allocating buffer");
			twInfoTable_Delete(*content);
			*content = NULL;
			if (!file) {
				TW_FCLOSE(f);
			} else {
				twFile_SetIsInUse(file, 0x00);
			}
			
			if (fm->mtx) twMutex_Unlock(fm->mtx);
			
			return TWX_INTERNAL_SERVER_ERROR;
		}
		/* Start from the beginning of the file */
		TW_FSEEK(f,0,SEEK_SET);
		twMD5_Init(&md5);
		bytesRead = TW_FREAD(buffer, 1, twcfg.file_xfer_md5_block_size, f);
		while (bytesRead > 0) {
			twMD5_Update(&md5, (unsigned char *)buffer, bytesRead);
			bytesRead = TW_FREAD(buffer, 1, twcfg.file_xfer_md5_block_size, f);
		}
		twMD5_Final((unsigned char *)digest, &md5);
		/* Make sure we didn't have an error */
		if (TW_FERROR(f)) {
			TW_LOG(TW_ERROR, "twGetFileChecksum: Error reading from file %s. Bytes Read: %d", realPath, bytesRead);
			TW_FREE(buffer);
			twInfoTable_Delete(*content);
			*content = NULL;
			if (!file) {
				TW_FCLOSE(f);
			} else {
				twFile_SetIsInUse(file, 0x00);
			}
			
			if (fm->mtx) twMutex_Unlock(fm->mtx);
			
			return TWX_INTERNAL_SERVER_ERROR;
		}
		/* Only close if we opened it, else set isInUse to false */
		if (!file) {
			TW_FCLOSE(f);
		} else {
			twFile_SetIsInUse(file, 0x00);
		}

		if (fm->mtx) twMutex_Unlock(fm->mtx);

		/* Convert the hash to hex */
		memset(hexHash, 0, 48);
		memset(hex, 0, 4);
		for (i = 0; i <16; i++) {
			snprintf(hex, 3, "%02x", (unsigned char)digest[i]);
			strcat(hexHash, hex);
		}
		
		row = twInfoTableRow_Create(twPrimitive_CreateFromString(hexHash, TRUE));
		if (!row) {
			TW_LOG(TW_ERROR, "twGetFileChecksum: Error creating infotable row");
			twInfoTable_Delete(*content);
			*content = NULL;
			return TWX_INTERNAL_SERVER_ERROR;
		}
		twInfoTable_AddRow(*content, row);
		TW_FREE(buffer);
		return TWX_SUCCESS;
	}
	
	/* if the file was opened set isInUse to false */
	if (file) {
		twFile_SetIsInUse(file, 0x00);
	}
	
	if (fm->mtx) twMutex_Unlock(fm->mtx);
	
	TW_LOG(TW_WARN, "twGetFileChecksum: twFile for %s not found", realPath);
	twInfoTable_Delete(*content);
	*content = NULL;
	return TWX_NOT_FOUND;
}

twFile * twFileManager_OpenFile(const char * thingName, const char * path, const char * filename, char * mode) {
	twFile * tmp = NULL;
	char * realpath = NULL;
	int res = 0;
	if (!fm || !fm->mtx || !fm->openFiles) {
		TW_LOG(TW_ERROR,"twFileManager_OpenFile: FileTransferManager not initialized");
		return NULL;
	}
	/* Check to see if this file is already open */
	tmp = s_twFileManager_GetOpenFile(thingName, path, filename, NULL, NULL);
	if (tmp) {
		/* This file already is open */
		TW_LOG(TW_WARN,"twFileManager_OpenFile: File %s is already open", filename);
		return NULL;
	}
	realpath = s_twFileManager_GetRealPath(thingName, path, filename);
	if (!realpath) {
		TW_LOG(TW_ERROR,"twFileManager_OpenFile: Error getting real path to file %s", filename ? filename : "NULL");
		return NULL;
	}
	/* Open the file */
	tmp = twFile_Create();
	if (!tmp) {
		TW_LOG(TW_ERROR,"twFileManager_OpenFile: Error allocating twFile structure");
		TW_FREE(realpath);
		return NULL;
	}
	tmp->repository = duplicateString(thingName);
	tmp->virtualPath = duplicateString(path);
	tmp->realPath = realpath; /* tmp now owns the realpath pointer */
	if (filename) {
		tmp->name = duplicateString(filename);
	} else {
		/* Get the filename From the path */
		char * fname = 0;
		fname = strrchr(realpath, TW_FILE_DELIM);
		if (fname) {
			tmp->name = (char *)TW_CALLOC(strnlen(realpath, MAX_PATH_LEN),1);
			if (tmp->name) strcpy(tmp->name,fname + 1);
		}
	}
	res = s_twDirectory_GetFileInfo(tmp->realPath, &tmp->size, &tmp->lastModified, &tmp->isDir, &tmp->readOnly);
	if (res && (!strcmp(mode,"a+b") && twDirectory_FileExists(realpath))) {
		TW_LOG(TW_ERROR,"twFileManager_OpenFile: Error getting file info.  Error: %d", res);
		twFileManager_CloseFile(tmp);
		return NULL;
	}
	tmp->handle = s_twFile_FOpen(tmp->realPath, mode);
	if (!tmp->handle) {
		TW_LOG(TW_ERROR,"twFileManager_OpenFile: Opening file.  Error: %d", twDirectory_GetLastError());
		s_twFileManager_CloseFile(tmp);
		return NULL;
	}
	tmp->lastFileXferActivity = twGetSystemMillisecondCount();
	twMutex_Lock(fm->mtx);
	twList_Add(fm->openFiles, tmp);
	twMutex_Unlock(fm->mtx);
	return tmp;
}

void twFileManager_CloseFile(void * file) {
	ListEntry * le = NULL;
	twFile * f = NULL;
	twFile * tmp = (twFile *) file;
	if (!tmp->realPath) {
    		TW_LOG(TW_ERROR,"twFileManager_CloseFile: file has no name, cannot close file");
		return;
	}
    	TW_LOG(TW_TRACE,"twFileManager_CloseFile: Closing file: %s", (tmp && tmp->name) ? tmp->name : "UNKNOWN");
	if (!tmp ||!fm) return;
	if (fm->mtx) twMutex_Lock(fm->mtx);
	if (fm->openFiles) {
		le = twList_Next(fm->openFiles, NULL);
		while (le && le->value) {
			f = (twFile *)(le->value);
			if (!f->realPath) {
    			TW_LOG(TW_WARN,"twFileManager_CloseFile: file without path found in open file list, removing file from list and deleting");
				twList_Remove(fm->openFiles, le, FALSE);
				twFile_Delete(f);	
			} else if (!strcmp(f->realPath, tmp->realPath)) {
				twList_Remove(fm->openFiles, le, FALSE);
				break;
			}
			le = twList_Next(fm->openFiles, le);
		}
	}
	if (tmp) twFile_Delete(tmp);
	if (fm->mtx) twMutex_Unlock(fm->mtx);
}

void twFileManager_CheckStalledTransfers() {
	twFileManager_GetOpenFile(TW_FAKE_THINGNAME, NULL, NULL, TW_FAKE_TID, NULL);
}

typedef struct twFileManager_BuildCallbacksToInvokeListForEachParams {
	twFileTransferInfo *fti;
	twList *callbacksToInvoke;
} twFileManager_BuildCallbacksToInvokeListForEachParams;

typedef struct twFileManager_InvokeCallbacksForEachParams {
	twFileTransferInfo *fti;
	char rcvd;
} twFileManager_InvokeCallbacksForEachParams;

typedef struct twFileManager_SplitCallbacksListForEachParams {
	twList *registeredCallbacks;
	twList *unregisteredCallbacks;
} twFileManager_SplitCallbacksListForEachParams;

int twFileManager_BuildCallbacksToInvokeListForEachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *userData) {
	twFileXferCallback *fxc = (twFileXferCallback*)currentValue;
	twFileManager_BuildCallbacksToInvokeListForEachParams *params = (twFileManager_BuildCallbacksToInvokeListForEachParams*)userData;
	twFileTransferInfo *fti = params->fti;
	twList *callbacksToInvoke = params->callbacksToInvoke;
	twMutex_Lock(fxc->mtx);
	if (!fxc->unregistered && (!fxc->filter || IsWildcardMatch(fxc->filter, fti->targetFile, TRUE))) {
		twList_Add(callbacksToInvoke, fxc);
		++(fxc->refCount);
	}
	twMutex_Unlock(fxc->mtx);
	return TW_FOREACH_CONTINUE;
}

int twFileManager_InvokeCallbacksForEachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *userData) {
	twFileXferCallback *fxc = (twFileXferCallback*)currentValue;
	twFileManager_InvokeCallbacksForEachParams *params = (twFileManager_InvokeCallbacksForEachParams*)userData;
	char rcvd = params->rcvd;
	twFileTransferInfo *fti = params->fti;
	twMutex_Lock(fxc->mtx);
	if (!fxc->unregistered) {
		fxc->cb(rcvd, fti, fxc->userdata);
		if (fxc->onceOnly) {
			fxc->unregistered = TRUE;
			fxc->cb = NULL;
		}
	}
	--(fxc->refCount);
	twMutex_Unlock(fxc->mtx);
	return TW_FOREACH_CONTINUE;
}

int twFileManager_SplitCallbacksListForEachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *userData) {
	twFileXferCallback *fxc = (twFileXferCallback*)currentValue;
	twFileManager_SplitCallbacksListForEachParams *params = (twFileManager_SplitCallbacksListForEachParams*)userData;
	twList *registeredCallbacks = params->registeredCallbacks;
	twList *unregisteredCallbacks = params->unregisteredCallbacks;
	twMutex_Lock(fxc->mtx);
	/* Retain any callbacks that are registered, or have a reference from another MakeFileCallback */
	if (!fxc->unregistered || fxc->refCount > 1) {
		twList_Add(registeredCallbacks, fxc);
	} else {
		twList_Add(unregisteredCallbacks, fxc);
	}
	twMutex_Unlock(fxc->mtx);
	return TW_FOREACH_CONTINUE;
}

void twFileManager_MakeFileCallback(char rcvd, twFileTransferInfo * fti) {
	twList *callbacksToInvoke = NULL;
	twList *registeredCallbacks = NULL;
	twList *unregisteredCallbacks = NULL;
	twList *tmp = NULL;
	twFileManager_BuildCallbacksToInvokeListForEachParams *buildCallbacksToInvokeListParams = NULL;
	twFileManager_InvokeCallbacksForEachParams *invokeCallbacksParams = NULL;
	twFileManager_SplitCallbacksListForEachParams *splitCallbacksListParams = NULL;
	int i = 0;

	if (!fm || !fm->callbacks || !fti || !fti->targetFile) {
		return;
	}

	/* Build a list of callbacks to invoke with fm locked */
	callbacksToInvoke = twList_Create(twFileXferCallback_Delete);
	buildCallbacksToInvokeListParams = TW_MALLOC(sizeof(twFileManager_BuildCallbacksToInvokeListForEachParams));
	buildCallbacksToInvokeListParams->callbacksToInvoke = callbacksToInvoke;
	buildCallbacksToInvokeListParams->fti = fti;
	twMutex_Lock(fm->mtx);
	twList_Foreach(fm->callbacks, twFileManager_BuildCallbacksToInvokeListForEachHandler, buildCallbacksToInvokeListParams);
	twMutex_Unlock(fm->mtx);
	TW_FREE(buildCallbacksToInvokeListParams);

	/* Invoke the callbacks with fm unlocked */
	invokeCallbacksParams = TW_MALLOC(sizeof(twFileManager_InvokeCallbacksForEachParams));
	invokeCallbacksParams->fti = fti;
	invokeCallbacksParams->rcvd = rcvd;
	twList_Foreach(callbacksToInvoke, twFileManager_InvokeCallbacksForEachHandler, invokeCallbacksParams);
	TW_FREE(invokeCallbacksParams);
	twList_ClearEntries(callbacksToInvoke); /* Clear the list without deleting entry values */
	twList_Delete(callbacksToInvoke); /* Clean up remaining list memory */

	/* Split the callbacks list into registered/referenced and unregistered/unreferenced lists with fm locked */
	registeredCallbacks = twList_Create(twFileXferCallback_Delete);
	unregisteredCallbacks = twList_Create(twFileXferCallback_Delete);
	splitCallbacksListParams = TW_MALLOC(sizeof(twFileManager_SplitCallbacksListForEachParams));
	splitCallbacksListParams->registeredCallbacks = registeredCallbacks;
	splitCallbacksListParams->unregisteredCallbacks = unregisteredCallbacks;
	twMutex_Lock(fm->mtx);
	twList_Foreach(fm->callbacks, twFileManager_SplitCallbacksListForEachHandler, splitCallbacksListParams);
	TW_FREE(splitCallbacksListParams);

	/* With fm still locked, replace the old callbacks list with the new one */
	tmp = fm->callbacks;
	fm->callbacks = registeredCallbacks;
	/* Delete the old callbacks list without deleting the fxc nodes */
	twList_ClearEntries(tmp); /* Clear the list without deleting entry values */
	twList_Delete(tmp); /* Clean up remaining list memory */

	/* Delete unregistered/unreferenced callbacks */
	twList_Delete(unregisteredCallbacks);
	twMutex_Unlock(fm->mtx);
}

int twFileManager_RegisterFileCallback(file_cb cb, char * filter, char onceOnly, void * userdata) {
	twFileXferCallback * fxc = NULL;
	if (!cb) return TW_OK;
	if (!fm || !fm->mtx || !fm->callbacks) {
		TW_LOG(TW_ERROR,"twFileManager_RegisterRecvFileCallback: FileTransferManager not initialized");
		return TW_FILE_XFER_MANAGER_NOT_INITIALIZED;
	}
	fxc = twFileXferCallback_Create(cb, filter, onceOnly, userdata);
	if (!fxc) {
		TW_LOG(TW_ERROR,"twFileManager_RegisterRecvFileCallback: Error creating twFileXferCallback structure");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	twMutex_Lock(fm->mtx);
	twList_Add(fm->callbacks, fxc);
	twMutex_Unlock(fm->mtx);
	return TW_OK;
}

typedef struct twFileManager_UnregisterFileCallbackForEachParams {
	file_cb cb;
	char *filter;
	void *data;
} twFileManager_UnregisterFileCallbackForEachParams;

int twFileManager_UnregisterFileCallback_ForEachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *userData) {
	twFileManager_UnregisterFileCallbackForEachParams *params = (twFileManager_UnregisterFileCallbackForEachParams*)userData;
	file_cb cb = params->cb;
	char *filter = params->filter;
	void *data = params->data;
	twFileXferCallback *fxc = (twFileXferCallback*)currentValue;
	twMutex_Lock(fxc->mtx);
	if (fxc->cb == cb && fxc->userdata == data && (!filter || !strcmp(fxc->filter, filter))) {
		fxc->unregistered = TRUE;
		fxc->cb = NULL;
		twMutex_Unlock(fxc->mtx);
		return TW_FOREACH_EXIT;
	}
	twMutex_Unlock(fxc->mtx);
	return TW_FOREACH_CONTINUE;
}

int twFileManager_UnregisterFileCallback(file_cb cb, char * filter, void * userdata) {
	twFileManager_UnregisterFileCallbackForEachParams * params = NULL;
	if (!fm || !fm->mtx || !fm->callbacks) {
		TW_LOG(TW_ERROR,"twFileManager_RegisterRecvFileCallback: FileTransferManager not initialized");
		return TW_FILE_XFER_MANAGER_NOT_INITIALIZED;
	}
	params = TW_MALLOC(sizeof(twFileManager_UnregisterFileCallbackForEachParams));
	params->cb = cb;
	params->filter = filter;
	params->data = userdata;
	twMutex_Lock(fm->mtx);
	twList_Foreach(fm->callbacks, twFileManager_UnregisterFileCallback_ForEachHandler, params);
	twMutex_Unlock(fm->mtx);
	TW_FREE(params);
	return TW_OK;
}

int twFileManager_TransferFile(const char * sourceRepo, const char * sourcePath, const char * sourceFile,
						   const char * targetRepo, const char * targetPath, const char * targetFile,
						   uint32_t timeout, char asynch, char ** tid) {

	twDataShape * ds = NULL;
	twInfoTable * it = NULL;
	twInfoTableRow * row = NULL;
	twInfoTable * transferInfo = NULL;
	int res = 0;

	/* Error out if timeout is not in bounds for timeout argument to twApi_InvokeService */
	if (timeout > INT32_MAX) {
		TW_LOG(
			TW_ERROR,
			"twFileManager_TransferFile: Timeout exceeds maximum allowable value (%u ms > %d ms)",
			timeout,
			INT32_MAX
		);
		return TW_INVALID_PARAM;
	}

	if (!sourceRepo || !sourcePath || !sourceFile || !targetRepo || !targetPath || !targetFile || !tid) {
		TW_LOG(TW_ERROR,"twFileManager_TransferFile: Missing parameters");
		return TW_INVALID_PARAM;
	}
	/* Create an infotable out of the parameters */
	ds = twDataShape_Create(twDataShapeEntry_Create("sourceRepo", NULL, TW_STRING));
	if (!ds) {
		TW_LOG(TW_ERROR,"twFileManager_TransferFile: Error creating datashape");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	res = twDataShape_AddEntry(ds, twDataShapeEntry_Create("sourcePath", NULL, TW_STRING));
	res |= twDataShape_AddEntry(ds, twDataShapeEntry_Create("sourceFile", NULL, TW_STRING));
	res |= twDataShape_AddEntry(ds, twDataShapeEntry_Create("targetRepo", NULL, TW_STRING));
	res |= twDataShape_AddEntry(ds, twDataShapeEntry_Create("targetPath", NULL, TW_STRING));
	res |= twDataShape_AddEntry(ds, twDataShapeEntry_Create("targetFile", NULL, TW_STRING));
	res |= twDataShape_AddEntry(ds, twDataShapeEntry_Create("async", NULL, TW_BOOLEAN));
	res |= twDataShape_AddEntry(ds, twDataShapeEntry_Create("timeout", NULL, TW_INTEGER));
	if (res) {
		TW_LOG(TW_ERROR,"twFileManager_TransferFile: Error adding entry to datashape");
		twDataShape_Delete(ds);
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	it = twInfoTable_Create(ds); 
	if (!it) {
		TW_LOG(TW_ERROR,"twFileManager_TransferFile: Error creating infotable");
		twDataShape_Delete(ds);
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(sourceRepo, TRUE));
	if (!row) {
		TW_LOG(TW_ERROR,"twFileManager_TransferFile: Error creating infotable row");
		twInfoTable_Delete(it);
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	res = twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(sourcePath, TRUE));
	res |= twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(sourceFile, TRUE));
	res |= twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(targetRepo, TRUE));
	res |= twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(targetPath, TRUE));
	res |= twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(targetFile, TRUE));
	res |= twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(asynch));
	res |= twInfoTableRow_AddEntry(row, twPrimitive_CreateFromInteger(timeout));
	if (res) {
		TW_LOG(TW_ERROR,"twFileManager_TransferFile: Error adding entry to infotable row");
		twInfoTable_Delete(it);
		twInfoTableRow_Delete(row);
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	twInfoTable_AddRow(it,row);
	/* Make the service call */
	res = s_twApi_InvokeService(
		TW_SUBSYSTEM,
		"FileTransferSubsystem",
		"Copy",
		it,
		&transferInfo,
		timeout ? timeout : -1,
		FALSE
	);
	twInfoTable_Delete(it);
	if (res || !transferInfo) {
		TW_LOG(TW_ERROR,"twFileManager_TransferFile: Error invoking Copy service on FileTransferSubsystem");
		if (transferInfo) twInfoTable_Delete(transferInfo);
		return res;
	}
	/* Grab the tid */
	res = twInfoTable_GetString(transferInfo,"transferId",0, tid);
	if (res) {
		TW_LOG(TW_ERROR,"twFileManager_TransferFile: Error getting tid from transferInfo");
		twInfoTable_Delete(transferInfo);
		transferInfo = 0;
		return res;
	}
	/* If this is not async, grab the current state and see if the transfer succeeded */
	if (!asynch) {
		char * state = 0;
		res = twInfoTable_GetString(transferInfo,"state", 0, &state);
		if (res) {
			TW_LOG(TW_ERROR,"twFileManager_TransferFile: Error getting state from transferInfo");
			twInfoTable_Delete(transferInfo);
			if (state) TW_FREE(state);
			transferInfo = 0;
			return res;
		}
		if (strcmp(state,"VALIDATED")) {
			TW_LOG(TW_ERROR,"twFileManager_TransferFile: Synchronous file transfer failed. State = %s", state);
			twInfoTable_Delete(transferInfo);
			transferInfo = 0;
			if (state) TW_FREE(state);
			return FILE_TRANSFER_FAILED;
		}
		if (state) TW_FREE(state);
	}
	twInfoTable_Delete(transferInfo);
	return 0;
}

int twFileManager_SendFile(const char * sourceRepo, const char * sourcePath, const char * sourceFile,
						   const char * targetRepo, const char * targetPath, const char * targetFile,
						   uint32_t timeout, char asynch, char ** tid) {

	char * realPath = NULL;
	if (!sourceRepo || !sourcePath || !sourceFile || !targetRepo || !targetPath || !targetFile || !tid) {
		TW_LOG(TW_ERROR,"twFileManager_SendFile: Missing parameters");
		return TW_INVALID_PARAM;
	}
	/* Check to see if this file exists before going any further */
	realPath = twFileManager_GetRealPath(sourceRepo, sourcePath, sourceFile);
	if (!twDirectory_FileExists(realPath)) {
		TW_LOG(TW_ERROR,"twFileManager_SendFile: File %s:%s%c%s does not exist", 
						sourceRepo, sourcePath, TW_FILE_DELIM, sourceFile);
		TW_FREE(realPath);
		return TW_FILE_NOT_FOUND;
	}
	TW_FREE(realPath);
	return twFileManager_TransferFile(sourceRepo, sourcePath, sourceFile, targetRepo, targetPath, 
		                               targetFile, timeout, asynch, tid);
}

int twFileManager_GetFile(const char * sourceRepo, const char * sourcePath, const char * sourceFile,
						   const char * targetRepo, const char * targetPath, const char * targetFile,
						   uint32_t timeout, char asynch, char ** tid) {

	return twFileManager_TransferFile(sourceRepo, sourcePath, sourceFile, targetRepo, targetPath, 
		                               targetFile, timeout, asynch, tid);
}

twList * twFileManager_ListEntities(const char * entityName, const char * path, const char * namemask, char returnType) {
	twList * list = NULL;
	/* Inputs */
	char * fixedPath = fixPath(path);
	char * realPath = s_twFileManager_GetRealPath(entityName, fixedPath, NULL);
    if (!entityName || !fixedPath || !realPath) {
		TW_LOG(TW_ERROR, "twFileManager_ListEntities: Invalid input param(s)");
		if (fixedPath) TW_FREE(fixedPath);
		if (realPath) TW_FREE(realPath);
		return NULL;
	}
	if (!namemask) namemask = "*";
	/* Outputs */
	list = s_twList_Create(twFile_Delete);
	if (!list) {
		TW_LOG(TW_ERROR, "twFileManager_ListEntities: Error creating output list");
		TW_FREE(fixedPath);
		TW_FREE(realPath);
		return NULL;
	}
	/* Perform the function */
	{
		TW_DIR hnd = 0;
		int res = 0;
		twFile * tmp = NULL;
		
		/* Handle the case of "/" */
		if (fixedPath[0] == TW_FILE_DELIM) {
			if (returnType == LIST_FILES) {
				/* There are no files at the virtual root directory */
				TW_FREE(fixedPath);
				TW_FREE(realPath);
				return list;
			}
			return twFileManager_ListVirtualDirs(entityName);
		}
		if (fixedPath[strnlen(fixedPath, MAX_PATH_LEN) - 1] == TW_FILE_DELIM) fixedPath[strnlen(fixedPath, MAX_PATH_LEN) - 1] = 0;
		/* Get the first file in the dir */
		tmp = twFile_Create();
		if (!tmp) {
			twList_Delete(list);
			TW_FREE(fixedPath);
			TW_FREE(realPath);
			TW_LOG(TW_ERROR, "twFileManager_ListEntities: Allocating twFile structure");
			return NULL;
		}
		hnd = twDirectory_IterateEntries(realPath, hnd, &tmp->name, &tmp->size, 
											&tmp->lastModified, &tmp->isDir, &tmp->readOnly);
		while (hnd && tmp && tmp->name) {
			if ((returnType == LIST_FILES && !tmp->isDir) || (returnType == LIST_DIRS && tmp->isDir) || returnType == LIST_ALL) {
				if ((IsWildcardMatch(namemask, tmp->name, TRUE)) || ((tmp->isDir && strcmp(tmp->name, ".") && strcmp(tmp->name, "..")))) {
					/* Add this to our list */
					TW_LOG(TW_TRACE,"twFileManager_ListEntities: Adding file %s%c%s to list.", path, TW_FILE_DELIM, tmp->name);
					tmp->realPath = duplicateString(realPath);
					tmp->virtualPath = duplicateString(fixedPath);
					twList_Add(list, tmp);
				} else {
					twFile_Delete(tmp);
				}
			} else {
				twFile_Delete(tmp);
			}
			/* Get ready for the next iteration */
			tmp = twFile_Create();
			if (!tmp) {
				twList_Delete(list);
				TW_FREE(fixedPath);
				TW_FREE(realPath);
				TW_LOG(TW_ERROR, "twFileManager_ListEntities: Allocating twFile structure");
				return NULL;
			}
			hnd = twDirectory_IterateEntries(realPath, hnd, &tmp->name, &tmp->size, 
				&tmp->lastModified, &tmp->isDir, &tmp->readOnly);
		}
		if (tmp) twFile_Delete(tmp);
		res = s_twDirectory_GetLastError();
		if (res != ERROR_NO_MORE_FILES) {
			TW_LOG(TW_ERROR, "twListEntities: Error iterating through %s.  Error: %d", path, res);
			twList_Delete(list);
			TW_FREE(fixedPath);
			TW_FREE(realPath);
			TW_LOG(TW_ERROR, "twFileManager_ListEntities: Allocating twFile structure");
			return NULL;
		}
	} 
	TW_FREE(fixedPath);
	TW_FREE(realPath);
	return list;
}

enum msgCodeEnum twFileManager_FinishFileTransfer(
    const char * entityName,
    twFileTransferInfo * ft,
    const char * path,
    const char * adjustedPath
) {
    /* an additional twFile * was created as well as additional strings to hold path members in order to allow the deletion of multiple
    file manager open files list entries (fm->openFiles). The specific files being targeted are related to transfers while using identifiers,
    which could result in two open file entries being created (one using the tid and Identifier, the other using only the ThingName).
    This could be necessary in the following cases when using identifiers:

    total transfer < idle transfer timeout -- previous behavior-- both open files will need to be deleted
    total transfer > idle transfer timeout -- only the ThingName list entry will need

    first transfer end < idle transfer timeout, second transfer start > first idle transfer timeout -- only the ThingName list entry will need
    first transfer end < idle transfer timeout, second transfer start < first idle transfer timeout -- both open files will need to be deleted

    */

    twFile * f1 = NULL;
    twFile * f2 = NULL;
	TW_LOG(TW_INFO,"twFileManager_FinishFileTransfer: 1");

    twMutex_Lock(fm->fileTransferMtx);

    TW_LOG(
        TW_DEBUG,
        "twFileManager_FinishFileTransfer: Finishing file transfer %s",
        ft->transferId ? ft->transferId : "tid missing"
    );

    /* Perform the function */
    /* attempt to find an open file based off of the tid */
	TW_LOG(TW_INFO,"twFileManager_FinishFileTransfer: 2");
    f1 = s_twFileManager_GetOpenFile(entityName, NULL, NULL, ft->transferId, NULL);
    if(path && adjustedPath) {
        /* attempt to find an open file based off of the file path */
		TW_LOG(TW_INFO,"twFileManager_FinishFileTransfer: 3");
		f2 = s_twFileManager_GetOpenFile(entityName, adjustedPath, NULL, NULL, NULL);
    }

    if (!f1 && !f2){
        TW_LOG(
            TW_ERROR,
            "twFileManager_FinishFileTransfer: File: %s not found in open files list",
            ft->targetPath
        );
        twMutex_Unlock(fm->fileTransferMtx);
        return TWX_INTERNAL_SERVER_ERROR;
    }

    TW_LOG(
        TW_AUDIT,
        "twFileManager_FinishFileTransfer: %s.  File: %s",
        ft->state,
        ft->targetPath
    );

    /* Make only one callback if it is defined */
    /* moved callbacks above file close operations, to avoid potential memory corruption when accessing
    fx->openForRead */
    if (f1) {
		TW_LOG(TW_INFO,"twFileManager_FinishFileTransfer: 4");
		s_twFileManager_MakeFileCallback(!f1->openForRead, ft);
    } else if (f2) {
		TW_LOG(TW_INFO,"twFileManager_FinishFileTransfer: 5");
		s_twFileManager_MakeFileCallback(!f2->openForRead, ft);
    }

    /* attempt to close both files if necessary */
    /* we need to delete both files, but only if they are not identical list entries */
    if (f2 &&  (f1 != f2) ) {
		TW_LOG(TW_INFO,"twFileManager_FinishFileTransfer: 6");
		/* if f2 is valid and unique, close it */
        s_twFileManager_CloseFile(f2);
    }
    if (f1) {
        /* delete f1 if it is valid */
		TW_LOG(TW_INFO,"twFileManager_FinishFileTransfer: 7");

		s_twFileManager_CloseFile(f1);
    }
	TW_LOG(TW_INFO,"twFileManager_FinishFileTransfer: 8");
    twMutex_Unlock(fm->fileTransferMtx);
    return TWX_SUCCESS;
}

/* creating a function in order to support stubbing of fopen macro*/
TW_FILE_HANDLE twFile_FOpen(const char * name, const char * mode) {
	return TW_FOPEN(name, mode);
}