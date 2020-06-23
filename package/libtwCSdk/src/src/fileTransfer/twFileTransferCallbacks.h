/*************************************
 * Copyright 2017, PTC, Inc.
 *************************************/

/**
 * \file twFileTransferCallbacks.h
 *
 * \brief ThingWorx file transfer callback function prototypes
*/

#ifndef FILE_TRANSFER_CALLBACKS_H
#define FILEMANAGER_H

#include "twOSPort.h"
#include "twDefinitions.h"
#include "twInfoTable.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************/
/*     Service Callbacks        */
/********************************/
/********
enum msgCodeEnum twBrowseDirectory(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twDeleteFile(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twGetFileInfo(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twListFiles(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twMoveFile(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twGetTransferInfo(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twGetFileChecksum(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twCreateBinaryFile(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twReadFromBinaryFile(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twWriteToBinaryFile(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twListDirectories(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twMakeDirectory(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twStartFileTransfer(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twFinishFileTransfer(const char * entityName, twInfoTable * params, twInfoTable ** content);
***********/

enum msgCodeEnum fileTransferCallback(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content) ;

#ifdef __cplusplus
}
#endif

#endif
