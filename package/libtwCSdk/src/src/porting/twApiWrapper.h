#include "twApi.h"
#include "twFileManager.h"
#include "stringUtils.h"
#ifdef _WIN32
#include <ole2.h> /* needed for CoTaskMemAlloc */
#endif
#ifndef TW_WRAPPER_EXPORTS_H
#define TW_WRAPPER_EXPORTS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct twWPropertyRegInfo
{
	char *propertyName;
	enum BaseType propertyType;
	char *propertyDescription;
	char *propertyPushType;
	double propertyPushThreshold;
} twWPropertyRegInfo;

typedef struct twWServiceRegInfo
{
	char *serviceName;
	char *serviceDescription;
	unsigned char *inputDataShapeBytesPtr;
	int32_t inputDataShapeLength;
	enum BaseType outputType;
	unsigned char *outputDataShapeBytesPtr;
	int32_t outputDataShapeLength;
} twWServiceRegInfo;

typedef struct twWRegInfoList
{
	int length; // number of elements
	void *listPtr; // array of twWPropertyRegInfo | twWServiceRegInfo
} twWRegInfoList;

typedef struct twWDataBlock
{
	char isPrimitive;
	int32_t length;
	unsigned char *bytesPtr;
} twWDataBlock;

typedef int(*processPropertyReadRequest_cb)(const char *entityName, const char *propertyName, twWDataBlock **dataBlock, void *userdata);
typedef int(*processPropertyWriteRequest_cb)(const char *entityName, const char *propertyName, twWDataBlock *dataBlock, void *userdata);
typedef int(*processServiceRequest_cb)(const char *entityName, const char *serviceName, twWDataBlock *paramsBlock, twWDataBlock **contentBlock, void *userdata);
typedef void(*fileTransfer_cb)(char fileRcvd, char *sourceRepository, char *sourcePath, char *sourceFile, char *sourceChecksum, 
								char *targetRepository, char *targetPath, char *targetFile, char *targetChecksum, DATETIME startTime, 
								DATETIME endTime, int32_t duration, char *state, char isComplete, double size, char *transferId, 
								char *user, char *message, void *userdata);


//**********  P R I V A T E S  *********//

int twW_InitializeThreadPool();
void twW_DestroyThreadPool();

twPrimitive * twWDataBlock_ToPrimitive(twWDataBlock *block);
twInfoTable * twWDataBlock_ToInfoTable(twWDataBlock *block);
int  twWDataBlock_FromPrimitive(twPrimitive *p, twWDataBlock **outBlock);
int twWDataBlock_FromInfoTable(twInfoTable *it, twWDataBlock **outBlock);
int twWDataBlock_FromStruct(void *structPtr, char isPrimitive, twWDataBlock **outBlock);
void twWDataBlock_Free(twWDataBlock *block);

twDataShape * twW_BytesToDataShape(unsigned char *bytesPtr, int32_t length);

enum msgCodeEnum twW_PropertyCallbackDispatcher(const char *entityName, const char *propertyName, twInfoTable **value, char isWrite, void *userdata);
enum msgCodeEnum twW_ServiceCallbackDispatcher(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
void twW_FileTransferCallbackDispatcher(char fileRcvd, twFileTransferInfo *info, void *userdata);

#ifdef __cplusplus
}
#endif

#endif