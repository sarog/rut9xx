#include "TestServices.h"
#include "TestUtilities.h"

static enum msgCodeEnum
handlePush(const char *entityName, const char *serviceName, const char *propertyName, twInfoTable *params,
           twInfoTable **content, void *userdata) {
	twPrimitive *value = 0;
	twPrimitive *send_value = 0;
	twInfoTable *t = NULL;

	TW_LOG(TW_DEBUG, "Push service %s called for property %s", serviceName, propertyName);
	value = 0;
	twInfoTable_GetPrimitive(params, "value", 0, &value);
	if (!params || !content) {
		TW_LOG(TW_ERROR, "push - NULL params or content pointer");
		return TWX_BAD_REQUEST;
	}
	send_value = twPrimitive_FullCopy(value);
	twApi_WriteProperty(TW_THING, (char *) entityName, (char *) propertyName, send_value, -1, FALSE);
	twPrimitive_Delete(send_value);

	t = twInfoTable_CreateFromPrimitive("result", twPrimitive_FullCopy(value));
	*content = t;

	return TWX_SUCCESS;
}

enum msgCodeEnum
pushBoolean(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
            void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Boolean", params, content, userdata);
}

enum msgCodeEnum
pushDatetime(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
             void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Datetime", params, content, userdata);
}

enum msgCodeEnum
pushGroupName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
              void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_GroupName", params, content, userdata);
}

enum msgCodeEnum
pushHTML(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_HTML", params, content, userdata);
}

enum msgCodeEnum
pushHyperlink(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
              void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Hyperlink", params, content, userdata);
}

enum msgCodeEnum
pushImage(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Image", params, content, userdata);
}

enum msgCodeEnum
pushImagelink(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
              void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Imagelink", params, content, userdata);
}

enum msgCodeEnum
pushInfoTable(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
              void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_InfoTable", params, content, userdata);
}

enum msgCodeEnum
pushInteger(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
            void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Integer", params, content, userdata);
}

enum msgCodeEnum
pushJson(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_JSON", params, content, userdata);
}

enum msgCodeEnum
pushLocation(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
             void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Location", params, content, userdata);
}

enum msgCodeEnum
pushMashupName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
               void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_MashupName", params, content, userdata);
}

enum msgCodeEnum
pushMenuName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
             void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_MenuName", params, content, userdata);
}

enum msgCodeEnum pushNumber(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
                            void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Number", params, content, userdata);
}

enum msgCodeEnum pushString(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
                            void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_String", params, content, userdata);
}

enum msgCodeEnum
pushQuery(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Query", params, content, userdata);
}

enum msgCodeEnum
pushText(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_Text", params, content, userdata);
}

enum msgCodeEnum
pushThingName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
              void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_ThingName", params, content, userdata);
}

enum msgCodeEnum
pushUserName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
             void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_UserName", params, content, userdata);
}

enum msgCodeEnum
pushXML(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata) {
	return handlePush(entityName, serviceName, "AlwaysPush_XML", params, content, userdata);
}

enum msgCodeEnum
Get_ThingName(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
              void *userdata) {
	twPrimitive *result_value = NULL;
	int result_test = 1;
	char force_connect = 0x01;
	int timeout_tmp = 10000;
	char prop_tmp[5] = "name";
	char *name_tmp = NULL;
	prop_tmp[4] = 0x00;
	if (entityName) {
		name_tmp = (char *) TW_MALLOC(strlen(entityName) + 1);
	}
	if (name_tmp) {
		strncpy(name_tmp, entityName, strlen(entityName) + 1);
		name_tmp[strlen(entityName)] = 0x00;
	} else {
		return TWX_BAD_REQUEST;
	}
	result_value = twPrimitive_CreateFromString("", 0x00);
	result_test = twApi_ReadProperty(TW_THING, name_tmp, prop_tmp, &result_value, timeout_tmp, force_connect);
	TW_FREE(name_tmp);
	if (result_test) {
		return TWX_BAD_REQUEST;
	} else {
		if ((*content = twInfoTable_CreateFromPrimitive("result", result_value))) {
			return TWX_SUCCESS;
		}
		return TWX_BAD_REQUEST;
	}
}

enum msgCodeEnum
Set_FileXferTimeout(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
                    void *userdata) {
	int32_t a;
	TW_LOG(TW_TRACE, "Set_FileXferTimeout - Function called");
	if (!params || !content) {
		TW_LOG(TW_ERROR, "Set_FileXferTimeout - NULL params or content pointer");
		return TWX_BAD_REQUEST;
	}

	twInfoTable_GetInteger(params, "value", 0, &a);
	/* there is currently no built in method to check active transfers, but we should probably do that in the future */
	twcfg_pointer->file_xfer_timeout = a;
	return TWX_SUCCESS;
}

enum msgCodeEnum Do_Nothing(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
                            void *userdata) {
	return TWX_SUCCESS;
}