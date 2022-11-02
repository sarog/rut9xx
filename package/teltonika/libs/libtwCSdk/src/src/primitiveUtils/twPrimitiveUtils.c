/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

#include "twPrimitiveUtils.h"
#include <stdarg.h>

twLocation* twCreateLocationFrom(double latitude, double longitude, double elevation){
    twLocation *location = TW_MALLOC(sizeof(twLocation));
    location->elevation=elevation;
    location->longitude=longitude;
    location->latitude=latitude;
    return location;
}

twDataShape * twDataShape_CreateFromEntries(char* shapeName,twDataShapeEntry* firstEntry, ...){

    va_list ap;
    twDataShapeEntry* entry;
    twDataShape* ds;

    if(NULL==firstEntry) {
        return NULL;
    }

    ds = twDataShape_Create(firstEntry);
	if(shapeName!=NULL){
		twDataShape_SetName(ds, shapeName);
	}

    va_start(ap,firstEntry);
    entry = va_arg(ap, twDataShapeEntry *);

	while(TRUE){
		if(NULL == entry){
			break;
		}
		twDataShape_AddEntry(ds,entry);
		entry = va_arg(ap, twDataShapeEntry *);

	}

    va_end(ap);

    return ds;

}

twInfoTableRow* twInfoTable_CreateRowFromEntries(twPrimitive * firstEntry, ... ){

    va_list ap;
    twPrimitive * entry;
    twInfoTableRow* row;

	if(NULL==firstEntry) {
		return NULL;
	}

    va_start(ap, firstEntry);
    row = twInfoTableRow_Create(firstEntry);
    while(TRUE){
        entry = va_arg(ap, twPrimitive *);
        if(NULL == entry){
            break;
        }
        twInfoTableRow_AddEntry(row, entry);
    }
    va_end(ap);
    return row;

}

twInfoTable* twInfoTable_CreateInfoTableFromRows(twDataShape * dataShape, ...){

    va_list ap;
    twInfoTableRow* row = NULL;
    twInfoTable* it = twInfoTable_Create(dataShape);
	if(NULL == it){
		return NULL;
	}
    va_start(ap, dataShape);
    while(TRUE){
        row = va_arg(ap, twInfoTableRow*);
        if(NULL == row){
            break;
        }
        twInfoTable_AddRow(it, row);
    }

    va_end(ap);
    return it;

}
