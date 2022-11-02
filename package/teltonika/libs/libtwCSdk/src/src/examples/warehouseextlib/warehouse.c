/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/
/**
 * This Shape library contains three shapes.
 * A WareHouse Shape and An Inventory Shape and an AddressShape
 */

#include "twBaseTypes.h"
#include "twExt.h"
#include "twMacros.h"

twDataShape* getInventoryDataShape(){
	return TW_MAKE_DATASHAPE("InventoryEntry",
							 TW_DS_ENTRY("description", "Description of Inventory Item" ,TW_STRING),
							 TW_DS_ENTRY("price", "Price of Inventory Item" ,TW_NUMBER)
	);
}

enum msgCodeEnum generateSalutation(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata){
	 char * title = NULL;
	 char * salutation = NULL;

    /* Get the parameters */
    TW_GET_STRING_PARAM(params,title);

	/*set greeting */
    salutation = duplicateString("Dear ");

	/* Set title */
	concatenateStrings(&salutation,title);

	/* add blank space */
    concatenateStrings(&salutation," ");

	/* set first name*/
	concatenateStrings(&salutation, TW_GET_STRING_PROPERTY(entityName, "firstName"));
	
	/* add blank space */
    concatenateStrings(&salutation," ");

	/* set last name */
    concatenateStrings(&salutation, TW_GET_STRING_PROPERTY(entityName, "lastName"));

    /* Return Result */
    *content = twInfoTable_CreateFromString("result",salutation,FALSE);
    return TWX_SUCCESS;

}

void createInfotablePropertyValue(char * entityName){
	twInfoTable* it = TW_MAKE_IT(
			getInventoryDataShape(),
			TW_IT_ROW(TW_MAKE_STRING("Hat"),TW_MAKE_NUMBER(5.0))
	);
	TW_SET_PROPERTY(entityName,"inventory",twPrimitive_CreateFromInfoTable(it));
}

enum msgCodeEnum addProduct(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata){
    twInfoTable* it = NULL;
	twInfoTable* retIt = NULL;
	char* description;
	double price;

	TW_GET_STRING_PARAM(params,description);
    TW_GET_NUMBER_PARAM(params,price);

    it = TW_GET_PROPERTY((char*)entityName,"inventory").infotable;

    twInfoTable_AddRow(it, 
		TW_IT_ROW(
			TW_MAKE_STRING(description),
			TW_MAKE_NUMBER(price)
		)
	);

    *content = NULL;
    return TWX_SUCCESS;
 }

enum msgCodeEnum currentInventory(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata){
    *content = twInfoTable_FullCopy(TW_GET_PROPERTY(entityName,"inventory").infotable);
    return TWX_SUCCESS;
}

void constructWarehouseTemplate(const char* thingName,const char* namespace) {

    {
        TW_DECLARE_TEMPLATE(thingName, "Warehouse Template", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY("instanceName", "Instance Name", TW_STRING);
    }

}


void constructInventoryShape(const char* thingName,const char* namespace) {

    {
        TW_DECLARE_SHAPE(thingName, "Inventory Shape",namespace);
		TW_PROPERTY("inventory", "A list of inventory", TW_INFOTABLE);
		createInfotablePropertyValue(thingName);
        TW_DECLARE_SERVICE("addProduct",
                           "Adds A product to the Inventory",
						   getInventoryDataShape(),
                           TW_NOTHING,
                           TW_NO_RETURN_DATASHAPE,
                           addProduct
        );
        TW_DECLARE_SERVICE("currentInventory",
                           "Displays the Inventory",
                           TW_NO_PARAMETERS,
                           TW_INFOTABLE,
						   getInventoryDataShape(),
                           currentInventory
        );
    }

}

void constructAddressShape(const char* thingName,const char* namespace){
    {
        TW_DECLARE_SHAPE(thingName,"Address Shape",namespace);
		TW_PROPERTY("category", "What category of business this is.", TW_STRING);
		TW_PROPERTY("firstName", "Your First Name", TW_STRING);
		TW_PROPERTY("lastName", "Your Last Name", TW_STRING);
		TW_PROPERTY("address", "Where this business is located", TW_STRING);
		TW_PROPERTY("city", TW_NO_DESCRIPTION, TW_STRING);
		TW_PROPERTY("state", TW_NO_DESCRIPTION, TW_STRING);
		TW_PROPERTY("zip", TW_NO_DESCRIPTION, TW_STRING);
		TW_PROPERTY("yearsAtLocation", "", TW_NUMBER);


		TW_DECLARE_SERVICE("generateSalutation",
                           "Creates a salutation sentence.",
                           TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
                                   TW_DS_ENTRY("title", "Ms,Mrs,Dr or Mr" ,TW_STRING)
                           ),
                           TW_STRING,
                           TW_NO_RETURN_DATASHAPE,
                           generateSalutation
        );
    }
}

/**
 * Called to initialize this library
 */
int init_libwarehouseext(){
    twExt_RegisterTemplate("WarehouseTemplate", constructWarehouseTemplate);
    twExt_RegisterShape("AddressShape", constructAddressShape);
    twExt_RegisterShape("InventoryShape", constructInventoryShape);
	return TW_OK;
}

