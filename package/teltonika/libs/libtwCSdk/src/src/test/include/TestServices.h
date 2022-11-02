#ifndef TW_C_SDK_TESTSERVICES_H
#define TW_C_SDK_TESTSERVICES_H

#include "twApi.h"

enum msgCodeEnum pushBoolean(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushDatetime(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushGroupName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushHTML(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushHyperlink(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushImage(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushImagelink(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushInfoTable(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushInteger(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushJson(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushLocation(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushMashupName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushMenuName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushNumber(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushString(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushQuery(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushText(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushThingName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushUserName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum pushXML(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum Get_ThingName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum Set_FileXferTimeout(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);
enum msgCodeEnum Do_Nothing(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);

#endif //TW_C_SDK_TESTSERVICES_H
