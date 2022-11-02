/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx File Transfer
 */

#include "twFileManager.h"
#include "twLogger.h"
#include "twInfoTable.h"
#include "twDefinitions.h"
#include "twApi.h"
#include "stringUtils.h"
#include "wildcard.h"
#include "tomcrypt.h"
#include "twTls.h"
#include "twApiStubs.h"

#define MAX_PATH_LEN 4096

/********************************/
/*       Helper Functions       */
/********************************/
void cleanUpPaths(char **path, char **realPath, char **sourcePath, char **realSourcePath, char **targetPath, char **realTargetPath, char ** adjustedPath, char ** realAdjustedPath) {
	if (path && *path) {
		TW_FREE(*path);
		*path = NULL;
	}
	if (adjustedPath && *adjustedPath) {
		TW_FREE(*adjustedPath);
		*adjustedPath = NULL;
	}
	if (realAdjustedPath && *realAdjustedPath) {
		TW_FREE(*realAdjustedPath);
		*realAdjustedPath = NULL;
	}
	if (realPath && *realPath) {
		TW_FREE(*realPath);
		*realPath = NULL;
	}
	if (sourcePath && *sourcePath) {
		TW_FREE(*sourcePath);
		*sourcePath = NULL;
	}
	if (realSourcePath && *realSourcePath) {
		TW_FREE(*realSourcePath);
		*realSourcePath = NULL;
	}
	if (targetPath && *targetPath) {
		TW_FREE(*targetPath);
		*targetPath = NULL;
	}
	if (realTargetPath && *realTargetPath) {
		TW_FREE(*realTargetPath);
		*realTargetPath = NULL;
	}
}

int isPre5_1Server = 0;
char * adjustPath(const char * entityName, char * path, char fromStartTransfer) {
	/*
		StartFileTransfer does not prepend the FILE_XFER_STAGING_DIR, so we need to do that if
		this is a write.  There is also an inconsistency in older servers that we need to address
		here as older servers use the standard delimiter and adds .part onto the path in the
		StartFileTransfer request and use that path for Create/Read/Write requests.  Newer servers
		do not tack on the .part file in the StartFileTransfer request but do in Create/Read/Write
		requests. It also prepends the ThingName to the path. In addition those Create/Read/Write
		requests use '.' as the delimiter to flatten the staging directory.

		                              Old Server                           New Server
        StartFileTransfer      <vdir>/path/to/file.part                 <vdir>/path/to/file
		GetFileInfo         <staging>/<vdir>.path/to/file.part  <staging>/ThingName.<vdir>.path.to.file.part
		CreateBinaryFile    <staging>/<vdir>/path/to/file.part  <staging>/ThingName.<vdir>.path.to.file.part
		WriteBinaryFile     <staging>/<vdir>/path/to/file.part  <staging>/ThingName.<vdir>.path.to.file.part
		ReadBinaryFile            <vdir>/path/to/file                   <vdir>/path/to/file
	*/
	/* Want this to be: <staging>/ThingName.<vdir>.path.to.file.part */
	char * adjustedPath = NULL;
	char * dotPart = NULL;
	char * newPath  = NULL;
	size_t len = strnlen(path, MAX_PATH_LEN);
	size_t stageDir = fromStartTransfer ? 0 : strnlen(TW_VIRTUAL_STAGING_DIR, MAX_PATH_LEN) + 1;
	/* Change the delimiter to a '.' */
	if (!isPre5_1Server && !fromStartTransfer) {
		return duplicateString(path);
	}
	newPath = duplicateString(path);
	if (!newPath) {
		TW_LOG(TW_ERROR, "adjustPath: Unable to allocate memory for newPath.");
		return NULL;
	}
	while (len-- > stageDir) {
		if (newPath[len - 1] == '/' || newPath[len - 1] == '\\') newPath[len - 1] = '.';
	}
	adjustedPath = (char *)TW_CALLOC(strnlen(newPath, MAX_PATH_LEN) + 1 + strnlen(entityName, MAX_PATH_LEN) + strnlen(TW_VIRTUAL_STAGING_DIR, MAX_PATH_LEN) + 6, 1);
	if (!adjustedPath) {
		TW_LOG(TW_ERROR, "adjustPath: Unable to allocate memory for adjustedPath.");
		return NULL;
	}
	/* Start with the staging dir */
	strncat(adjustedPath, TW_VIRTUAL_STAGING_DIR, MAX_PATH_LEN);
	strncat(adjustedPath, TW_FILE_DELIM_STR, MAX_PATH_LEN);
	if (fromStartTransfer) {
		/* Check to see if this is an old or new server by looking for '.part' */
		if(entityName[0] == '*') {
			strncat(adjustedPath, &entityName[1], MAX_PATH_LEN);
		} else {
			strncat(adjustedPath, entityName, MAX_PATH_LEN);
		}
		strncat(adjustedPath, newPath, MAX_PATH_LEN);
		dotPart = strrchr(newPath, '.');
		if (dotPart && !strcmp(dotPart,".part")) {
			/* This is an old server. */
			isPre5_1Server = 1;
			TW_LOG(TW_WARN, "adjustPath:setting isPre5_1Server = %d.", isPre5_1Server);
		} else {
			/* This is a new server */
			strncat(adjustedPath, ".part", MAX_PATH_LEN);
		}
	} else {
		strncat(adjustedPath, entityName, MAX_PATH_LEN);
		strncat(adjustedPath, ".", MAX_PATH_LEN);
		strncat(adjustedPath, newPath + strnlen(TW_VIRTUAL_STAGING_DIR, MAX_PATH_LEN) + 1,  MAX_PATH_LEN);
	}
	TW_LOG(TW_TRACE, "adjustPath: \nOriginal Path: %s. \nAdjustedPath: %s", path, adjustedPath);
	TW_FREE(newPath);
	return adjustedPath;
}

int getPaths(twInfoTable * params, const char * entityName, char ** path, char ** realPath,
			 char ** sourcePath, char ** realSourcePath, char ** targetPath, char ** realTargetPath,
			 char ** adjustedPath, char ** realAdjustedPath, char fromStartTransfer, const char * pathFieldName) {
				 /*	added pathFieldName  to getPaths inputs to allow searching for a different path field name in the supplied params infotable
					this is intended to allow twFinishFileTransfer to search for an open file based on path instead of tid, in this context the
					desired path is stored in the "targetPath" column of the params infotable, not the default "path" column. In order to maintain
					support for getPaths in other functions, a NULL pathFieldName will default to "path" and the funciton will execute normally*/
	if (!params || !entityName) return TW_INVALID_PARAM;
	if (path) {
		if (twInfoTable_GetString(params, (pathFieldName ? pathFieldName : "path"), 0, path)) return TW_BAD_REQUEST;

        if (strstr(*path, "..")) {
            TW_LOG(TW_ERROR, "getPaths: No relative paths are permitted.");
            return TW_BAD_REQUEST;
        }

		if (adjustedPath) {
			*adjustedPath = adjustPath(entityName, *path, fromStartTransfer);
			if (!adjustedPath) {
				cleanUpPaths(path, realPath, sourcePath, realSourcePath, targetPath, realTargetPath, adjustedPath, realAdjustedPath);
				return TWX_INTERNAL_SERVER_ERROR;
			}
			if (realAdjustedPath) {
				*realAdjustedPath =  twFileManager_GetRealPath(entityName, *adjustedPath, NULL);
				if (!realAdjustedPath) {
					cleanUpPaths(path, realPath, sourcePath, realSourcePath, targetPath, realTargetPath, adjustedPath, realAdjustedPath);
					return TW_BAD_REQUEST;
				}
			}
		}
		if (realPath) {
			*realPath =  twFileManager_GetRealPath(entityName, *path, NULL);
			if (!realPath) {
				cleanUpPaths(path, realPath, sourcePath, realSourcePath, targetPath, realTargetPath, adjustedPath, realAdjustedPath);
				return TW_BAD_REQUEST;
			}
		}
	}
	if (sourcePath) {
		if (twInfoTable_GetString(params, "sourcePath", 0, sourcePath)) {
			cleanUpPaths(path, realPath, sourcePath, realSourcePath, targetPath, realTargetPath, adjustedPath, realAdjustedPath);
			return TW_BAD_REQUEST;
		}
		if (realSourcePath) {
			*realSourcePath =  twFileManager_GetRealPath(entityName, *sourcePath, NULL);
			if (!realSourcePath) {
				cleanUpPaths(path, realPath, sourcePath, realSourcePath, targetPath, realTargetPath, adjustedPath, realAdjustedPath);
				return TW_BAD_REQUEST;
			}
		}
	}
	if (targetPath) {
		if (twInfoTable_GetString(params, "targetPath", 0, targetPath)) {
			cleanUpPaths(path, realPath, sourcePath, realSourcePath, targetPath, realTargetPath, adjustedPath, realAdjustedPath);
			return TW_BAD_REQUEST;
		}
		if (realTargetPath) {
			*realTargetPath =  twFileManager_GetRealPath(entityName, *targetPath, NULL);
			if (!realTargetPath) {
				cleanUpPaths(path, realPath, sourcePath, realSourcePath, targetPath, realTargetPath, adjustedPath, realAdjustedPath);
				return TW_BAD_REQUEST;
			}
		}
	}
	return TW_OK;
}

int listVirtualDirsInInfoTable(const char * entityName, twInfoTable * it, char fullDetails) {
	twList * list = NULL;
	ListEntry * le = NULL;
	char fileType[8];
	twInfoTableRow * row = NULL;
	list = twFileManager_ListVirtualDirs(entityName);
    if (!it || !list) {
		TW_LOG(TW_ERROR, "listVirtualDirsInInfoTable: Invalid input param(s) of could not get list");
		if (list) twList_Delete(list);
		return TW_INVALID_PARAM;
	}
	le = twList_Next(list, NULL);
	while (le && le->value) {
		char * fullPath = NULL;
		twFile * tmp = (twFile *)(le->value);
		strcpy(fileType,"D");
		/* Fill in the info table row for this entry */
		TW_LOG(TW_TRACE,"listVirtualDirsInInfoTable: Adding dir %s%c%s to list.", tmp->realPath, TW_FILE_DELIM, tmp->name);
		row = twInfoTableRow_Create(twPrimitive_CreateFromString(tmp->name, TRUE));
		if (!row) {
			TW_LOG(TW_ERROR, "listVirtualDirsInInfoTable: Error allocating infotable row");
			twInfoTable_Delete(it);
			it = 0;
			twList_Delete(list);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		fullPath = (char *)TW_CALLOC(strnlen(tmp->name, MAX_PATH_LEN) + 2, 1);
		if (fullPath) {
			strncpy(fullPath,"/", 1);
			strncat(fullPath, tmp->name, MAX_PATH_LEN);
		}
		twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(fullPath, FALSE));
		if (fullDetails) {
			twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber(tmp->size));
			twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(tmp->readOnly ? "D+RO" : "D", TRUE));
			twInfoTableRow_AddEntry(row, twPrimitive_CreateFromDatetime(tmp->lastModified));
		} else {
			twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString("/", TRUE));
		}
		if (twInfoTable_AddRow(it, row)) {
			TW_LOG(TW_ERROR, "listVirtualDirsInInfoTable: Error adding infotable row");
			twInfoTable_Delete(it);
			it = 0;
			twList_Delete(list);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		le = twList_Next(list, le);
	}
	twList_Delete(list);
	return 0;
}

enum msgCodeEnum twListEntities(const char * entityName, twInfoTable * params, twInfoTable ** content, char files) {
	twDataShapeEntry * dse = NULL;
	twDataShape * ds = NULL;
	/* Inputs */
	char * path = NULL;
	char * realPath = NULL;
	char * nameMask = NULL;
    if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twListEntities: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	if (files) {
		twInfoTable_GetString(params, "nameMask", 0, &nameMask);
		if (!nameMask) nameMask = duplicateString("*");
	}
	/* Outputs */
	dse = twDataShapeEntry_Create("name","",TW_STRING);
	if (!dse) {
		TW_LOG(TW_ERROR, "twListEntities: Error creating output datashape entry");
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		if (nameMask) TW_FREE(nameMask);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShapeEntry_AddAspect(dse,"isPrimaryKey", twPrimitive_CreateFromBoolean(TRUE));
	ds = twDataShape_Create(dse);
	if (!ds) {
		TW_LOG(TW_ERROR, "twListEntities: Error creating output datashape");
		twDataShapeEntry_Delete(dse);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		if (nameMask) TW_FREE(nameMask);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("path","",TW_STRING));
	if (files) {
		twDataShape_AddEntry(ds, twDataShapeEntry_Create("size","",TW_NUMBER));
		twDataShape_AddEntry(ds, twDataShapeEntry_Create("fileType","",TW_STRING));
		twDataShape_AddEntry(ds, twDataShapeEntry_Create("lastModifiedDate","",TW_DATETIME));
	} else {
		twDataShape_AddEntry(ds, twDataShapeEntry_Create("parentPath","",TW_STRING));
	}
    /* Create the output infotable */
	*content = twInfoTable_Create(ds);
	if (!*content) {
		TW_LOG(TW_ERROR, "twListEntities: Error creating output infotable");
		twDataShape_Delete(ds);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		if (nameMask) TW_FREE(nameMask);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	/* Perform the function */
	{
		twInfoTableRow * row = NULL;
		TW_DIR hnd = 0;
		char fileType[8];
		twFile tmp;
		int res = 0;
		char * fullPath = NULL;
		memset(&tmp, 0, sizeof(twFile));
		/* Handle the case of "/" */
		if (strnlen(path, MAX_PATH_LEN) == 1 && (path[0] == '/' || path[0] == '\\')) {
			if (nameMask) TW_FREE(nameMask);
			if (files) {
				/* There are no files at the virtual root directory */
				cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
				return TWX_SUCCESS;
			}
			res = listVirtualDirsInInfoTable(entityName, *content, FALSE);
			if (res) {
				twInfoTable_Delete(*content);
				*content = NULL;
				cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
				return TWX_INTERNAL_SERVER_ERROR;
			}
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
			return TWX_SUCCESS;
		}
		if (path[strnlen(path, MAX_PATH_LEN) - 1] == '/' || path[strnlen(path, MAX_PATH_LEN) - 1] == '\\') path[strnlen(path, MAX_PATH_LEN) - 1] = 0;
		hnd = twDirectory_IterateEntries(realPath, hnd, &tmp.name, &tmp.size,
											&tmp.lastModified, &tmp.isDir, &tmp.readOnly);
		while (hnd && tmp.name) {
			if ((files && !tmp.isDir) || (!files && tmp.isDir)) {
				if ((files && IsWildcardMatch(nameMask, tmp.name, TRUE)) || ((tmp.isDir && strcmp(tmp.name, ".") && strcmp(tmp.name, "..")))) {
					strcpy(fileType,"F");
					/* Fill in the info table row for this entry */
					TW_LOG(TW_TRACE,"twListEntities: Adding file %s%c%s to list.", path, TW_FILE_DELIM, tmp.name);
					row = twInfoTableRow_Create(twPrimitive_CreateFromString(tmp.name, FALSE));
					if (!row) {
						TW_LOG(TW_ERROR, "twListEntities: Error allocating infotable row");
						TW_FREE(tmp.name);
						twInfoTable_Delete(*content);
						*content = NULL;
						cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
						if (nameMask) TW_FREE(nameMask);
						return TWX_INTERNAL_SERVER_ERROR;
					}
					if(tmp.isDir) {
						fullPath = (char *)TW_CALLOC(strnlen(path, MAX_PATH_LEN) + strnlen(tmp.name, MAX_PATH_LEN) + 2, 1);
						if (fullPath) {
							strcpy(fullPath, path);
							strcat(fullPath,"/");
							strcat(fullPath, tmp.name);
						}
					} else {
						fullPath = (char *)TW_CALLOC(strnlen(path, MAX_PATH_LEN) + 1, 1);
						if (fullPath) {
							strcpy(fullPath, path);
						}
					}
					twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(fullPath, FALSE));
					if (files) {
						twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber((double)tmp.size));
						if (tmp.readOnly) strcat(fileType, "+RO");
						twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(fileType, TRUE));
						twInfoTableRow_AddEntry(row, twPrimitive_CreateFromDatetime(tmp.lastModified));
					} else {
						twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(path, TRUE));
					}
					if (twInfoTable_AddRow(*content, row)) {
						TW_LOG(TW_ERROR, "twListEntities: Error adding infotable row");
						twInfoTableRow_Delete(row);
						twInfoTable_Delete(*content);
                        *content = NULL;
						cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
						if (nameMask) TW_FREE(nameMask);
						return TWX_INTERNAL_SERVER_ERROR;
					}
				} else {
					TW_FREE(tmp.name);
				}
			} else TW_FREE(tmp.name);
			hnd = twDirectory_IterateEntries(realPath, hnd, &tmp.name, &tmp.size,
				&tmp.lastModified, &tmp.isDir, &tmp.readOnly);
		}
		res = twDirectory_GetLastError();
		if (res != ERROR_NO_MORE_FILES) {
			TW_LOG(TW_ERROR, "twListEntities: Error iterating through %s.  Error: %d", path, res);
		}
	}
	cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
	if (nameMask) TW_FREE(nameMask);
	return TWX_SUCCESS;
}

int listDirsInInfoTable(char * entityName, char * virtualPath, twInfoTable * it) {
	/* Need to iterate through the directory */
	twInfoTableRow * row = NULL;
	twFile * tmp = NULL;
	TW_DIR hnd = 0;
	char * realPath = NULL;
	if (!entityName || !it || !virtualPath) {
		TW_LOG(TW_ERROR, "listDirsInInfoTable: NULL parameter passed in");
		return TW_INVALID_PARAM;
	}
	realPath =  twFileManager_GetRealPath(entityName, virtualPath, NULL);
	if (!realPath) {
		TW_LOG(TW_ERROR, "listDirsInInfoTable: Error getting real path for %s", virtualPath);
		return TW_INVALID_PARAM;
	}
	tmp = twFile_Create();
	if (!tmp) {
		TW_LOG(TW_ERROR, "listDirsInInfoTable: error allocating twFile struct");
		TW_FREE(realPath);
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	hnd = twDirectory_IterateEntries(realPath, hnd, &tmp->name, &tmp->size,
										&tmp->lastModified, &tmp->isDir, &tmp->readOnly);
	while (tmp && hnd) {
		char * fullVirtualPath = NULL;
		char * fullVirtualParent = NULL;
		if (!strcmp(tmp->name,".") || !strcmp(tmp->name,"..") || !tmp->isDir) {
			twFile_Delete(tmp);
			tmp = twFile_Create();
			if (!tmp) {
				TW_LOG(TW_ERROR, "listDirsInInfoTable: error allocating twFile struct");
				TW_FREE(realPath);
				return TW_ERROR_ALLOCATING_MEMORY;
			}
			hnd = twDirectory_IterateEntries(realPath, hnd, &tmp->name, &tmp->size,
						&tmp->lastModified, &tmp->isDir, &tmp->readOnly);
			continue;
		}
		/* Fill in the info table row for this entry */
		TW_LOG(TW_TRACE,"listDirsInInfoTable: Adding dir %s%c%s to list.", tmp->realPath, TW_FILE_DELIM, tmp->name);
		row = s_twInfoTableRow_Create(twPrimitive_CreateFromString(tmp->name, TRUE));
		if (!row) {
			TW_LOG(TW_ERROR, "listDirsInInfoTable: Error allocating infotable row");
			TW_FREE(realPath);
			twFile_Delete(tmp);
			return TWX_INTERNAL_SERVER_ERROR;
		}

		fullVirtualParent = (char *)TW_CALLOC(strnlen(virtualPath, MAX_PATH_LEN) + 1, 1);
		fullVirtualPath   = (char *)TW_CALLOC(1 + strnlen(virtualPath, MAX_PATH_LEN) + 1 + strnlen(tmp->name, MAX_PATH_LEN) + 1, 1);

		if (!fullVirtualPath || !fullVirtualParent) {
			TW_LOG(TW_ERROR, "listDirsInInfoTable: Error allocating virtual path strings");
			TW_FREE(realPath);
			twFile_Delete(tmp);
			TW_FREE(fullVirtualPath);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		strcat(fullVirtualParent, virtualPath);
		strcat(fullVirtualPath, fullVirtualParent);
		strcat(fullVirtualPath, "/");
		strcat(fullVirtualPath, tmp->name);
		twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(fullVirtualPath, FALSE));
		twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(fullVirtualParent, FALSE));
		if (s_twInfoTable_AddRow(it, row)) {
			TW_LOG(TW_ERROR, "twGetDirectoryStructure: Error adding infotable row");
			twFile_Delete(tmp);
			TW_FREE(realPath);
			twInfoTableRow_Delete(row);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		/* recurse into this directory */
		if (s_listDirsInInfoTable(entityName, fullVirtualPath, it)) {
			TW_LOG(TW_ERROR, "twGetDirectoryStructure: Error adding directory %s into infoTable", fullVirtualPath);
			twFile_Delete(tmp);
			TW_FREE(realPath);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		twFile_Delete(tmp);
		tmp = s_twFile_Create();
		if (!tmp) {
			TW_LOG(TW_ERROR, "listDirsInInfoTable: error allocating twFile struct");
			TW_FREE(realPath);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		hnd = twDirectory_IterateEntries(realPath, hnd, &tmp->name, &tmp->size,
			&tmp->lastModified, &tmp->isDir, &tmp->readOnly);
	}
	if(tmp) twFile_Delete(tmp);
	TW_FREE(realPath);
	return TW_OK;
}

/********************************/
/*     Service Callbacks        */
/********************************/
enum msgCodeEnum twGetDirectoryStructure(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	/* Inputs */
	twDataShapeEntry * dse = NULL;
	twDataShape * ds = NULL;
	/* Outputs */

	dse = s_twDataShapeEntry_Create("name","",TW_STRING);
    TW_LOG(TW_TRACE, "Executing twGetDirectoryStructure.");
	if (!dse) {
		TW_LOG(TW_ERROR, "twGetDirectoryStructure: Error creating output datashape entry");
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShapeEntry_AddAspect(dse,"isPrimaryKey", twPrimitive_CreateFromBoolean(TRUE));
	ds = s_twDataShape_Create(dse);
	if (!ds) {
		TW_LOG(TW_ERROR, "twGetDirectoryStructure: Error creating output datashape");
		twDataShapeEntry_Delete(dse);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("path","",TW_STRING));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("parentPath","",TW_STRING));
    /* Create the output infotable */
	*content = s_twInfoTable_Create(ds);
	if (!*content) {
		TW_LOG(TW_ERROR, "twGetDirectoryStructure: Error creating output infotable");
		twDataShape_Delete(ds);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	/* Perform the function */
	{
		twList * list = NULL;
		ListEntry * le = NULL;
		list = twFileManager_ListVirtualDirs(entityName);
		if ( !list) {
			TW_LOG(TW_ERROR, "twGetDirectoryStructure: Invalid input param(s) of could not get list");
			twInfoTable_Delete(*content);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		le = twList_Next(list, NULL);
		while (le && le->value) {
			twFile * f = (twFile *)le->value;

			/* Each virtual directory must be added to the result as a top level node */
			/* in the directory tree.  The parent of each of these nodes is /.        */
			twInfoTableRow * row = NULL;
			char * virtualPath = NULL;

			row = s_twInfoTableRow_Create(twPrimitive_CreateFromString(f->name, TRUE));

			if (!row) {
				TW_LOG(TW_ERROR, "twGetDirectoryStructure: Error allocating infotable row");
				twInfoTable_Delete(*content);
				*content = NULL;
				return TWX_INTERNAL_SERVER_ERROR;
			}

			virtualPath = (char *)TW_CALLOC(1 + strnlen(f->virtualPath, MAX_PATH_LEN) + 1, 1);
			strcat(virtualPath, "/");
			strcat(virtualPath, f->virtualPath);

			twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(virtualPath, TRUE));
			twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString("/", TRUE));

			if (s_twInfoTable_AddRow(*content, row)) {
				TW_LOG(TW_ERROR, "twGetDirectoryStructure: Error adding infotable row");
				twInfoTableRow_Delete(row);
				TW_FREE(virtualPath);
				return TWX_INTERNAL_SERVER_ERROR;
			}

			/* Now add the virtual dirs subdirectories. This call is recursive. */
			if (s_listDirsInInfoTable((char *)entityName, virtualPath, *content)) {
				TW_LOG(TW_ERROR, "twGetDirectoryStructure: Error copying directories into infoTable");
				twInfoTable_Delete(*content);
				*content = NULL;
				TW_FREE(virtualPath);
				return TWX_INTERNAL_SERVER_ERROR;
			}

			TW_FREE(virtualPath);
			le = twList_Next(list, le);
		}
		twList_Delete(list);
	}
	return TWX_SUCCESS;
}

enum msgCodeEnum twBrowseDirectory(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	/* Inputs */
	char * path = NULL;
	char * realPath = NULL;
	twDataShapeEntry * dse = NULL;
	twDataShape * ds = NULL;

    TW_LOG(TW_TRACE, "executing twBrowseDirectory.");

	if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL, FALSE, NULL)) {
		/* Handle the case of "/" */
		if (!path || strnlen(path, MAX_PATH_LEN) > 1) {
			TW_LOG(TW_ERROR, "twBrowseDirectory: Invalid input param(s)");
			return TWX_BAD_REQUEST;
		}
	}

	/* Outputs */
	dse = twDataShapeEntry_Create("name","",TW_STRING);
	if (!dse) {
		TW_LOG(TW_ERROR, "twBrowseDirectory: Error creating output datashape entry");
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShapeEntry_AddAspect(dse,"isPrimaryKey", twPrimitive_CreateFromBoolean(TRUE));
	ds = twDataShape_Create(dse);
	if (!ds) {
		TW_LOG(TW_ERROR, "twBrowseDirectory: Error creating output datashape");
		twDataShapeEntry_Delete(dse);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("path","",TW_STRING));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("size","",TW_NUMBER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("fileType","",TW_STRING));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("lastModifiedDate","",TW_DATETIME));
    /* Create the output infotable */
	*content = twInfoTable_Create(ds);
	if (!*content) {
		TW_LOG(TW_ERROR, "twBrowseDirectory: Error creating output infotable");
		twDataShape_Delete(ds);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	/* Perform the function */
	{
		/* Handle the case of "/" */
		if (strnlen(path, MAX_PATH_LEN) == 1 && (path[0] == '/' || path[0] == '\\')) {
			int res = 0;
			res = listVirtualDirsInInfoTable(entityName, *content, TRUE);
			if (res) {
				twInfoTable_Delete(*content);
				*content = NULL;
				cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
				return TWX_INTERNAL_SERVER_ERROR;
			}
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
			return TWX_SUCCESS;
		} else {
			/* Need to iterate through the directory */
			twInfoTableRow * row = 0;
			TW_DIR hnd = 0;
			char fileType[8];
			twFile * tmp = twFile_Create();
			if (!tmp) {
				TW_LOG(TW_ERROR, "listDirsInInfoTable: error allocating twFile struct");
				twInfoTable_Delete(*content);
				*content = NULL;
				cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
				return TW_ERROR_ALLOCATING_MEMORY;
			}
			hnd = twDirectory_IterateEntries(realPath, hnd, &tmp->name, &tmp->size,
											   &tmp->lastModified, &tmp->isDir, &tmp->readOnly);
			while (tmp && hnd) {
				if (!strcmp(tmp->name,".") || !strcmp(tmp->name,"..")) {
					twFile_Delete(tmp);
					tmp = twFile_Create();
					if (!tmp) {
						TW_LOG(TW_ERROR, "listDirsInInfoTable: error allocating twFile struct");
						twInfoTable_Delete(*content);
						*content = NULL;
						cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
						return TW_ERROR_ALLOCATING_MEMORY;
					}
					hnd = twDirectory_IterateEntries(realPath, hnd, &tmp->name, &tmp->size,
								&tmp->lastModified, &tmp->isDir, &tmp->readOnly);
					continue;
				}
				strcpy(fileType,"F");
				/* Fill in the info table row for this entry */
				TW_LOG(TW_TRACE,"twBrowseDirectory: Adding dir %s%c%s to list.", tmp->realPath, TW_FILE_DELIM, tmp->name);
				row = twInfoTableRow_Create(twPrimitive_CreateFromString(tmp->name, TRUE));
				if (!row) {
					TW_LOG(TW_ERROR, "twBrowseDirectory: Error allocating infotable row");
					twFile_Delete(tmp);
					twInfoTable_Delete(*content);
					*content = NULL;
					cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
					return TWX_INTERNAL_SERVER_ERROR;
				}
				twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(path, TRUE));
				twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber((double)tmp->size));
				if (tmp->isDir) fileType[0] = 'D';
				if (tmp->readOnly) strcat(fileType, "+RO");
				twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(fileType, TRUE));
				twInfoTableRow_AddEntry(row, twPrimitive_CreateFromDatetime(tmp->lastModified));
				if (twInfoTable_AddRow(*content, row)) {
					TW_LOG(TW_ERROR, "twBrowseDirectory: Error adding infotable row");
					twInfoTable_Delete(*content);
					*content = NULL;
					cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
					twFile_Delete(tmp);
					return TWX_INTERNAL_SERVER_ERROR;
				}
				twFile_Delete(tmp);
				tmp = twFile_Create();
				if (!tmp) {
					TW_LOG(TW_ERROR, "listDirsInInfoTable: error allocating twFile struct");
					twInfoTable_Delete(*content);
					*content = NULL;
					cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
					return TW_ERROR_ALLOCATING_MEMORY;
				}
				hnd = twDirectory_IterateEntries(realPath, hnd, &tmp->name, &tmp->size,
					&tmp->lastModified, &tmp->isDir, &tmp->readOnly);
			}
			if (tmp) twFile_Delete(tmp);
		}
	}
	cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
	return TWX_SUCCESS;
}

enum msgCodeEnum twDeleteFile(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	int res;
	/* Inputs */
	char * path = NULL;
	char * realPath = NULL;

	/* variables to check if the file is in use or timed out */
	twFile * f = NULL;
	char isTimedOut = FALSE;

    TW_LOG(TW_TRACE, "Executing twDeleteFile.");

    if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twDeleteFile: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	/* Outputs */
	*content = NULL;
	/* Perform the function */
	if (!s_twDirectory_FileExists(realPath)) {
		TW_LOG(TW_WARN, "twDeleteFile: File %s doesn't exist.  Nothing to do", path);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		return TWX_SUCCESS;
	} else {
		/* Check if File is open */
		f = twFileManager_GetOpenFile(entityName, path, NULL, NULL, &isTimedOut);
	}
	if(!f || isTimedOut){
		TW_LOG(TW_DEBUG, "twDeleteFile: Deleting file %s", realPath);
		res = twDirectory_DeleteFile(realPath);
		if (res) {
			TW_LOG(TW_ERROR, "twDeleteFile: Error deleting file %s.  Error: %d",realPath, res);
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
			return TWX_INTERNAL_SERVER_ERROR;
		}
	} else {
		TW_LOG(TW_DEBUG, "twDeleteFile: File is currently in Use: %s", realPath);
	}
	cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
	return TWX_SUCCESS;
}

enum msgCodeEnum twGetFileInfo(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	int res = 0;
	twInfoTableRow * row = NULL;
	twDataShapeEntry * dse = NULL;
	twDataShape * ds = NULL;
	/* Inputs */
	char * path = NULL;
	char * realPath = NULL;
	char * adjustedPath = NULL;
	char * realAdjustedPath = NULL;

    TW_LOG(TW_TRACE, "Executing twGetFileInfo.");

    if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twGetFileInfo: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	/* Outputs */
	dse = twDataShapeEntry_Create("name","",TW_STRING);
	if (!dse) {
		TW_LOG(TW_ERROR, "twGetFileInfo: Error creating output datashape entry");
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShapeEntry_AddAspect(dse,"isPrimaryKey", twPrimitive_CreateFromBoolean(TRUE));
	ds = twDataShape_Create(dse);
	if (!ds) {
		TW_LOG(TW_ERROR, "twGetFileInfo: Error creating output datashape");
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		twDataShapeEntry_Delete(dse);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("path","",TW_STRING));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("size","",TW_NUMBER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("fileType","",TW_STRING));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("lastModifiedDate","",TW_DATETIME));
    /* Create the output infotable */
	*content = twInfoTable_Create(ds);
	if (!*content) {
		TW_LOG(TW_ERROR, "twGetFileInfo: Error creating output infotable");
		twDataShape_Delete(ds);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	/* Perform the function */
	if (!twDirectory_FileExists(realPath)) {
		TW_LOG(TW_WARN, "twGetFileInfo: File %s doesn't exist.", path);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		twInfoTable_Delete(*content);
		*content = NULL;
		return TWX_NOT_FOUND;
	}
	{
	/* New scope for these variable declarations */
	uint64_t size;
	char isDirectory;
	char isReadOnly;
	DATETIME lastModified;
	char fileType[8];
	char * tmp;
	memset(fileType, 0, 8);
	TW_LOG(TW_DEBUG, "twGetFileInfo: Getting info for file %s", path);
	res = twDirectory_GetFileInfo(realAdjustedPath, &size, &lastModified, &isDirectory, &isReadOnly);
	if (res) {
		TW_LOG(TW_ERROR, "twGetFileInfo: Error getting info for file %s.  Error: %d",realAdjustedPath, res);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		twInfoTable_Delete(*content);
		*content = NULL;
		return TWX_INTERNAL_SERVER_ERROR;
	}

	/* Get the file name without leading '/' or '\' */
	tmp = strrchr(path, '/');
	if (!tmp) tmp = strrchr(path, '\\');
	if (tmp) tmp++;

	/* Fill in the info table */
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(tmp, TRUE));
	if (!row) {
		TW_LOG(TW_ERROR, "twGetFileInfo: Error allocating infotable row");
		twInfoTable_Delete(*content);
		*content = NULL;
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(path, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber((double)size));
	fileType[0] = 'F';
	if (isDirectory) fileType[0] = 'D';
	if (isReadOnly) strcat(fileType, "+RO");
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(fileType, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromDatetime(lastModified));
	twInfoTable_AddRow(*content, row);
	}
	cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
	return TWX_SUCCESS;
}

enum msgCodeEnum twListFiles(const char * entityName, twInfoTable * params, twInfoTable ** content) {
    TW_LOG(TW_TRACE, "Executing twListFiles.");
	return twListEntities(entityName, params, content, TRUE);
}

enum msgCodeEnum twMoveFile(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	int res;
	/* Inputs */
	char * sourcePath = NULL;
	char * targetPath = NULL;
	char overwrite = 0;
	char * realSourcePath = NULL;
	char * realTargetPath = NULL;
	char * adjustedPath = NULL;
	twFile * f = NULL;

    TW_LOG(TW_TRACE, "Executing twMoveFile.");

	if (getPaths(params, entityName, NULL, NULL, &sourcePath, &realSourcePath, &targetPath, &realTargetPath, NULL, NULL, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twMoveFile: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	twInfoTable_GetBoolean(params, "overwrite", 0, &overwrite);
	if (!overwrite && twDirectory_FileExists(realTargetPath)) {
		TW_LOG(TW_ERROR, "twMoveFile: Target file %s exists and 'overwrite' is FALSE", targetPath);
		cleanUpPaths(NULL, NULL, &sourcePath, &realSourcePath, &targetPath, &realTargetPath, NULL, NULL);
		return TWX_PRECONDITION_FAILED;
	}
	/* We likely have the source file open, so we need to close it before moving */
	if (isPre5_1Server) {
		adjustedPath = adjustPath(entityName, sourcePath, FALSE);
		TW_FREE(sourcePath);
		TW_FREE(realSourcePath);
		sourcePath = adjustedPath;
		realSourcePath = twFileManager_GetRealPath(entityName, sourcePath, NULL);
	}
	f = twFileManager_GetOpenFile(entityName, sourcePath, NULL, NULL, NULL);
	if (f) {
		if (f->handle) {
			TW_FCLOSE(f->handle);
			f->handle = 0;
		}
		twFile_SetIsInUse(f, FALSE);
	}
	res = twDirectory_MoveFile(realSourcePath,realTargetPath);
	TW_LOG(TW_DEBUG, "twMoveFile: Moving file %s to %s", sourcePath, targetPath);
	if (res) {
		TW_LOG(TW_ERROR, "twMoveFile: Error moving file %s to %s. Error: %d",sourcePath, targetPath, res);
		cleanUpPaths(NULL, NULL, &sourcePath, &realSourcePath, &targetPath, &realTargetPath, NULL, NULL);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	cleanUpPaths(NULL, NULL, &sourcePath, &realSourcePath, &targetPath, &realTargetPath, NULL, NULL);
	return TWX_SUCCESS;
}

enum msgCodeEnum twGetTransferInfo(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	/* Generic function across the board for all enitites */
	int res;
	twInfoTableRow * row = NULL;
	/* Inputs */

	/* Outputs */
	twDataShape * ds = twDataShape_Create(twDataShapeEntry_Create("blockSize","",TW_INTEGER));

    TW_LOG(TW_TRACE, "Executing twGetTransferInfo.");

	if (!ds) {
		TW_LOG(TW_ERROR, "GetTransferInfo: Error creating output datashape");
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("maxFileSize","",TW_NUMBER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("stagingDir","",TW_STRING));
    /* Create and fill in the output infotable */
	*content = twInfoTable_Create(ds);
	if (!*content) {
		TW_LOG(TW_ERROR, "GetTransferInfo: Error creating output infotable");
		twDataShape_Delete(ds);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	row = twInfoTableRow_Create(twPrimitive_CreateFromInteger(twcfg.file_xfer_block_size));
	if (!row) {
		TW_LOG(TW_ERROR, "GetTransferInfo: Error creating output infotable row");
		twInfoTable_Delete(*content);
		*content = NULL;
		return TWX_INTERNAL_SERVER_ERROR;
	}
	res = twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber((double)twcfg.file_xfer_max_file_size));
	res |= twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(TW_VIRTUAL_STAGING_DIR, TRUE));
	res |= twInfoTable_AddRow(*content, row);
	if (res) {
		TW_LOG(TW_ERROR, "GetTransferInfo: Error creating populating output infotable row");
		twInfoTable_Delete(*content);
		*content = NULL;
		return TWX_INTERNAL_SERVER_ERROR;
	}
	return TWX_SUCCESS;
}

enum msgCodeEnum twGetFileChecksum(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	int res = TW_OK;

	char * path = NULL;
	char * realPath = NULL;
	char * adjustedPath = NULL;
	char * realAdjustedPath = NULL;

    TW_LOG(TW_TRACE, "Executing twGetFileChecksum.");

	if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twGetFileChecksum: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}

	res =  twFileManager_GetFileChecksum(entityName, content, realPath, adjustedPath);

	cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);

	return res;
}

enum msgCodeEnum twReadFromBinaryFile(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	twDataShape * ds = NULL;
	/* Inputs */
	char * path = NULL;
	char * realPath = NULL;
	char isTimedOut = 0x00;
	double offset = 0;
	int32_t count = 0;

    TW_LOG(TW_TRACE, "Executing twReadFromBinaryFile.");

	twInfoTable_GetNumber(params, "offset", 0, &offset);
	twInfoTable_GetInteger(params, "count", 0, &count);
	if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twReadFromBinaryFile: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	/* Outputs */
	ds = twDataShape_Create(twDataShapeEntry_Create("count","",TW_INTEGER));
	if (!ds) {
		TW_LOG(TW_ERROR, "twReadFromBinaryFile: Error creating output datashape");
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("data","",TW_BLOB));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("eof","",TW_BOOLEAN));
    /* Create the output infotable */
	*content = twInfoTable_Create(ds);
	if (!*content) {
		TW_LOG(TW_ERROR, "twReadFromBinaryFile: Error creating output infotable");
		twDataShape_Delete(ds);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	/* Execute the service */
	{
		twFile * f = NULL;
		char * buffer = NULL;
		int res = 0;
		twInfoTableRow * row = NULL;
		buffer = (char *)TW_CALLOC(count, 1);
		if (!buffer) {
			TW_LOG(TW_ERROR, "twReadFromBinaryFile: Error allocating buffer");
			twInfoTable_Delete(*content);
			*content = NULL;
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
			twInfoTableRow_Delete(row);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		/* If we are not open yet, open the file */
		f = twFileManager_GetOpenFile(entityName, path, NULL, NULL, &isTimedOut);
		/* if the file transfer timed out, return with an error*/
		if (isTimedOut) {
			TW_LOG(TW_ERROR, "twReadFromBinaryFile: Transfer timed out for %s", path);
			twInfoTable_Delete(*content);
			TW_FREE(buffer);
			*content = NULL;
			twInfoTableRow_Delete(row);
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		if (!f || !f->handle) {
				f = twFileManager_OpenFile(entityName, path, NULL, "rb");
				if (!f) {
					TW_LOG(TW_ERROR, "twReadFromBinaryFile: Error opening %s for reading", path);
					twInfoTable_Delete(*content);
					TW_FREE(buffer);
					*content = NULL;
					twInfoTableRow_Delete(row);
					cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
					return TWX_INTERNAL_SERVER_ERROR;
				}
				if (twInfoTable_GetString(params, "tid", 0, &f->tid)) {
					twFileManager_CloseFile(f);
					cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
					return TWX_INTERNAL_SERVER_ERROR;
				}
				f->openForRead = TRUE;
		}
		res = TW_FSEEK(f->handle, (uint64_t)offset, SEEK_SET);
		if (res) {
			TW_LOG(TW_ERROR, "twReadFromBinaryFile: Error seeking in file %s to %f.  Error: %d", path, offset, twDirectory_GetLastError());
			twInfoTable_Delete(*content);
			*content = NULL;
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
			TW_FREE(buffer);
			twFile_SetIsInUse(f, 0x00);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		res = TW_FREAD(buffer, 1, count, f->handle);
		if (res > 0) {
			char eof = FALSE;
			row = twInfoTableRow_Create(twPrimitive_CreateFromNumber(res));
			if (!row) {
				TW_LOG(TW_ERROR, "twReadFromBinaryFile: Error creating output infotable row");
				twInfoTable_Delete(*content);
				*content = NULL;
				cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
				TW_FREE(buffer);
				twFile_SetIsInUse(f, 0x00);
				return TWX_INTERNAL_SERVER_ERROR;
			}
			twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBlob(buffer, res, FALSE, FALSE));
			/* Check for EOF, including corner case */
			if (res < count && (offset + res == f->size)) eof = TRUE;
			twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(eof));
			twInfoTable_AddRow(*content, row);
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
			twFile_SetIsInUse(f, 0x00);
			return TWX_SUCCESS;
		} else {
			TW_LOG(TW_ERROR, "twReadFromBinaryFile: Error reading from file %s", path);
			twInfoTable_Delete(*content);
			*content = NULL;
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
			twInfoTableRow_Delete(row);
			TW_FREE(buffer);
			twFile_SetIsInUse(f, 0x00);
			return TWX_INTERNAL_SERVER_ERROR;
		}
	}
}

enum msgCodeEnum twWriteToBinaryFile(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	/* Inputs */
	char * path = NULL;
	char * realPath = NULL;
	char * adjustedPath = NULL;
	char * realAdjustedPath = NULL;
	char isTimedOut = FALSE;
	double offset = 0;
	char * data = 0;
	int32_t count = 0;

    TW_LOG(TW_TRACE, "Executing twWriteToBinaryFile.");

	if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twWriteToBinaryFile: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	twInfoTable_GetNumber(params, "offset", 0, &offset);
	twInfoTable_GetBlob(params, "data", 0, &data, &count);
	if (!data) {
		TW_LOG(TW_ERROR, "twWriteToBinaryFile: Invalid input param(s)");
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		return TWX_BAD_REQUEST;
	}
	/* Outputs */
	*content = NULL;
	/* Execute the service */
	{
		twFile * f = NULL;
		int res = 0;
		f = twFileManager_GetOpenFile(entityName, adjustedPath, NULL, NULL, &isTimedOut);
		if (isTimedOut) {
			TW_LOG(TW_ERROR, "twWriteToBinaryFile: Error opening %s for writing, file timed out", adjustedPath);
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		if (!f || !f->handle) {
			TW_LOG(TW_WARN, "twWriteToBinaryFile: File %s is not open.  Opening now...", adjustedPath);
			f = twFileManager_OpenFile(entityName, adjustedPath, NULL, "a+b");
			if (!f) {
				TW_LOG(TW_ERROR, "twWriteToBinaryFile: Error opening %s for writing", adjustedPath);
				cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
				return TWX_INTERNAL_SERVER_ERROR;
			}
			f->openForRead = FALSE;
		}
		res = TW_FSEEK(f->handle, (uint64_t)offset, SEEK_SET);
		if (res) {
			TW_LOG(TW_ERROR, "twWriteToBinaryFile: Error seeking in file %s to %f.  Error: %d", adjustedPath, offset, twDirectory_GetLastError());
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
			twFile_SetIsInUse(f, FALSE);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		res = TW_FWRITE(data, 1, count, f->handle);
		twFile_SetIsInUse(f, FALSE);
		if (res > 0) {
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
			return TWX_SUCCESS;
		} else {
			TW_LOG(TW_ERROR, "twWriteToBinaryFile: Error writing to file %s", path);
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
			return TWX_INTERNAL_SERVER_ERROR;
		}
	}
}

enum msgCodeEnum twCreateBinaryFile(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	int res;
	/* Inputs */
	char * path = NULL;
	char * realPath = NULL;
	char * adjustedPath = NULL;
	char * realAdjustedPath = NULL;
	char overwrite = FALSE;
	char * data = NULL;
	int32_t count = 0;
	twFile * f = NULL;
	char wasAlreadyOpen = TRUE;

    TW_LOG(TW_TRACE, "Executing twCreateBinaryFile.");

	twInfoTable_GetBoolean(params, "overwrite", 0, &overwrite);
	if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twCreateBinaryFile: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	/* Outputs */
	*content = NULL;
	/* Perform the function */
	if (!overwrite && twDirectory_FileExists(realPath)) {
		TW_LOG(TW_WARN, "CreateBinaryFile: File %s exists and 'overwrite' is FALSE.  Nothing to do", path);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		return TWX_SUCCESS;
	}
	/* Check to see if this is a write, which will have data */
	s_twInfoTable_GetBlob(params, "data", 0, &data, &count);
	if (data) {
		/* Check to see if this is open already */
		f = twFileManager_GetOpenFile(entityName, adjustedPath, NULL, NULL, NULL);
		if (!f || !f->handle) {
			/* This is a write so we don't want to just create, we want to open */
			f = twFileManager_OpenFile(entityName, path, NULL, "a+b");
			wasAlreadyOpen = FALSE;
		}
		if (!f) {
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
			return TWX_INTERNAL_SERVER_ERROR;
		}
		f->openForRead = FALSE;
		res = twWriteToBinaryFile(entityName, params, content);
		/* If we opened the file here we want to leave things the way we found them */
		if (!wasAlreadyOpen) {
			twFileManager_CloseFile(f);
		} else {
			twFile_SetIsInUse(f, FALSE);
		}
	} else {
		/* This is just a create */
		TW_LOG(TW_DEBUG, "CreateBinaryFile: Creating file %s", path);
		res = twDirectory_CreateFile(realPath);
		if (res) {
			TW_LOG(TW_ERROR, "CreateBinaryFile: Error creating file %s. Error: %d", path, res);
			cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
			return TWX_INTERNAL_SERVER_ERROR;
		}
	}
	cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
	return TWX_SUCCESS;
}

enum msgCodeEnum twListDirectories(const char * entityName, twInfoTable * params, twInfoTable ** content) {
    TW_LOG(TW_TRACE, "Executing twListDirectories.");
	return twListEntities(entityName, params, content, FALSE);
}

enum msgCodeEnum twMakeDirectory(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	int res;
	/* Inputs */
	char * path = NULL;
	char * realPath = NULL;
	if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL, FALSE, NULL)) {
		TW_LOG(TW_ERROR, "twMakeDirectory: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	/* Outputs */
	*content = NULL;
	/* Perform the function */
	if (s_twDirectory_FileExists(realPath)) {
		TW_LOG(TW_WARN, "twMakeDirectory: Directory %s exists.  Nothing to do", path);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		return TWX_SUCCESS;
	}
	TW_LOG(TW_DEBUG, "twMakeDirectory: Creating directory %s", path);
	res = s_twDirectory_CreateDirectory(realPath);
	if (res) {
		TW_LOG(TW_ERROR, "twMakeDirectory: Error creating file %s.  Error: %d", path, res);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, NULL, NULL);
	return TWX_SUCCESS;
}

enum msgCodeEnum twStartFileTransfer(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	/* Inputs */
	char * tid = NULL;
	char * path = NULL;
	char * adjustedPath = NULL;
	char * realAdjustedPath = NULL;
	char * realPath = NULL;
	twFile * f = NULL;
	char * mode = NULL;

    TW_LOG(TW_TRACE, "Executing twStartFileTransfer.");

	if (getPaths(params, entityName, &path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath, TRUE, NULL)) {
		TW_LOG(TW_ERROR, "twStartFileTransfer: Invalid input param(s)");
		return TWX_BAD_REQUEST;
	}
	/* Get the mode */
	if (twInfoTable_GetString(params, "mode", 0, &mode)) {
		TW_LOG(TW_ERROR, "twStartFileTransfer: Error opening %s", adjustedPath);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		return TWX_BAD_REQUEST;
	}
	/*
		StartFileTransfer does not prepend the FILE_XFER_STAGING_DIR, so we need to do that if
		this is a write.  There is also an inconsistency in older servers that we need to address
		here as older servers use the standard delimiter and adds .part onto the path in the
		StartFileTransfer request and use that path for Create/Read/Write requests.  Newer servers
		do not tack on the .part file in the StartFileTransfer request but do in Create/Read/Write
		requests. It also prepends the ThingName to the path. In addition those Create/Read/Write
		requests use '.' as the delimiter to flatten the staging directory.

		                              Old Server                           New Server
        StartFileTransfer      <vdir>/path/to/file.part                 <vdir>/path/to/file
		GetFileInfo         <staging>/<vdir>.path/to/file.part  <staging>/ThingName.<vdir>.path.to.file.part
		CreateBinaryFile    <staging>/<vdir>/path/to/file.part  <staging>/ThingName.<vdir>.path.to.file.part
		WriteBinaryFile     <staging>/<vdir>/path/to/file.part  <staging>/ThingName.<vdir>.path.to.file.part
		ReadBinaryFile            <vdir>/path/to/file                   <vdir>/path/to/file
	*/
	if (!strcmp(mode,"read")) {
		TW_FREE(adjustedPath);
		adjustedPath = duplicateString(path);
	}
	/* Check to see if we are already transferring this file */
	if (twInfoTable_GetString(params, "tid", 0, &tid)) {
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		TW_FREE(mode);
		return TWX_INTERNAL_SERVER_ERROR;
	}
	f = twFileManager_GetOpenFile(entityName, adjustedPath, NULL, tid, NULL);
	if (f) {
		TW_LOG(TW_ERROR, "twStartFileTransfer: Transfer to %s already in progress.", path);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		TW_FREE(mode);
		if(tid) TW_FREE(tid);
		twFile_SetIsInUse(f, FALSE);
		return TWX_PRECONDITION_FAILED;
	}
	TW_LOG(TW_AUDIT, "FILE TRANSFER STARTED.  File: %s, Mode: %s", realPath, mode ? mode : "unknown");
	f = twFileManager_OpenFile(entityName, adjustedPath, NULL, (strcmp(mode, "write") ? "rb" : "a+b"));
	if (!f) {
		TW_LOG(TW_ERROR, "twStartFileTransfer: Error opening %s", adjustedPath);
		cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
		if(tid) TW_FREE(tid);
		TW_FREE(mode);
		return TWX_PRECONDITION_FAILED;
	}
	f->tid = tid;
	f->openForRead = strcmp(mode, "write") ? TRUE : FALSE;
	twFile_SetIsInUse(f, FALSE);
	/* Outputs */
	*content = NULL;
	/* Perform the function */
	cleanUpPaths(&path, &realPath, NULL, NULL, NULL, NULL, &adjustedPath, &realAdjustedPath);
	TW_FREE(mode);
	return TWX_SUCCESS;
}

enum msgCodeEnum twFinishFileTransfer(
	const char * entityName,
	twInfoTable * params,
	twInfoTable ** content
) {
	/* Inputs */
	twPrimitive * job = NULL;
	twFileTransferInfo * ft = NULL;
	char * adjustedPath = NULL;
	char * path = NULL;
	enum msgCodeEnum res;

    TW_LOG(TW_TRACE, "Executing twFinishFileTransfer.");

	if (twInfoTable_GetPrimitive(params, "job", 0, &job)) {
		TW_LOG(TW_ERROR,"twFinishFileTransfer: Missing 'job' parameter");
		return TWX_BAD_REQUEST;
	}
	if (job->type != TW_INFOTABLE) {
		TW_LOG(TW_ERROR,"twFinishFileTransfer: 'job' parameter is not an infotable");
		return TWX_BAD_REQUEST;
	}
	ft = twFileTransferInfo_Create(job->val.infotable);
	if (!ft) {
		TW_LOG(TW_ERROR,"twFinishFileTransfer: Error getting file transfer info");
		return TWX_BAD_REQUEST;
	}

	*content = NULL;
	getPaths(
		job->val.infotable,
		ft->targetRepository,
		&path,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&adjustedPath,
		NULL,
		TRUE,
		"targetPath"
	);
	res = twFileManager_FinishFileTransfer(entityName, ft, path, adjustedPath);
	cleanUpPaths(&path, NULL, NULL, NULL, NULL, NULL, &adjustedPath, NULL);
    twFileTransferInfo_Delete(ft);

    return res;
}

char * fileXferServices[] = {
	"BrowseDirectory", "DeleteFile", "GetFileInfo", "ListFiles", "MoveFile",
	"GetFileListing", "GetTransferInfo", "GetFileChecksum", "CreateBinaryFile",
	"ReadFromBinaryFile", "WriteToBinaryFile", "ListDirectories",
	"MakeDirectory", "StartFileTransfer", "CancelFileTransfer", "CompleteFileTransfer",
	"GetDirectoryStructure", "SENTINEL"
};

enum msgCodeEnum fileTransferCallback(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content) {
	/* GetTransferInfo && GetDirectoryStructure have no aparams, all others do */
	if (!entityName || !serviceName || ((strcmp(serviceName,"GetTransferInfo") && !params) && (strcmp(serviceName,"GetDirectoryStructure") && !params))) {
		TW_LOG(TW_ERROR, "fileTransferCallback: missing entityName, serviceName, or input params");
		return TWX_BAD_REQUEST;
	}
	if (!content) {
		TW_LOG(TW_ERROR, "fileTransferCallback: missing content param");
		return TWX_INTERNAL_SERVER_ERROR;
	}

    TW_LOG(TW_TRACE, "executing fileTransferCallback.");

	if (!strcmp(serviceName, "BrowseDirectory")) {
		return twBrowseDirectory(entityName, params, content);
	} else 	if (!strcmp(serviceName, "DeleteFile")) {
		return twDeleteFile(entityName, params, content);
	} else 	if (!strcmp(serviceName, "GetFileInfo")) {
		return twGetFileInfo(entityName, params, content);
	} else 	if (!strcmp(serviceName, "ListFiles")) {
		return twListFiles(entityName, params, content);
	} else 	if (!strcmp(serviceName, "MoveFile")) {
		return twMoveFile(entityName, params, content);
	} else 	if (!strcmp(serviceName, "GetTransferInfo")) {
		return twGetTransferInfo(entityName, params, content);
	} else 	if (!strcmp(serviceName, "GetFileChecksum")) {
		return twGetFileChecksum(entityName, params, content);
	} else 	if (!strcmp(serviceName, "CreateBinaryFile")) {
		return twCreateBinaryFile(entityName, params, content);
	} else 	if (!strcmp(serviceName, "ReadFromBinaryFile")) {
		return twReadFromBinaryFile(entityName, params, content);
	} else 	if (!strcmp(serviceName, "WriteToBinaryFile")) {
		return twWriteToBinaryFile(entityName, params, content);
	} else 	if (!strcmp(serviceName, "ListDirectories")) {
		return twListDirectories(entityName, params, content);
	} else 	if (!strcmp(serviceName, "RenameFile")) {
		return twMoveFile(entityName, params, content);
	} else 	if (!strcmp(serviceName, "StartFileTransfer")) {
		return twStartFileTransfer(entityName, params, content);
	} else 	if (!strcmp(serviceName, "CancelFileTransfer")) {
		return twFinishFileTransfer(entityName, params, content);
	} else  if (!strcmp(serviceName, "CompleteFileTransfer")) {
		return twFinishFileTransfer(entityName, params, content);
	} else  if (!strcmp(serviceName, "GetDirectoryStructure")) {
		return twGetDirectoryStructure(entityName, params, content);
	} else {
		TW_LOG(TW_ERROR, "fileTransferCallback: Bad serviceName: %s", serviceName);
		return TWX_BAD_REQUEST;
	}
}
