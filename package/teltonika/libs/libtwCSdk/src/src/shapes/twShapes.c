/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

#include <stdarg.h>
#include "twBaseTypes.h"
#include "twShapes.h"
#include "twConstants.h"
#include "twPrimitiveUtils.h"
#include "twPath.h"

#ifdef __APPLE__
#include <sys/syslimits.h>
#else
#include <limits.h>
#endif

/* Support dlopen on unix systems */
#ifndef WIN32
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#endif

#define TW_LAST_ARGUMENT NULL
extern twApi * tw_api;

/* Private api access */
void * findCallback(enum entityTypeEnum entityType, char *entityName,
					enum characteristicEnum characteristicType, char *characteristicName, void **userdata);

typedef int (*PluginInitFunc)();

static twList * tw_thingname_template_list = NULL;
static twList * tw_thingname_shape_list = NULL;

static twMap* tw_shapes_map = NULL;
static twMap* tw_templates_map = NULL;

void twConstructEmptyEdgeThingTemplate(char *thingName, char *namespace) {}

const char *twUsedPropertiesServicesParse(void *item) {
    return duplicateString(item);
}

void twUserPropertiesServicesDelete(char *entry) {
	TW_FREE(entry);
}

twDict* twGetUsedServicesDict(void){

    if(NULL == tw_api->tw_used_service_names){
        tw_api->tw_used_service_names = twDict_Create(twUserPropertiesServicesDelete, twUsedPropertiesServicesParse);
    }
    return tw_api->tw_used_service_names;
}

twDict* twGetUsedPropertiesDict(void){

    if(NULL == tw_api->tw_used_property_names){
        tw_api->tw_used_property_names = twDict_Create(twUserPropertiesServicesDelete, twUsedPropertiesServicesParse);
    }
    return tw_api->tw_used_property_names;
}

/* This service will load a shape library. */
enum msgCodeEnum twLoadShapeLibraryService(const char *entityName, const char *serviceName, twInfoTable *params,
                                           twInfoTable **content, void *userdata){
    char* libraryName;
    twInfoTable_GetString(params, "libraryName", 0, &libraryName);
    if(NULL == twExt_LoadExtensionLibrary(libraryName)){
        return TWX_NOT_FOUND;
    }

    return TWX_SUCCESS;
}

/* This service will add a shape to an existing thing. */
enum msgCodeEnum twAddEdgeThingShapeService(const char *entityName, const char *serviceName, twInfoTable *params,
                                            twInfoTable **content, void *userdata){
    twPrimitive* namespacePrimitive;
    char* shapeName,*namespace;
    twInfoTable_GetString(params, "shapeName", 0, &shapeName);
    twInfoTable_GetPrimitive(params, "namespace",0,&namespacePrimitive);
    if(NULL == namespacePrimitive || TW_NOTHING == namespacePrimitive->type){
        if(TW_OK != twExt_AddEdgeThingShape(entityName,shapeName,TW_NO_NAMESPACE)){
            return TWX_NOT_FOUND;
        }
    } else {
        twInfoTable_GetString(params, "namespace", 0, &namespace);
        if(TW_OK != twExt_AddEdgeThingShape(entityName,shapeName,namespace)){
            return TWX_NOT_FOUND;
        }
    }

    return TWX_SUCCESS;
}

char twVerifyUniquePropertyName(char* thingName,char *propertyName) {
    char* result;
    char* testPropertyName = duplicateString(thingName);
    twDict* propDict = twGetUsedPropertiesDict();
    concatenateStrings(&testPropertyName,"::");
    concatenateStrings((char**)&testPropertyName,propertyName);

    if (TW_MAP_MISSING == twDict_Find(propDict, testPropertyName, &result)) {
        twDict_Add(propDict, testPropertyName);
        return 1;
    }
	TW_FREE(testPropertyName);
    return 0;
}

char twVerifyUniqueServiceName(char* thingName,char *serviceName) {
    char* result;
    twDict* serviceDict = twGetUsedServicesDict();
    char* testServiceName = duplicateString(thingName);
    concatenateStrings(&testServiceName,"::");
    concatenateStrings(&testServiceName,serviceName);

    if (TW_MAP_MISSING == twDict_Find(serviceDict, testServiceName, &result)) {
        twDict_Add(serviceDict, testServiceName);
        return 1;
    }
    TW_FREE(testServiceName);
    return 0;
}

void twConstructGenericEdgeThingTemplate(char *thingName, char *namespace) {

    twDataShape *loadShapeLibraryInParams, *addEdgeThingShapeInParams;
    twExt_InheritFromTemplate(thingName, TW_THING_TEMPLATE_EMPTY);

    /* Register loadShapeLibrary() Service */
    loadShapeLibraryInParams = twDataShape_CreateFromEntries(TW_SHAPE_NAME_NONE,
                                                             twDataShapeEntry_Create("libraryName",
                                                                                     "The name of a remote shared extension library.",
                                                                                     TW_STRING), TW_LAST_ARGUMENT
    );

    twExt_RegisterNamespacedService(thingName, "loadShapeLibrary", namespace, "Loads an Edge Thing Shape Library.",
                                    loadShapeLibraryInParams, TW_NOTHING, TW_NO_RETURN_DATASHAPE,
                                    twLoadShapeLibraryService, TW_NO_USER_DATA);


    /* Register AddEdgeThingShape() Service */
    addEdgeThingShapeInParams = twDataShape_CreateFromEntries(TW_SHAPE_NAME_NONE,
                                                              twDataShapeEntry_Create(
                                                                      "shapeName",
                                                                      "The name of the registered shape to add to this thing.",
                                                                      TW_STRING),
                                                              twDataShapeEntry_Create(
                                                                      "namespace",
                                                                      "Optional Namespace to use when adding this shape.",
                                                                      TW_STRING),
                                                              TW_LAST_ARGUMENT
    );
    twExt_RegisterNamespacedService(thingName, "AddEdgeThingShape", namespace, "Loads an Edge Thing Shape Library.",
                                    addEdgeThingShapeInParams, TW_NOTHING, TW_NO_RETURN_DATASHAPE,
                                    twAddEdgeThingShapeService, TW_NO_USER_DATA);
}

void twThingNameToTemplateList_Delete(void *item) {
    twThingNameToTemplateRecord * tmp = (twThingNameToTemplateRecord *) item;
    if (!item) {return;}
    if (tmp->entityName) TW_FREE(tmp->entityName);
    if (tmp->templateName) TW_FREE(tmp->templateName);
    TW_FREE(tmp);
}

twList* twGetThingNameToTemplateList(){
    if(NULL == tw_thingname_template_list){
        tw_thingname_template_list = twList_Create(twThingNameToTemplateList_Delete);
    }
    return tw_thingname_template_list;
}

void twThingNameToShapeList_Delete(void *item) {

    twThingNameToShapeRecord * tmp = (twThingNameToShapeRecord *) item;
    if (!item) {return;}
    if (tmp->entityName) TW_FREE(tmp->entityName);
    if (tmp->shapeName) TW_FREE(tmp->shapeName);
    TW_FREE(tmp);
}

twList* twGetThingNameToShapeList(){
    if(NULL == tw_thingname_shape_list){
        tw_thingname_shape_list = twList_Create(twThingNameToShapeList_Delete);
    }
    return tw_thingname_shape_list;
}

/**
 * Returns a map of Thing names to the Template name they are based on.
 */
twMap* twGetTemplatesMap(void){

    if(NULL == tw_templates_map){
        tw_templates_map = twMap_new();
        twMap_put(tw_templates_map, TW_THING_TEMPLATE_GENERIC, twConstructGenericEdgeThingTemplate);
        twMap_put(tw_templates_map, TW_THING_TEMPLATE_EMPTY, twConstructEmptyEdgeThingTemplate);
    }
    return tw_templates_map;
}

/**
 * Returns the map of Thing Shape names to their construction functions.
 */
twMap* twGetShapesMap(void){

    if(NULL == tw_shapes_map){
        tw_shapes_map = twMap_new();
    }
    return tw_shapes_map;
}

void twSetThingAsUsingShape(const char *thingName, const char *shapeName){
    twList* thingNameToShapeList = twGetThingNameToShapeList();

    twThingNameToShapeRecord * thingNameToShapeRecord = NULL;
    if (!thingName) {
        TW_LOG(TW_ERROR,"twExt_SetThingAsBasedOnTemplateOf:thingName NULL entity pointer passed in.");
        return;
    }
    if (!shapeName) {
        TW_LOG(TW_ERROR,"twExt_SetThingAsBasedOnTemplateOf:shapeName NULL entity pointer passed in.");
        return;
    }

    thingNameToShapeRecord = (twThingNameToShapeRecord *)TW_CALLOC(sizeof(twThingNameToShapeRecord), 1);

    /* populate it */
    thingNameToShapeRecord->entityName = duplicateString(thingName);
    thingNameToShapeRecord->shapeName = duplicateString(shapeName);

    twList_Add(thingNameToShapeList,thingNameToShapeRecord);

}
#ifdef _WIN32
#define MAX_PATH_LEN 260
#define REGISTRY_KEY "SOFTWARE\\Wow6432Node\\Thingworx"

char * twExt_getRegValueAsString(char *valueName, char *defaultValue) {
	char value[255];
	DWORD BufferSize = 255;
	LSTATUS err;
	err = RegGetValue(HKEY_LOCAL_MACHINE, REGISTRY_KEY, valueName, RRF_RT_ANY, NULL, (PVOID) & value, &BufferSize);

	if (ERROR_SUCCESS == err) {
		return duplicateString(value);
	}
	if (NULL == defaultValue) {
		return NULL;
	}
	return duplicateString(defaultValue);
}

/* Windows Implementation of twExt_LoadExtensionLibrary */
void * twExt_LoadExtensionLibrary(char *shapeLibraryName){

    char * initFunctionName = NULL;
	char * twxlibPath = NULL;
	char * twxlibBasePath = NULL;
	char * canonicalLibraryPath = NULL;
	void* returnValue = NULL;
	int initFunctionReturnValue;
	HMODULE libhandle;
    FARPROC initfunc;
	char* noLibName = NULL;
	char * lastPathSeparator = NULL;
	char noLibNameCpy[255];
	char shapePath[255];

   /* Determine the shared library path */
    if(getenv("TWXLIB")){
        twxlibPath = duplicateString(getenv("TWXLIB"));
		if(FALSE == stringEndsWithSuffix(twxlibPath,"/")) {
			concatenateStrings(&twxlibPath, "/");
		}
		twxlibBasePath = twPath_GetFullPath(twxlibPath);
		TW_LOG(TW_DEBUG,"twExt_LoadExtensionLibrary: TWXLIB value was set from the TWXLIB environment variable.");
    } else if(NULL!= twExt_getRegValueAsString("TWXLIB", NULL)){
        twxlibPath = twExt_getRegValueAsString("TWXLIB", NULL);
		if(FALSE == stringEndsWithSuffix(twxlibPath,"/")) {
			concatenateStrings(&twxlibPath, "/");
		}
		twxlibBasePath = twPath_GetFullPath(twxlibPath);
		TW_LOG(TW_DEBUG,"twExt_LoadExtensionLibrary: TWXLIB value was set from the registry entry HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Thingworx\\TWXLIB.");
    } else {
		TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: TWXLIB environment variable is not set, returning NULL");
		return NULL;
	}
	/* DLLs produced by the build script do not have the lib prefix. This is causing them
	to fail to load so remove the lib prefix before trying to load them. In order to support
	a path we must parse out the library name from the rest of the path first. */
	lastPathSeparator = strrchr(shapeLibraryName, '\\');
	if (!lastPathSeparator){
		lastPathSeparator = strrchr (shapeLibraryName, '/');
		}
	if (lastPathSeparator){
		noLibName = lastPathSeparator+1;
	} else {
		noLibName = shapeLibraryName;
	}
	if (0 == strncmp (noLibName, "lib", 3)){
		noLibName += 3;
	}
	/* If we had to strip out the library name from a path, add it back in */
	if (lastPathSeparator){
		int pathLen = strnlen(shapeLibraryName, MAX_PATH_LEN) - strnlen(lastPathSeparator, MAX_PATH_LEN) + 1;
		strncpy(shapePath,shapeLibraryName,pathLen);
		shapePath[lastPathSeparator-shapeLibraryName] = '\0';
		snprintf(noLibNameCpy, 255, "%s\\%s", shapePath, noLibName);
		noLibName=noLibNameCpy;
	}
	concatenateStrings(&twxlibPath,noLibName);
	concatenateStrings (&twxlibPath, ".dll");
	canonicalLibraryPath = twPath_GetFullPath(twxlibPath);

	if(!canonicalLibraryPath){
		TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: The library, %s, could not be found on this device.",twxlibPath);
		TW_FREE(twxlibPath);
		TW_FREE(twxlibBasePath);
		return NULL;
	}
	/* Need to ensure that the library won't try to load from outside the sandbox, ensure the full path is within TWXLIB */
	if (strncmp(twxlibBasePath, canonicalLibraryPath, strnlen(twxlibBasePath, MAX_PATH)) != 0){
		TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: The library %s, is not within the valid base path.",shapeLibraryName);
		TW_FREE(twxlibPath);
		TW_FREE(twxlibBasePath);
		return NULL;
		}
    TW_FREE(twxlibPath);
    TW_FREE(twxlibBasePath);
    TW_LOG(TW_TRACE,"Attempting to load extension library from path: %s.",canonicalLibraryPath);

	/* Attempt to load this DLL extension library */
	libhandle = LoadLibrary(canonicalLibraryPath);
	if(NULL == libhandle){
		DWORD lastError = GetLastError();
		TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: Failed loading extension library %s. The error number was 0x%llx.",canonicalLibraryPath, lastError);
		TW_FREE(canonicalLibraryPath);
		return NULL;
	}
	TW_FREE(canonicalLibraryPath);
	returnValue=libhandle;

	/* Load the plugin initializer Determine init function name */
    initFunctionName = duplicateString("init_");
	if (lastPathSeparator){
		concatenateStrings (&initFunctionName, lastPathSeparator+1);
	} else {
		concatenateStrings (&initFunctionName, shapeLibraryName);
	}
    initfunc = GetProcAddress(libhandle, initFunctionName);
    if(!initfunc){
        TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: Error loading init function %s from library %s. The error Number was 0x%llx.",initFunctionName, shapeLibraryName, GetLastError());
		TW_FREE(initFunctionName);
		FreeLibrary(libhandle);
		return NULL;
	}
	TW_FREE(initFunctionName);

	/* Initialize the plugin
	 During initialization the plugin will register its shapes
	 Its shape property and service dispatcher functions and its shape constructors */
	initFunctionReturnValue = initfunc();
	if (TW_OK != initFunctionReturnValue ) {
        TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: Error. Plugin init function for the library %s returned %d indicating a failure to execute.", shapeLibraryName, initFunctionReturnValue);
		return NULL;
	}
	TW_LOG(TW_INFO, "Successfully loaded extension library %s.",shapeLibraryName);
	return returnValue;
}
#else
/* Unix Implementation of twExt_LoadExtensionLibrary */
void * twExt_LoadExtensionLibrary(char *shapeLibraryName){

    void* returnValue = NULL;
    char * initFunctionName = NULL;
	char * twxlibPath = NULL;
	char * twxlibBasePath = NULL;
	char * potentialLibraryPath1 = NULL;
    char * potentialLibraryPath2 = NULL;
    char* canonicalLibraryPath = NULL;
	int initFunctionReturnValue;
    void* libhandle;
    PluginInitFunc initfunc;
    char * lastPathSeparator = NULL;

    /* Determine the shared library path */
    if(getenv("TWXLIB")){
        twxlibPath = duplicateString(getenv("TWXLIB"));
		if(FALSE == stringEndsWithSuffix(twxlibPath,"/")) {
			concatenateStrings(&twxlibPath, "/");
		}
		twxlibBasePath = twPath_GetFullPath(twxlibPath);
    } else {
		TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: TWXLIB environment variable is not set, returning NULL");
		return NULL;
	}
	concatenateStrings(&twxlibPath,shapeLibraryName);

    /* Attempt to open the shared library plugin first assuming a .so extension and then a .dylib extension */
    potentialLibraryPath1 = duplicateString(twxlibPath);
    concatenateStrings(&potentialLibraryPath1,".so");
    potentialLibraryPath2= duplicateString(twxlibPath);
    concatenateStrings(&potentialLibraryPath2, ".dylib");
    TW_FREE(twxlibPath);

    canonicalLibraryPath = twPath_GetFullPath(potentialLibraryPath1);
    if(NULL == canonicalLibraryPath) {
        TW_LOG(TW_INFO,"twExt_LoadExtensionLibrary: The library %s does not exist on this path. Will try loading it as a dylib.", potentialLibraryPath1);
        canonicalLibraryPath = twPath_GetFullPath(potentialLibraryPath2);
        if(NULL == canonicalLibraryPath) {
            TW_LOG(TW_INFO,"twExt_LoadExtensionLibrary: The library %s does not exist on this path.", potentialLibraryPath2);
        }
    }

    /* Release all paths except the canonical one */
    TW_FREE(potentialLibraryPath1);
    TW_FREE(potentialLibraryPath2);

    if(NULL == canonicalLibraryPath){
		TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: The library %s can't be loaded because it can't be found in your local file system.", shapeLibraryName);
        return NULL;
	} else {
		/* Need to ensure that the library won't try to load from outside the sandbox, ensure the full path is within TWXLIB */
		if (strncmp(twxlibBasePath, canonicalLibraryPath, strnlen(twxlibBasePath, PATH_MAX)) != 0){
			TW_LOG(TW_ERROR,"twExt_LoadExtensionLibrary: The library %s, is not within the valid base path.",shapeLibraryName);
			TW_FREE(twxlibBasePath);
			return NULL;
        }
        TW_FREE(twxlibBasePath);
        TW_LOG(TW_TRACE,"Attempting to load extension library from path: %s.",canonicalLibraryPath);
        libhandle = dlopen(canonicalLibraryPath, RTLD_NOW);
        TW_FREE(canonicalLibraryPath);
        if (NULL == libhandle) {
                TW_LOG(TW_ERROR, "twExt_LoadExtensionLibrary: Error loading extension library %s from path %s with error %s.", shapeLibraryName,
                       canonicalLibraryPath, dlerror());
                return NULL;
        }
    }

    returnValue=libhandle;

    /* Load the initializer function from this library */
    initFunctionName = duplicateString("init_");
    lastPathSeparator = strrchr (shapeLibraryName, '/');
    if (lastPathSeparator){
        concatenateStrings(&initFunctionName,lastPathSeparator+1);
    } else { 
        concatenateStrings(&initFunctionName,shapeLibraryName);
    }
    initfunc = (PluginInitFunc)dlsym(libhandle, initFunctionName);
    if (!initfunc) {
        TW_LOG(TW_ERROR,"Error loading init function %s from library %s with error %s.",initFunctionName, shapeLibraryName, dlerror());
        dlclose(libhandle);
		TW_FREE(initFunctionName);
		return NULL;
    }
	TW_FREE(initFunctionName);

    /* Initialize the plugin. During initialization the plugin will register its shapes,
     shape property and service dispatcher functions and its shape constructors */
    initFunctionReturnValue = initfunc();
    if (TW_OK != initFunctionReturnValue) {
        TW_LOG(TW_ERROR,"Error: Plugin init function for the library %s returned %d indicating a failure to execute.", shapeLibraryName, initFunctionReturnValue);
        dlclose(libhandle);
        return NULL;
    }
	TW_LOG(TW_INFO, "Successfully loaded extension library %s.",shapeLibraryName);
    return returnValue;

}
#endif



void twExt_SetThingAsBasedOnTemplateOf(const char *thingName, const char *templateName){
    twList* thingNameToTemplateList = twGetThingNameToTemplateList();

    twThingNameToTemplateRecord * thingNameToTemplateRecord = NULL;
    if (!thingName) {
        TW_LOG(TW_ERROR,"twExt_SetThingAsBasedOnTemplateOf:thingName NULL entity pointer passed in.");
        return;
    }
    if (!templateName) {
        TW_LOG(TW_ERROR,"twExt_SetThingAsBasedOnTemplateOf:templateName NULL entity pointer passed in.");
        return;
    }

    thingNameToTemplateRecord = (twThingNameToTemplateRecord *)TW_CALLOC(sizeof(twThingNameToTemplateRecord), 1);

    /* populate it */
    thingNameToTemplateRecord->entityName = duplicateString(thingName);
    thingNameToTemplateRecord->templateName = duplicateString(templateName);

    twList_Add(thingNameToTemplateList,thingNameToTemplateRecord);

}

char twExt_DoesThingImplementTemplate(char *entityName, char *templateName){

    twList* thingNameToTemplateList = twGetThingNameToTemplateList();

    struct ListEntry * le = NULL;

    if(NULL == entityName)
        return FALSE;

    if(NULL == templateName)
        return FALSE;

    le = twList_Next(thingNameToTemplateList, 0);
    /* Use an asterisk to check if this enity has registered any properties */
    while (le) {
        if (le->value) {
            twThingNameToTemplateRecord * currentRecord = (twThingNameToTemplateRecord *)(le->value);
            /* Determine if they are for a template or a shape */

            if (strcmp(entityName, currentRecord->entityName)==0) {
                if (strcmp(templateName, currentRecord->templateName)==0) {
                    return TRUE;
                }
            }
        }

        le = twList_Next(thingNameToTemplateList, le);
    }
    return FALSE;

}

char twExt_DoesThingImplementShape(char *entityName, char *shapeName){
    twList* thingNameToShapeList = twGetThingNameToShapeList();

    struct ListEntry * le = NULL;

    if(NULL == entityName)
        return FALSE;

    if(NULL == shapeName)
        return FALSE;

    le = twList_Next(thingNameToShapeList, 0);
    /* Use an asterisk to check if this enity has registered any properties */
    while (le) {
        /* Iterate all polled functions */
        if (le->value) {
            twThingNameToShapeRecord * currentRecord = (twThingNameToShapeRecord *)(le->value);

            if (strcmp(entityName, currentRecord->entityName) == 0) {
                if (strcmp(shapeName, currentRecord->shapeName) == 0) {
                    return TRUE;
                }
            }
        }

        le = twList_Next(thingNameToShapeList, le);
    }
    return FALSE;

}

void twExt_RegisterShape(const char *shapeName,
                         shapeHandlerFunction shapeConstructorFunction){
    twMap* shapeMap = twGetShapesMap();
    twMap_put(shapeMap,shapeName,shapeConstructorFunction);
}

int twExt_AddEdgeThingShape(const char *entityName, const char *shapeName, const char *thing_namespace){
    shapeHandlerFunction shapeFunction = NULL;
    twMap* shapesMap = twGetShapesMap();
    if(TW_MAP_MISSING==twMap_get(shapesMap,shapeName,(void**)&shapeFunction)) {
        TW_LOG(TW_ERROR,"Failed to find Shape: %s",shapeName);
        return TW_LIST_ENTRY_NOT_FOUND;
    }
    twSetThingAsUsingShape(entityName,shapeName);
    shapeFunction(entityName,thing_namespace);

    return TW_OK;

}

void twExt_RegisterTemplate(const char *templateName, templateHandlerFunction handler){
    twMap* templateMap = twGetTemplatesMap();
    twMap_put(templateMap,templateName,handler);
}

int twExt_CreateThingFromTemplate(const char *thingName, const char *templateName, ...){
    va_list ap;
    twMap* templatesMap = twGetTemplatesMap();
    twMap* shapesMap = twGetShapesMap();
    shapeHandlerFunction shapeFunction = NULL;
    char* shapeName;
    templateHandlerFunction templateFunction = NULL;

    /* Determine if the required template is valid */
    if(TW_MAP_MISSING==twMap_get(templatesMap,templateName,(void**)&templateFunction)) {
        va_end(ap);
        TW_LOG(TW_ERROR,"twExt_CreateThingFromTemplate() Failed to find Template: %s",templateName);
        return TW_ERROR_TEMPLATE_DOES_NOT_EXIST;
    }

    /* Call the template constructor function */
    twExt_SetThingAsBasedOnTemplateOf(thingName, templateName);
    templateFunction(thingName,NULL);

    va_start(ap,templateName);
    shapeName = va_arg(ap, char*);
    while(shapeName!=NULL){
        TW_LOG(TW_TRACE,"twExt_CreateThingFromTemplate() using in memory shape map %llu\n", shapesMap);
        if(TW_MAP_MISSING==twMap_get(shapesMap,shapeName,(void**)&shapeFunction)) {
            va_end(ap);
            TW_LOG(TW_ERROR,"twExt_CreateThingFromTemplate() Failed to find Shape: %s",shapeName);
            return TW_ERROR_SHAPE_DOES_NOT_EXIST;
        }
        twSetThingAsUsingShape(thingName,shapeName);
        shapeFunction(thingName,NULL);/* No Namespace */

        shapeName = va_arg(ap, char*);
    }

    va_end(ap);
    return TW_OK;

}

void twExt_InheritFromTemplate(const char *thingName, const char *templateNameToInherit) {
    templateHandlerFunction templateFunction = NULL;
    twMap* templatesMap = twGetTemplatesMap();

    /* Determine if the required template is valid */
    if (TW_MAP_MISSING == twMap_get(templatesMap, templateNameToInherit, (void**)&templateFunction)) {
        return;
    }
    twExt_SetThingAsBasedOnTemplateOf(thingName, templateNameToInherit);
    templateFunction(thingName,NULL);

}

service_cb twExt_GetCallbackForService(char *entityName, char *serviceName){
    void * userdata;
    service_cb serviceCallback = (service_cb)findCallback(TW_THING, entityName,
                        TW_SERVICES, serviceName, &userdata);
    return serviceCallback;
}

int twExt_RegisterStandardProperty(const char *entityName, const char *propertyName, const char *thing_namespace,
                                   enum BaseType propertyType, const char *propertyDescription,
                                   char *propertyPushType, double propertyPushThreshold) {

    int ret = TW_OK;
    char *namespacedPropertyName=(char*)propertyName;

    if(thing_namespace){
        namespacedPropertyName=duplicateString(thing_namespace);
        concatenateStrings(&namespacedPropertyName,"_");
        concatenateStrings(&namespacedPropertyName,propertyName);
    }

    if(twVerifyUniquePropertyName((char*)entityName,namespacedPropertyName)==0){
        return TW_INVALID_PARAM;
    }

    ret = twApi_RegisterProperty(TW_THING, entityName, namespacedPropertyName,
                                 propertyType, propertyDescription, propertyPushType,
                                 propertyPushThreshold, twExt_StandardPropertyHandler, TW_NO_USER_DATA);
    if(namespacedPropertyName!=propertyName){
        TW_FREE(namespacedPropertyName);
    }
    return ret;
}


int twExt_RegisterNamespacedService(const char *entityName, const char *serviceName, const char *thing_namespace,
                                    const char *serviceDescription,
                                    twDataShape *inputs, enum BaseType outputType, twDataShape *outputDataShape,
                                    service_cb cb, void *userdata){

    int ret = TW_OK;
    char *namespacedServiceName=(char*)serviceName;

    if(thing_namespace){
        namespacedServiceName=duplicateString(thing_namespace);
        concatenateStrings(&namespacedServiceName,"_");
        concatenateStrings(&namespacedServiceName,serviceName);
    }
    if(twVerifyUniqueServiceName((char*)entityName,namespacedServiceName)==0){
        return TW_INVALID_PARAM;
    }
    ret = twApi_RegisterService(TW_THING, entityName, namespacedServiceName, (char*)serviceDescription, inputs, outputType, outputDataShape, cb, userdata);

    if(namespacedServiceName!=serviceName) {
        TW_FREE(namespacedServiceName);
    }

    return ret;
}

