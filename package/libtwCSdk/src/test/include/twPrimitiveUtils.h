#ifndef TW_C_SDK_TWPRIMITIVEUTILS_H
#define TW_C_SDK_TWPRIMITIVEUTILS_H

/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twPrimitiveUtils.h
 * \brief Provides a set of functions for rapidly declaring ThingShapes and InfoTables using a variable number of
 * arguemnts
 * \author bill.reichardt@thingworx.com
 *
 * Provides a set of functions for rapidly declaring ThingShapes and InfoTables using a variable number of
 * arguemnts
*/

#include <twBaseTypes.h>
#include "twInfoTable.h"

/**
 * Creates a location primitive from double parameters.
 * @param latitude
 * @param longitude
 * @param elevation
 * @return
 */
twLocation* twCreateLocationFrom(double latitude, double longitude, double elevation);

/**
 * Supports the declaration of an InfoTableRow from a variable number of twPrimitive * arguments.
 * The list of twPrimitive * arguments must be terminated with a NULL parameter.
 *
 * @param firstEntry
 * @return
 */
twInfoTableRow* twInfoTable_CreateRowFromEntries(twPrimitive * firstEntry, ...);

/**
 * Supports the declaration of an InfoTableRow from a variable number of arguments.
 * All arguments after dataShape must be of type twInfoTableRow* and must be terminated with a NULL parameter.
 * @param dataShape
 * @return
 */
twInfoTable* twInfoTable_CreateInfoTableFromRows(twDataShape * dataShape, ...);
/**
 * Supports the declaration of a DataShape from a variable number of twDataShapeEntry * arguments.
 * The list of twDataShapeEntry * arguments must be terminated with a NULL parameter.
 *
 * @param shapeName
 * @param firstEntry
 * @return
 */
twDataShape * twDataShape_CreateFromEntries(char* shapeName,twDataShapeEntry* firstEntry, ...);


#endif //TW_C_SDK_TWPRIMITIVEUTILS_H
