#ifndef TW_C_SDK_TWSHAPES_H
#define TW_C_SDK_TWSHAPES_H

/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twShapes.h
 * \brief EdgeThingShape and EdgeThingTemplate Support.
 * \author bill.reichardt@thingworx.com
 *
 * Implements the loading of libraries which provide EdgeThingTemplates and EdgeThingShapes for the C SDK.
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "twApi.h"
#include "twOSPort.h"
#include "stringUtils.h"
#include <twBaseTypes.h>
#include "twStandardProps.h"


#ifndef true
#define true 1
#endif

/* Types */
typedef void (*shapeHandlerFunction)(const char*,const char*);
typedef void (*templateHandlerFunction)(const char*,const char*);

typedef twInfoTable* (*InitialConfigurationHandler)(void);
typedef void (*OnConfigureHandler)(twInfoTable*,char*);

typedef struct twThingNameToTemplateRecord {
    char *entityName;
    char * templateName;
} twThingNameToTemplateRecord;

typedef struct twThingNameToShapeRecord {
    char *entityName;
    char * shapeName;
} twThingNameToShapeRecord;


/* Prototypes */
/**
 * Dynamically loads a shape library and calls its init_<libraryname>() function to register its
 * Shapes and Templates. shapeLibraryName must be a full path to a library if the TWXLIB
 * environment variable is not set, or a partial path relative to the TWXLIB environment variable
 * if it is set.
 * @param shapeLibraryName The name of the library to load. Should not include the file extension.
 * @return Extension library handle.
 */
void * twExt_LoadExtensionLibrary(char *shapeLibraryName);

/**
 * Given the name of a Thing (entityName) and the name of a Shape, returns TRUE if that thing has implemented
 * the provided shape. Can be useful for grouping Things together by "Kind" based on their shape.
 * @param entityName The name of the Thing you wish to test.
 * @param shapeName The name of the Shape you want to test for.
 * @return Boolean: Shape implemented by Thing.
 */
char twExt_DoesThingImplementShape(char *entityName, char *shapeName);

/**
 * Given the name of a Thing (entityName) and the name of a ThingTemplate , returns TRUE if that thing extends
 * the requested ThingTemplate. Can be useful for grouping Things together by "Kind" based on their ThingTemplate.
 * @param entityName The name of the Thing you wish to test.
 * @param templateName The name of the ThingTemplate you want to test for.
 * @return Boolean: Thing extends ThingTemplate.
 */
char twExt_DoesThingImplementTemplate(char *entityName, char *templateName);

/**
 * Add a Shape to an existing Thing. Supports an optional namespace name which will allow a Shape to be added
 * to the same thing more than once. Any properties or services that use a namespace will be added in a format like
 * this:   Property A in namespace B would appear in a thing as property B_A.
 * @param entityName The name of the thing to apply this shape to.
 * @param shapeName The name of the registered shape to apply.
 * @param thing_namespace The name to use as a namespace prefix.
 * @return Result code.
 *     - TW_OK indicates shape added to Thing.
 *     - TW_LIST_ENTRY_NOT_FOUND indicates Shape was not found.
 */
int twExt_AddEdgeThingShape(const char *entityName, const char *shapeName, const char *thing_namespace);

/**
 * Adds a Thing Shape to the Map of available shapes. Should be called in shape library init functions
 * to make a Shape available for use.
 * @param shapeName The name of the shape to register.
 * @param shapeConstructorFunction Function to build shape.
 */
void twExt_RegisterShape(const char *shapeName,
						 shapeHandlerFunction shapeConstructorFunction);

/**
 * Called from a shape library initialization function to register an Edge Thing Template for later use.
 * @param templateName The name of the Template to register.
 * @param handler Template construction function.
 */
void twExt_RegisterTemplate(const char *templateName, templateHandlerFunction handler);

/**
 * Creates a new thing from a thingName, templateName and multiple shape names.
 * Count can be zero and is ignored, the list of shapes must be terminated with a NULL parameter.
 *
 * There is a convenience macro defined, `TW_MAKE_THING()`, that will add the NULL for you.
 *
 * @param thingName The name of a Thing you would like to create.
 * @param templateName The registered thing template to base this thing on.
 * @param ... A NULL terminated list of shape name strings
 * @return TW_OK on success or {TW_ERROR_SHAPE_DOES_NOT_EXIST,TW_ERROR_TEMPLATE_DOES_NOT_EXIST}
 * Note that after templateName you can pass multiple shape names to also add to this thing but this function
 * must have one last argument which is NULL to end the list of shapes. If no shapes are to be used, this function
 * must have a minimum of three arguments, thingName, templateName, and NULL indicating that it has no ThingShapes.
 */
int twExt_CreateThingFromTemplate(const char *thingName, const char *templateName, ...);

/**
 * Register the Template a Thing is based on.
 * @param thingName The name of a Thing you would like to create.
 * @param templateName The name of a Thing you would like to create.
 */
void twExt_SetThingAsBasedOnTemplateOf(const char *thingName, const char *templateName);

/**
 * Apply the provided thingTemplate to the provided thingName.
 * Used to manage inheritance from another ThingTemplate.
 *
 * There are convenience marcos defined that wrap this function, `TW_DECLARE_TEMPLATE`
 * and `TW_TEMPLATE`, which may be used.
 *
 * @param thingName The name of a Thing you would like to create.
 * @param templateNameToInherit The name of the already registered ThingTemplate you would like to base your Thing on.
 */
void twExt_InheritFromTemplate(const char *thingName, const char *templateNameToInherit);

/**
 * Get the callback function for any service registered on a Thing. This can then be used to call the
 * service locally from your application.
 * @param entityName The name of the Thing which implements this service.
 * @param serviceName The name of the service who's function you need to look up.
 * @return Callback function which can be used to call this service.
 */
service_cb twExt_GetCallbackForService(char *entityName, char *serviceName);

/**
 * Namespaced properties are properties that have been given a prefix to avoid a runtime name collision.
 * A namespace string is used as a prefix to make this property name unique. Namespaces should only be provided
 * by the developer who is using your Shape or Template to build their own thing.
 *
 * There are convenience macros defined that wrap this function, `TW_PROPERTY()`
 * and `TW_PROPERTY_LONG()`, which may be used.
 *
 * @param entityName The name of the Thing you want to add this property to.
 * @param propertyName The name of the property you want to add.
 * @param thing_namespace The namespace to be used when this property is created. NULL is acceptable if namespace support is not required.
 * @param propertyType A primitive type selected from the BaseType enum value list.
 * @param propertyDescription A text description of this property. This is optional and NULL can be used in place of a description.
 * @param propertyPushType A string constant describing a strategy for how a property should change before it is considered significant.
 * Options are one of the constants {TW_PUSH_TYPE_ALWAYS, TW_PUSH_TYPE_VALUE, TW_PUSH_TYPE_NEVER}.
 * @param propertyPushThreshold A numeric value to be used in conjunction with the selected propertyPushType. For example, if
 * TW_PUSH_TYPE_VALUE is chosen, then propertyPushThreshold would be the amount of change required before this property would be
 * considered to have changed.
 * @return Result code.
 *     - TW_OK indicates success.
 *     - TW_INVALID_PARAM indicates the property name chosen already exists and cannot be created.
 */
int twExt_RegisterStandardProperty(const char *entityName, const char *propertyName, const char *thing_namespace,
								   enum BaseType propertyType, const char *propertyDescription,
								   char *propertyPushType,
								   double propertyPushThreshold);

/**
 * Namespaced services are services that have been given a prefix to avoid a runtime name collision.
 * A namespace string is used as a prefix to make this service name unique. Namespaces should only be provided
 * by the developer who is using your Shape or Template to build their own thing.
 *
 * There are convenience macros defined that wrap this function, `TW_DECLARE_SERVICE()`
 * and `TW_SERVICE()`, which may be used.
 *
 * @param entityName The name of the Thing you want to add this property to
 * @param serviceName The name of the Service you would like to create
 * @param thing_namespace The namespace to be used when this Service is created. NULL is acceptable if namespace support is not required.
 * @param serviceDescription A text description of this Service. This is optional and NULL can be used in place of a description.
 * @param inputs A Datashape that describes a row of Primitive types which are used to describe the input parameters of this
 * service.
 * @param outputType A primitive type selected from the BaseType enum value list. This is the expected type used
 * as the return value for this function.
 * @param outputDataShape If outputType is TW_INFOTABLE then this parameter must be a DataShape describing the columns
 * used in the returned InfoTable of this Service.
 * @param cb A service callback function. This function will be called when your service is called from ThingWorx. It must
 * use the format specificed by the service_cb function definition. Ex:
 *   myService(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata);
 * @param userdata Any user provided data you would like provided to this function at runtime using the userdata argument of service_cb.
 * @return Result code.
 *     - TW_OK indicates success.
 *     - {TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ALLOCATING_MEMORY, TW_INVALID_PARAM, TW_ERROR_ITEM_EXISTS} indicates failure.
 *       Keep in mind that this function will fail with TW_ERROR_ITEM_EXISTS is the service name you are trying to use already exists.
 */
int twExt_RegisterNamespacedService(const char *entityName, const char *serviceName, const char *thing_namespace,
									const char *serviceDescription,
									twDataShape *inputs, enum BaseType outputType, twDataShape *outputDataShape,
									service_cb cb, void *userdata);


#endif /*TW_C_SDK_TWSHAPES_H */

