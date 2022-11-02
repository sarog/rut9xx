#ifndef TW_C_SDK_TWSTANDARDPROPS_H
#define TW_C_SDK_TWSTANDARDPROPS_H
/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twSimpleProps.h
 * \brief Provides a default property handler which stores all current property values in the heap.
 * \author bill.reichardt@thingworx.com
 *
 * This module provides a function called twExt_genericPropertyHandler() which can be used as a default handler for
 * all declared properties. When this property handler is called from the server it will write the property value
 * change to a hashed map of property values which can then be accessed at any time by your application by calling
 * twExt_GetPropertyValue(). Your application can call twExt_SetPropertyValue() to change this property value as well.
 * You may also establish listener functions which will be called when a specific property changes its value using the
 * twExt_AddPropertyChangeListener() function.
*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "twApi.h"
#include "twOSPort.h"
#include "stringUtils.h"
#include "twMap.h"
#include "twProperties.h"

typedef void (*PropertyChangeListenerFunction)(const char *, const char *, twPrimitive*);

typedef struct twPropertyChangeRecord {
    PropertyChangeListenerFunction listenerFunction;
    char * entityName;
    char * propertyName;
} twPropertyChangeRecord;

/**
 * Set the current property value for the Thing and propertyName combination.
 * The value argument is a twPrimitive value to use as the new value of the property.
 *
 * NOTE: This function only works when you are using the twExt_GenericPropertyHandler() which is automatically installed
 * when you used the twExt_RegisterNamespacedProperty() or the TW_PROPERTY() macro to register a new property.
 *
 * @param thingname the name of the Thing having a propertyName property to set.
 * @param propertyName a string indicateding which property to set.
 * @param value a twPrimitive value representing the new value of this property.
 * @param fold if TRUE, any previous, non pushed value of this property will be replaced with this one.
 * @param push if TRUE, as soon as this property is set, it will be pushed to the server, assuming the
 *              change meets with the criteria for a property value change.
 * @return Status code.
 */
int twExt_SetPropertyValue(const char *thingname, const char *propertyName, twPrimitive *value, char fold, char push);

/**
 * Does the property exist in the Thing.
 * @param thingname Name of Thing.
 * @param propertyName Name of property to look for.
 * @return TRUE if property exists.
 */
char twExt_PropertyExists(const char *thingname, char *propertyName);

/**
 * Get the current property value of the Thing and propertyName combination as a twPrimitive value.
 *
 * There is a convenience macro defined that wraps this function, `TW_GET_PROPERTY()`,
 * which may be used.
 *
 * @param thingname Name of Thing.
 * @param propertyName Name of Property.
 * @return Property value as twPrimitive.
 */
twPrimitive* twExt_GetPropertyValue(const char *thingname, char *propertyName);

/**
 * Add a property listener function for Thing.
 * @param entityName Thing with property.
 * @param propertyName Name of property on Thing.
 * @param propertyChangeListenerFunction Listener function.
 */
void twExt_AddPropertyChangeListener(char *entityName, char *propertyName,
									 PropertyChangeListenerFunction propertyChangeListenerFunction);

/**
 * Remove a property change listener function from service.
 * @param propertyChangeListenerFunction
 */
void twExt_RemovePropertyChangeListener(PropertyChangeListenerFunction propertyChangeListenerFunction);

/**
 * This function can service any request for a property value that can be looked up in the master property name to
 * value map. This property must have been previously set with twSetPropertyValue() since this handler expects
 * the property value to be in its own internal hashmap.
 * @param entityName Thing name.
 * @param propertyName Property name.
 * @param itValue[in, out] InfoTable handle.
 * @param isWrite Update InfoTable.
 * @param userdata Not used.
 * @return Status code.
 *     - TWX_SUCCESS indicates property was handled without error.
 *     - TWX_NOT_FOUND indicates property not found.
 *     - TWX_BAD_REQUEST indicates the function was called with a bad parameter.
 */
enum msgCodeEnum twExt_StandardPropertyHandler(const char *entityName, const char *propertyName, twInfoTable **value,
											   char isWrite, void *userdata);

#endif //TW_C_SDK_TWSTANDARDPROPS_H
