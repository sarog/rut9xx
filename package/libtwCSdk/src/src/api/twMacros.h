/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

#ifndef TW_C_SDK_TWMACROS_H
#define TW_C_SDK_TWMACROS_H

/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

#define TW_THING_TEMPLATE_GENERIC "GenericEdgeThingTemplate"
#define TW_THING_TEMPLATE_EMPTY "EmptyTemplate"
#define TW_UNUSED_VARIABLE(x) ((void)x)

/**
 * \file twShapeMacros.h
 * \brief Provides a set of macros for simplifying Thing, Shape, Template and Primitive Declarations
 * \author bill.reichardt@thingworx.com
 *
*/

/** @brief Use this constant in any property, event or service description parameter such as the arguments of TW_PROPERTY(),
 * TW_DS_ENTRY() or TW_SERVICE() or when you do not want to provide a description.
 */
#define VAR_ARG_END NULL

/**
 * \defgroup DeclarationUtilities Declaration Macros for Things, Shapes and Templates
 * @{
 */
/**
* @def TW_DECLARE_SHAPE
* @brief Declares a set of standard variables defining a ThingShape. This macro is used to establish the ThingName,
 * ShapeName and Namespace used in future macro calls.
* @param aThingName const char *  [in] The name of the Thing that will be using this Shape.
* @param shapeName const char * [in] The name of the Shape being declared.
* @param thing_namespace const char * [in] a string used to prevent naming conflicts when different shapes are used together.
* @return This macro does not return any value
*/
#define TW_DECLARE_SHAPE(aThingName,shapeName,thing_namespace) const char* _tw_thing_name=aThingName;const char* _tw_thing_namespace = thing_namespace;const char* _tw_shape_name=shapeName;TW_UNUSED_VARIABLE(_tw_thing_namespace);TW_UNUSED_VARIABLE(_tw_thing_name);TW_UNUSED_VARIABLE(_tw_shape_name);
/**
* @def TW_SHAPE
* @brief A briefer version of TW_DECLARE_SHAPE. Declares a set of standard variables defining a ThingShape. This macro
 * is used to establish the ThingName, ShapeName and Namespace used in future macro calls.
* @param aThingName const char *  [in] The name of the Thing that will be using this Shape.
* @param shapeName const char * [in] The name of the Shape being declared.
* @param thing_namespace const char * [in] a string used to prevent naming conflicts when different shapes are used together.
* @return This macro does not return any value
*/
#define TW_SHAPE(aThingName,shapeName,thing_namespace) const char* _tw_thing_name=aThingName;const char* _tw_thing_namespace = thing_namespace;const char* _tw_shape_name=shapeName;TW_UNUSED_VARIABLE(_tw_thing_namespace);TW_UNUSED_VARIABLE(_tw_thing_name);TW_UNUSED_VARIABLE(_tw_shape_name);
/**
* @def TW_DECLARE_TEMPLATE
* @brief Declares a set of standard variables defining a ThingTemplate. This
*   macro is used to establish the ThingName, TemplateName and Namespace used in future macro calls.
* @param aThingName const char *  [in] The name of the Thing that will be using this Template.
* @param templateName const char * [in] The name of the Template being declared.
* @param inheritsTemplate const char * [in] the name of the registered template that this thing should be based on.
* @return This macro does not return any value
*/
#define TW_DECLARE_TEMPLATE(aThingName,templateName,inheritsTemplate) const char* _tw_thing_name=aThingName;const char* _tw_thing_namespace = TW_NO_NAMESPACE;const char* _tw_template_name=templateName;twExt_InheritFromTemplate(aThingName, inheritsTemplate);TW_UNUSED_VARIABLE(_tw_thing_name);TW_UNUSED_VARIABLE(_tw_thing_namespace);TW_UNUSED_VARIABLE(_tw_template_name);
/**
* @def TW_TEMPLATE
* @brief A briefer version of TW_DECLARE_TEMPLATE. Declares a set of standard variables defining a ThingTemplate. This
*   macro is used to establish the ThingName, TemplateName and Namespace used in future macro calls.
* @param aThingName const char *  [in] The name of the Thing that will be using this Template.
* @param templateName const char * [in] The name of the Template being declared.
* @param inheritsTemplate const char * [in] the name of the registered template that this thing should be based on.* @return This macro does not return any value
*/
#define TW_TEMPLATE(aThingName,templateName,inheritsTemplate) const char* _tw_thing_name=aThingName;const char* _tw_thing_namespace = TW_NO_NAMESPACE;const char* _tw_template_name=templateName;twExt_InheritFromTemplate(aThingName, inheritsTemplate);TW_UNUSED_VARIABLE(_tw_thing_name);TW_UNUSED_VARIABLE(_tw_thing_namespace);TW_UNUSED_VARIABLE(_tw_template_name);

/**
* @def TW_DECLARE_EVENT
* @brief Creates a definition for an Event your application intends to emit at some
 *  point in the future and allows you to associate a DataShape describing the event's payload data.
* @param eventName const char *  [in] The identifying name for this Event.
* @param eventDescription const char * [in] An optional text description of the meaning of this event. If not provided use TW_NO_DESCRIPTION.
* @param eventDataShape twDataShape * [in] A DataShape describing the payload delivered with this event. Use the TW_MAKE_DATASHAPE macro
*   to create a DataShape for this parameter.
* @return TW_OK on success, {TW_INVALID_PARAM, TW_ERROR_ALLOCATING_MEMORY,TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ITEM_EXISTS} on failure
*/
#define TW_DECLARE_EVENT(eventName,eventDescription,eventDataShape) twApi_RegisterEvent(TW_THING, _tw_thing_name, eventName, eventDescription, eventDataShape);
/**
* @def TW_DECLARE_EVENT
* @brief A briefer version of TW_DECLARE_EVENT. Creates a definition for an Event your application intends to emit at some
 *  point in the future and allows you to associate a DataShape describing the event's payload data.
* @param eventName const char *  [in] The identifying name for this Event.
* @param eventDescription const char * [in] An optional text description of the meaning of this event. If not provided use TW_NO_DESCRIPTION.
* @param eventDataShape twDataShape * [in] A DataShape describing the payload delivered with this event. Use the TW_MAKE_DATASHAPE macro
*   to create a DataShape for this parameter.
* @return TW_OK on success, {TW_INVALID_PARAM, TW_ERROR_ALLOCATING_MEMORY,TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ITEM_EXISTS} on failure
*/
#define TW_EVENT(eventName,eventDescription,eventDataShape) twApi_RegisterEvent(TW_THING, _tw_thing_name, eventName, eventDescription, eventDataShape)
/**@}*/


/**
 * \defgroup PropertyDeclaration Property Declaration Macros
 * @{
 */
/**
* @def TW_PROPERTY
* @brief Defines a Property of A Thing. This macro must be proceeded by either TW_DECLARE_SHAPE,TW_DECLARE_TEMPLATE or TW_MAKE_THING
*  because these macros declare variables used by the TW_PROPERTY that follow them. Note that when using TW_PROPERTY to declare a property
*  you are accepting the use of the default property handler. This property handler will allocate and manage the storage used for this
*  property automatically.
* @param propertyName const char *  [in] The identifying name for this Property.
* @param description const char * [in] An optional text description of the meaning of this event. If not provided use TW_NO_DESCRIPTION.
* @param type twPrimitive * [in] A ThingWorx primitive type. Use of of the enum BaseType values. Examples are {TW_STRING,TW_NUMBER,TW_BOOLEAN}
* @return TW_OK on success, {TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ALLOCATING_MEMORY,TW_INVALID_PARAM,TW_ERROR_ITEM_EXISTS} on failure
*/
#define TW_PROPERTY(propertyName,description,type) twExt_RegisterStandardProperty(_tw_thing_name, propertyName, _tw_thing_namespace ,type, description, TW_PUSH_TYPE_ALWAYS,REPORT_ALL_CHANGES)
/**
* @def TW_PROPERTY_LONG
* @brief Defines a Property of A Thing. This version of TW_PROPERTY provides more options. This macro must be proceeded
*  by either TW_DECLARE_SHAPE,TW_DECLARE_TEMPLATE or TW_MAKE_THING
*  because these macros declare variables used by the TW_PROPERTY that follow them.
* @param propertyName const char *  [in] The identifying name for this Property.
* @param description const char * [in] An optional text description of the meaning of this event. If not provided use TW_NO_DESCRIPTION.
* @param type twPrimitive * [in] A ThingWorx primitive type. Use of of the enum BaseType values. Examples are {TW_STRING,TW_NUMBER,TW_BOOLEAN}
* @param pushType const char * [in] Defines a strategy for determining if your property value has changed significantly enough
*   to warrant pushing it up to the server. Valid values are TW_PUSH_TYPE_ALWAYS,TW_PUSH_TYPE_VALUE or TW_PUSH_TYPE_NEVER.
* @param threshold double [in] If your strategy set in pushType is TW_PUSH_TYPE_VALUE this is the numerical difference
*   that must occur between the last and the current property value to consider the change significant enough to push to the server.
* @return TW_OK on success, {TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ALLOCATING_MEMORY,TW_INVALID_PARAM,TW_ERROR_ITEM_EXISTS} on failure
*/
#define TW_PROPERTY_LONG(propertyName,description,type,pushType,threshold) twExt_RegisterStandardProperty(_tw_thing_name, propertyName, _tw_thing_namespace ,type, description, pushType,threshold)
/**
* @def TW_ADD_NUMBER_ASPECT
* @brief Aspects modify the behavior of a Property. See the ThingWorx composer for examples of available aspects of a Property.
 *   This macro is used to assign numeric values to aspects.
* @param propertyName const char *  [in] The name of the property to apply this Aspect to.
* @param aspectName const char * [in] The name of the Aspect to add to a property. Some aspects are {TW_ASPECT_ISPERSISTENT,
*  TW_ASPECT_ISREADONLY,TW_ASPECT_DATASHAPE,TW_ASPECT_ISLOGGED,TW_ASPECT_PUSHTYPE,TW_ASPECT_DATACHANGETYPE,TW_ASPECT_DATACHANGETHRESHOLD,TW_ASPECT_ISFOLDED,TW_ASPECT_DEFAULT_VALUE}
* @param valuedouble double [in] A value to apply for this aspect. Not all aspects accept all types (NUMBER,STRING,BOOLEAN)
* @return TW_OK on success, {TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ALLOCATING_MEMORY,TW_INVALID_PARAM,TW_ERROR_ITEM_EXISTS} on failure
*/
#define TW_ADD_NUMBER_ASPECT(propertyName,aspectName,valuedouble) twApi_AddAspectToProperty(_tw_thing_name, propertyName, aspectName, twPrimitive_CreateFromNumber(valuedouble))
/**
* @def TW_ADD_BOOLEAN_ASPECT
* @brief Aspects modify the behavior of a Property. See the ThingWorx composer for examples of available aspects of a Property.
 *   This macro is used to assign boolean values to aspects.
* @param propertyName const char *  [in] The name of the property to apply this Aspect to.
* @param aspectName const char * [in] The name of the Aspect to add to a property. Some aspects are {TW_ASPECT_ISPERSISTENT,
*  TW_ASPECT_ISREADONLY,TW_ASPECT_DATASHAPE,TW_ASPECT_ISLOGGED,TW_ASPECT_PUSHTYPE,TW_ASPECT_DATACHANGETYPE,TW_ASPECT_DATACHANGETHRESHOLD,TW_ASPECT_ISFOLDED,TW_ASPECT_DEFAULT_VALUE}
* @param valuebool char [in] A boolean value to apply for this aspect. Acceptable values are {TRUE,FALSE}
* @return TW_OK on success, {TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ALLOCATING_MEMORY,TW_INVALID_PARAM,TW_ERROR_ITEM_EXISTS} on failure
*/
#define TW_ADD_BOOLEAN_ASPECT(propertyName,aspectName,valuebool) twApi_AddAspectToProperty(_tw_thing_name, propertyName, aspectName, twPrimitive_CreateFromBoolean(valuebool))

/**
* @def TW_ADD_STRING_ASPECT
* @brief Aspects modify the behavior of a Property. See the ThingWorx composer for examples of available aspects of a Property.
 *   This macro is used to assign string values to aspects.
* @param propertyName const char *  [in] The name of the property to apply this Aspect to.
* @param aspectName const char * [in] The name of the Aspect to add to a property. Some aspects are {TW_ASPECT_ISPERSISTENT,
*  TW_ASPECT_ISREADONLY,TW_ASPECT_DATASHAPE,TW_ASPECT_ISLOGGED,TW_ASPECT_PUSHTYPE,TW_ASPECT_DATACHANGETYPE,TW_ASPECT_DATACHANGETHRESHOLD,TW_ASPECT_ISFOLDED,TW_ASPECT_DEFAULT_VALUE}
* @param valuebool const char * [in] A string value to apply for this aspect.
* @return TW_OK on success, {TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ALLOCATING_MEMORY,TW_INVALID_PARAM,TW_ERROR_ITEM_EXISTS} on failure
*/
#define TW_ADD_STRING_ASPECT(propertyName,aspectName,valuestring) twApi_AddAspectToProperty(_tw_thing_name, propertyName, aspectName, twPrimitive_CreateFromString(valuestring, TRUE))
/**@}*/

/**
 * \defgroup PropertyAccess Property Access Macros
 * @{
 */
/**
* @def TW_SET_PROPERTY
* @brief Set a value for a property previously declared using TW_PROPERTY. The setting of a property value does not push it to
 * the server. It records this value change for later delivery using the TW_PUSH macro. Multiple values along with the current
 * time set using TW_SET_PROPERTY will be stored in memory until TW_PUSH is called unless. Do not attempt to use this macro
 * on a property not declared using TW_PROPERTY. All property change listeners will be notified of this change.
* @param thingName const char *  [in] The name of the the Thing to apply this property change to.
* @param propertyName const char *  [in] The name of the property who's value you intend to change.
* @param value twPrimitive * [in] The value to set this property to. This must be provided in the form of a ThingWorx Primitive.
*   Primitives are created using one of these macros: TW_MAKE_NUMBER,TW_MAKE_INT,TW_MAKE_STRING ,TW_MAKE_BOOL,TW_MAKE_DATETIME,
*   TW_MAKE_DATETIME_NOW,TW_MAKE_EMPTY,TW_MAKE_LOC.
* @return TW_OK on success, {TW_NULL_OR_INVALID_API_SINGLETON,TW_INVALID_PARAM,TW_SUBSCRIBED_PROPERTY_NOT_FOUND} on failure.
*/
#define TW_SET_PROPERTY(thingName, propertyName,value) twExt_SetPropertyValue(thingName, propertyName, value,0,0)
/**
* @def TW_GET_PROPERTY
* @brief Returns the current Primitive value of this property. It is up to you to select the correct field from the
 * returned twPrimitive* which is returned. Do not attempt to use this macro on a property not declared using TW_PROPERTY.
* @param thingName const char *  [in] The name of the the Thing to return the property value of.
* @param propertyName const char *  [in] The name of the property who's value you want returned.
* @return the ->val field of a twPrimitive*. This is useful because you can then select the apropriate field of val to get
 * a specific C value.  Example: TW_GET_PROPERTY("MyThing","propertyA").number or TW_GET_PROPERTY("MyThing","propertyA").boolean
*/
#define TW_GET_PROPERTY(thingName, propertyName) twExt_GetPropertyValue(thingName,propertyName)->val
/**
* @def TW_GET_PROPERTY_TYPE
* @brief Returns the ThingWorx Primitive type of an existing property.
* @param thingName const char *  [in] The name of the the Thing to return the property type of.
* @param propertyName const char *  [in] The name of the property who's type you want returned.
* @return one of the values of enum BaseType. Examples would be {TW_STRING,TW_NUMBER,TW_BOOLEAN}.
*/
#define TW_GET_PROPERTY_TYPE(thingName, propertyName) twExt_GetPropertyValue(thingName,propertyName)->type
/**
* @def TW_GET_STRING_PROPERTY
* @brief A convenience version of TW_GET_PROPERTY which always returns a String. Do not attempt to use this macro on a
*   property not declared using TW_PROPERTY.
* @param thingName const char *  [in] The name of the the Thing to return the property value of.
* @param propertyName const char *  [in] The name of the property who's value you want returned.
* @return const char* to the string Primitives ->val.bytes.data field which contains the actual string value.
*/
#define TW_GET_STRING_PROPERTY(thingName, propertyName) twExt_GetPropertyValue(thingName,propertyName)->val.bytes.data
/**
* @def TW_PUSH_PROPERTIES_FOR
* @brief Delivers any property changes made with TW_SET_PROPERTY to the server.
* @param thingName const char *  [in] The name of the the Thing who's property changes have been previously set. To push
*   all stored values use TW_PUSH_ALL_THINGS.
* @param force char [in] Should a connection be established to deliver these changes if one does not already exist?
*   Acceptable values are TW_PUSH_CONNECT_FORCE, TW_PUSH_CONNECT_LATER
* @return TW_OK on success. {TW_UNKNOWN_ERROR,TW_SUBSCRIBED_PROPERTY_LIST_PERSISTED,TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED,
*   TW_PRECONDITION_FAILED} on failure.
*/
#define TW_PUSH_PROPERTIES_FOR(thingName,force) twApi_PushSubscribedProperties(thingName, force);
/**
* @def TW_BIND
* @brief Will bind a Thing definition to the ThingWorx server. Can only be used after a call to TW_MAKE_THING because
*   it requires the variables this macro declares. Binding is the processes of informing the ThingWorx server that a Thing
*   is supported by your application. Binding can be done with your application either connected or disconnected from
*   the server. If done before connection, all bind messages will be consolidated into a single message to the server.
*   If done after connection, each bind message will be sent as soon as this macro is used.
* @param thingName const char *  [in] The name of the the Thing to Bind.
* @return TW_OK on success. {TW_UNKNOWN_ERROR,TW_NULL_OR_INVALID_API_SINGLETON,TW_INVALID_PARAM} on failure.
*/
#define TW_BIND() twApi_BindThing(_tw_thing_name);
/**@}*/




/**
 *
 * \defgroup DataShape DataShape Macros
 * @{
 */
/**
 * @def TW_MAKE_DATASHAPE
 * @brief Accepts a shape name and n number of arguments, each being a description of the datatype of a column as created by
 * TW_DS_ENTRY(). Each argument must be presented in column order to describe the columns of a table.
 * @param shapeName const char * [in] The name of the DataShape to create. DataShape names are optional and the constant
 *   TW_SHAPE_NAME_NONE can be used if no name is required.
 * @param firstShape twDataShapeEntry* [in] A DataShapeEntry created using the macro TW_DS_ENTRY or TW_DECLARE_DS_ENTRY.
 *   This argument can be repeated once or each DataShape column you need to declare.
 * @return a twDataShape* structure
 */
#define TW_MAKE_DATASHAPE(shapeName,firstShape,...)  twDataShape_CreateFromEntries(shapeName,firstShape,##__VA_ARGS__,VAR_ARG_END)
/**
 * @def TW_DECLARE_DS_ENTRY
 * @brief Creates a DataShapeEntry for use in conjunction with the TW_MAKE_DATASHAPE macro. Each DataShapeEntry represents
 *   a column definition in your datashape.
 * @param fieldName const char * [in] The name of the DataShape to create. DataShape names are optional and the constant
 *   TW_SHAPE_NAME_NONE can be used if no name is required.
 * @param description A text description of the purpose of this column. If no description is required use TW_NO_DESCRIPTION
 * @param type twDataShapeEntry* [in] A DataShapeEntry created using the macro TW_DS_ENTRY or TW_DECLARE_DS_ENTRY.
 *   This argument can be repeated once or each DataShape column you need to add to your DataShape entry.
 * @return a twDataShapeEntry* structure
 */
#define TW_DECLARE_DS_ENTRY(fieldName,description,type) twDataShapeEntry_Create(fieldName, description, type)
/**
 * @def TW_DS_ENTRY
 * @brief A shorter version of TW_DECLARE_DS_ENTRY. Creates a DataShapeEntry for use in conjunction with the TW_MAKE_DATASHAPE macro. Each DataShapeEntry represents
 *   a column definition in your datashape.
 * @param fieldName const char * [in] The name of the DataShape to create. DataShape names are optional and the constant
 *   TW_SHAPE_NAME_NONE can be used if no name is required.
 * @param description A text description of the purpose of this column. If no description is required use TW_NO_DESCRIPTION
 * @param type twDataShapeEntry* [in] A DataShapeEntry created using the macro TW_DS_ENTRY or TW_DECLARE_DS_ENTRY.
 *   This argument can be repeated once or each DataShape column you need to add to your DataShape entry.
 * @return a twDataShapeEntry* structure
 */
#define TW_DS_ENTRY(fieldName,description,type) twDataShapeEntry_Create(fieldName, description, type)
/**@}*/

/**
 * \defgroup ServiceDeclaration Service Declaration Macros
 * @{
 */

/**
 * @def TW_DECLARE_SERVICE
 * @brief Defines a service name on your Thing, specifies its input and return parameters and maps it to en existing C function.
 * @param serviceName const char * [in] The name of the DataShape to create. DataShape names are optional and the constant
 *   TW_SHAPE_NAME_NONE can be used if no name is required.
 * @param description A text description of the purpose of this service. If no description is required use TW_NO_DESCRIPTION
 * @param inputShape twDataShape* [in] A DataShape created using the macro TW_MAKE_DATASHAPE that represents
 *   the input parameters of your Service.
 * @param outputType twPrimitive * [in] A ThingWorx primitive type representing the return type for your Service. Use one
 *   of the enum BaseType values. Examples are {TW_STRING,TW_NUMBER,TW_BOOLEAN,TW_INFOTABLE}
 * @param outputShape twDataShape* [in] A DataShapeEntry created using the macro TW_MAKE_DATASHAPE that should only be used
 *   if outputType is TW_INFOTABLE to define the DataShape to be used by the returned InfoTable. If outputType is anything other than TW_INFOTABLE this parameter should be TW_NO_RETURN_DATASHAPE.
 * @param serviceHandler service_cb [in] A pointer to a C function of type service_cb. Example: enum msgCodeEnum myServiceFunction(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata);
 * @return TW_OK on success. {TW_INVALID_PARAM,TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ALLOCATING_MEMORY} on failure
 */
#define TW_DECLARE_SERVICE(serviceName, description,inputShape,outputType,outputShape,serviceHandler) twExt_RegisterNamespacedService(_tw_thing_name, serviceName, _tw_thing_namespace , description, inputShape, outputType, outputShape, serviceHandler, NULL)
/**
 * @def TW_SERVICE
 * @brief A shorter version of TW_DECLARE_SERVICE. Defines a service name on your Thing, specifies its input and return parameters and maps it to en existing C function.
 * @param serviceName const char * [in] The name of the DataShape to create. DataShape names are optional and the constant
 *   TW_SHAPE_NAME_NONE can be used if no name is required.
 * @param description A text description of the purpose of this service. If no description is required use TW_NO_DESCRIPTION
 * @param inputShape twDataShape* [in] A DataShape created using the macro TW_MAKE_DATASHAPE that represents
 *   the input parameters of your Service.
 * @param outputType twPrimitive * [in] A ThingWorx primitive type representing the return type for your Service. Use one
 *   of the enum BaseType values. Examples are {TW_STRING,TW_NUMBER,TW_BOOLEAN,TW_INFOTABLE}
 * @param outputShape twDataShape* [in] A DataShapeEntry created using the macro TW_MAKE_DATASHAPE that should only be used
 *   if outputType is TW_INFOTABLE to define the DataShape to be used by the returned InfoTable. If outputType is anything other than TW_INFOTABLE this parameter should be TW_NO_RETURN_DATASHAPE.
 * @param serviceHandler service_cb [in] A pointer to a C function of type service_cb. Example: enum msgCodeEnum myServiceFunction(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata);
 * @return TW_OK on success. {TW_INVALID_PARAM,TW_NULL_OR_INVALID_API_SINGLETON,TW_ERROR_ALLOCATING_MEMORY} on failure
 */
#define TW_SERVICE(serviceName, description,inputShape,outputType,outputShape,serviceHandler) twExt_RegisterNamespacedService(_tw_thing_name, serviceName, _tw_thing_namespace , description, inputShape, outputType, outputShape, serviceHandler, NULL)
/**
 * @def TW_GET_STRING_PARAM
 * @brief A utility macro that can be used to extract a String parameter from the first row of an InfoTable. It is intended
 *   to be used inside Service handlers to extract input parameters that are assumed to be Strings from the input InfoTable.
 * @param params twInfoTable* An infotable passed into a Service handler function containing its input parameters.
 * @param paramName  const char * [in] The name of the parameter to extract from the first row of the params InfoTable.
 * @return A const char* to the requested string
 */
#define TW_GET_STRING_PARAM(params,paramName) twInfoTable_GetString(params, #paramName, 0, &paramName);
/**
 * @def TW_GET_NUMBER_PARAM
 * @brief A utility macro that can be used to extract a double/Number parameter from the first row of an InfoTable. It is intended
 *   to be used inside Service handlers to extract input parameters that are assumed to be Numbers from the input InfoTable.
 * @param params twInfoTable* An InfoTable passed into a Service handler function containing its input parameters.
 * @param paramName  const char * [in] The name of the parameter to extract from the first row of the params InfoTable.
 * @return a double containing the Number field value
 */
#define TW_GET_NUMBER_PARAM(params,paramName) twInfoTable_GetNumber(params, #paramName, 0, &paramName);
/**@}*/

/**
 * \defgroup PrimitiveDeclaration Primitive Declaration Macros
 * @{
 */
/**
 * @def TW_MAKE_NUMBER
 * @brief This macro creates Number primitives from C doubles. Useful for populating InfoTables or calling API functions.
 *   Note that the returned primitive has been allocated on the heap and if not used in a function call that takes over
 *   responsibility for this value, must be disposed of with TW_FREE().
 * @param numberValue double[in] A c double value to be converted
 * @return twPrimitive* the created primitive value.
 */
#define TW_MAKE_NUMBER(numberValue) twPrimitive_CreateFromNumber((double)numberValue)
/**
 * @def TW_MAKE_INT
 * @brief This macro creates Integer primitives from C unsigned integers. Useful for populating InfoTables or calling API functions.
 *   Note that the returned primitive has been allocated on the heap and if not used in a function call that takes over
 *   responsibility for this value, must be disposed of with TW_FREE().
 * @param numberValue int32_t[in] A C signed integer value to be converted
 * @return twPrimitive* the created primitive value.
 */
#define TW_MAKE_INT(intValue) twPrimitive_CreateFromInteger(intValue)
/**
 * @def TW_MAKE_STRING
 * @brief This macro creates String primitives from a C char *. Useful for populating InfoTables or calling API functions.
 *   Note that the returned primitive has been allocated on the heap and if not used in a function call that takes over
 *   responsibility for this value, must be disposed of with TW_FREE().
 * @param stringValue const char*[in] A C string value to be converted
 * @return twPrimitive* the created primitive value.
 */
#define TW_MAKE_STRING(stringValue) twPrimitive_CreateFromString((char*)stringValue,TRUE)
/**
 * @def TW_MAKE_BOOL
 * @brief This macro creates Boolean primitives from a C char which is the SDK boolean type. Useful for populating InfoTables or calling API functions.
 *   Note that the returned primitive has been allocated on the heap and if not used in a function call that takes over
 *   responsibility for this value, must be disposed of with TW_FREE().
 * @param boolValue char A C char to be converted
 * @return twPrimitive* the created primitive value.
 */
#define TW_MAKE_BOOL(boolValue) twPrimitive_CreateFromBoolean(boolValue)
/**
 * @def TW_MAKE_DATETIME
 * @brief This macro creates DateTime primitives from a C DATETIME. Useful for populating InfoTables or calling API functions.
 *   Note that the returned primitive has been allocated on the heap and if not used in a function call that takes over
 *   responsibility for this value, must be disposed of with TW_FREE().
 * @param timestamp C DATETIME to be converted
 * @return twPrimitive* the created primitive value.
 */
#define TW_MAKE_DATETIME(timestamp) twPrimitive_CreateFromDatetime(timestamp)
/**
 * @def TW_MAKE_DATETIME_NOW
 * @brief A convenience version of TW_MAKE_DATETIME that assumes you want to create a primitive representing the current time.
 *   It requires no arguments. Useful for populating InfoTables or calling API functions.
 *   Note that the returned primitive has been allocated on the heap and if not used in a function call that takes over
 *   responsibility for this value, must be disposed of with TW_FREE().
 * @return twPrimitive* the created primitive value.
 */
#define TW_MAKE_DATETIME_NOW twPrimitive_CreateFromDatetime(twGetSystemTime(TRUE))
/**
 * @def TW_MAKE_EMPTY
 * @brief This macro creates an emptry Primitive of type TW_NOTHING. It requires not arguments.
 *   Note that the returned primitive has been allocated on the heap and if not used in a function call that takes over
 *   responsibility for this value, must be disposed of with TW_FREE().
 * @return twPrimitive* the created primitive value.
 */
#define TW_MAKE_EMPTY twPrimitive_Create()
/**
 * @def TW_MAKE_LOC
 * @brief This macro creates a Location primitive from a latitude, longitude and elevation.
 *   Note that the returned primitive has been allocated on the heap and if not used in a function call that takes over
 *   responsibility for this value, must be disposed of with TW_FREE().
 * @param latitude double [in]
 * @param longitude double [in]
 * @param elevation double [in]
 * @return twPrimitive* the created primitive value.
 */
#define TW_MAKE_LOC(latitude,longitude,elevation) twPrimitive_CreateFromLocationAndDelete(twCreateLocationFrom(latitude,longitude,elevation))

/**@}*/

/**
 * \defgroup InfoTableCreation InfoTable Creation Macros
 * @{
 */
/**
 * @def TW_DECLARE_IT_ROW
 * @brief Creates an InfoTableRow structure on the heap and returns it. Most often used in the construction of InfoTables
 *   using the TW_MAKE_INFOTABLE() macro.
 * @param primitive twPrimitive* [in] A list of one or more primitives to be used to populate the row being created.
 * @return twInfoTableRow* the created row containing all the primitives passed in the argument list.
 */
#define TW_DECLARE_IT_ROW(primitive,...) twInfoTable_CreateRowFromEntries(primitive,##__VA_ARGS__,VAR_ARG_END)
/**
 * @def TW_IT_ROW
 * @brief A short version of TW_DECLARE_IT_ROW. Creates an InfoTableRow structure on the heap and returns it. Most often used in the construction of InfoTables
 *   using the TW_MAKE_INFOTABLE() macro.
 * @param primitive twPrimitive* [in] A list of one or more primitives to be used to populate the row being created.
 * @return twInfoTableRow* the created row containing all the primitives passed in the argument list.
 */
#define TW_IT_ROW(primitive,...) twInfoTable_CreateRowFromEntries(primitive,##__VA_ARGS__,VAR_ARG_END)
/**
 * @def TW_MAKE_INFOTABLE
 * @brief Creates an InfoTable structure on the heap. Should be used in conjunction with TW_IT_ROW() and TW_MAKE_DATASHAPE()
 *   to simplify the creation of complete InfoTables.
 * @param dataShape twDataShape* [in] A datashape that describes each row of this InfoTable.
 * @param infoTableRow twInfoTableRow* [in] A list of 0 or more InfoTable rows that conform to dataShape.
 * @return twInfoTable* the created InfoTable containing all the rows passed in the argument list.
 */
#define TW_MAKE_INFOTABLE(dataShape,...) twInfoTable_CreateInfoTableFromRows(dataShape,##__VA_ARGS__,VAR_ARG_END)
/**
 * @def TW_MAKE_IT
 * @brief A shorter version of TW_MAKE_INFOTABLE. Creates an InfoTable structure on the heap. Should be used in conjunction with TW_IT_ROW() and TW_MAKE_DATASHAPE()
 *   to simplify the creation of complete InfoTables.
 * @param dataShape twDataShape* [in] A datashape that describes each row of this InfoTable.
 * @param infoTableRow twInfoTableRow* [in] A list of 0 or more InfoTable rows that conform to dataShape.
 * @return twInfoTable* the created InfoTable containing all the rows passed in the argument list.
 */
#define TW_MAKE_IT(dataShape,...) twInfoTable_CreateInfoTableFromRows(dataShape,##__VA_ARGS__,VAR_ARG_END)
/**@}*/

/**
 * \defgroup ThingCreation Thing Creation Macros
 * @{
 */
/**
 * @def TW_MAKE_THING
 * @brief Creates a Thing based on a ThingTemplate and declares variables which can be used by the TW_PROPERTY and TW_SERVICE macros.
 * @param thingName const char* [in] The name of the thing to be created
 * @param templateName const char* [in] The name of the ThingTemplate to be used. If not custom ThingTemplate is required
 *   use TW_THING_TEMPLATE_GENERIC.
 * @return returns no value
 */
#define TW_MAKE_THING(thingName,templateName,...) char * _tw_thing_name=thingName;const char* _tw_thing_namespace = TW_NO_NAMESPACE;twExt_CreateThingFromTemplate(thingName,templateName,##__VA_ARGS__,VAR_ARG_END);TW_UNUSED_VARIABLE(_tw_thing_namespace);TW_UNUSED_VARIABLE(_tw_thing_name)
/**@}*/

/**
 * \defgroup EventCreation Event Creation Macros
 * @{
 */
/**
 * @def TW_FIRE_EVENT
 * @brief Fires an Event from your remote application to ThingWorx. Events should be registered in advance using the TW_EVENT() macro.
 * @param thingName const char* [in] The name of the thing originating this event.
 * @param eventName const char* [in] The name of the Event to be fired.
 * @param eventInfoTable twInfoTable* [in] The InfoTable containing the Event payload as declared using TW_EVENT() and created using TW_MAKE_IT()
 * @return returns no value
 */
#define TW_FIRE_EVENT(thingName,eventName,eventInfoTable) twApi_FireEvent(TW_THING, thingName, eventName, eventInfoTable, -1, TRUE)
/**@}*/

/**
 * \defgroup FileTransfer File Transfer Support Macros
 * @{
 */
/**
 * @def TW_ADD_FILE_TRANSFER_SHAPE
 * @brief Adds all services required for the current Thing to support File Transfers. Should be called once before using TW_SHARE_DIRECTORY().
 * @return returns no value
 */
#define TW_ADD_FILE_TRANSFER_SHAPE() twFileManager_Create()
/**
 * @def TW_SHARE_DIRECTORY
 * @brief Shares a directory on disk from within your application. Must be called after TW_ADD_FILE_TRANSFER_SHAPE(). Shared directories must exist before this macro is called.
 * @param alias const char* [in] a Name of a virtual directory to be created off of your virtual root "/". Example: An alias of shared would appear as a virtual directory "/shared".
 * @param path const char* [in] A full path to the local directory you indend to share the contents of.
 * @return returns TW_OK on success. Returns {TW_FILE_XFER_MANAGER_NOT_INITIALIZED,TW_ERROR_ALLOCATING_MEMORY} on failure.
 */
#define TW_SHARE_DIRECTORY(alias,path) twFileManager_AddVirtualDir(_tw_thing_name, alias, path)
/**@}*/

 #endif //TW_C_SDK_TWMACROS_H
