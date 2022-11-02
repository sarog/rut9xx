/*
 * Created by William Reichardt on 5/26/16.
 */

#include "twApi.h"
#include <twServices.h>
#include "twProperties.h"
#include "twShapes.h"
#include "twMacros.h"
#include "twConstants.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"

#ifdef _WIN32
static char const * libname = "warehouseext.dll";
#elif __APPLE__
static char const * libname = "libwarehouseext.dylib";
#else
static char const * libname = "libwarehouseext.so";
#endif

TEST_GROUP(unit_twExt_CreateThingFromTemplate);

TEST_SETUP(unit_twExt_CreateThingFromTemplate){
	char* extDirectory = twGetPreferedExtensionLoadingDirectory ();
	char dest[255];
	char srcLib[255];
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	/* Create a copy of the warehouseext library one level up from the ext folder */
	snprintf(dest, 255, "%s/../%s", extDirectory, libname);
	snprintf(srcLib, 255, "%s/%s", extDirectory, libname);
	TW_FREE (extDirectory);
	TEST_ASSERT_EQUAL(TW_OK,twDirectory_CopyFile(srcLib,dest));
}

TEST_TEAR_DOWN(unit_twExt_CreateThingFromTemplate){
	char* extDirectory = twGetPreferedExtensionLoadingDirectory ();
	char libCopy[255];
	twApi_Delete();
	TEST_ASSERT_NOT_NULL (extDirectory);
	/* Delete the created file */
	snprintf(libCopy, 255, "%s/../%s", extDirectory, libname);
	twDirectory_DeleteFile(libCopy);
	TW_FREE (extDirectory);
}
TEST_GROUP_RUNNER(unit_twExt_CreateThingFromTemplate) {
	RUN_TEST_CASE(unit_twExt_CreateThingFromTemplate, test_CreateSimpleThingWithTemplate);
	RUN_TEST_CASE(unit_twExt_CreateThingFromTemplate, test_CreateSimpleThingsWithTemplate);

}

extern char* programName;

/**
 * Test Plan: Instantiate a thing using a template and shapes. Verify its metadata.
 */
TEST(unit_twExt_CreateThingFromTemplate,test_CreateSimpleThingWithTemplate) {
	void * library=NULL;
	/* Load this shape library either statically or dynamically */
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	} else {
		/* Load Dynamically */
		{
			char buffer[256];
			char* extDirectory = twGetPreferedExtensionLoadingDirectory();
			TEST_ASSERT_NOT_NULL(extDirectory);
			snprintf(buffer,256,"TWXLIB=%s/",extDirectory);
			TW_FREE(extDirectory);
			putenv(buffer);
		}
		library = twExt_LoadExtensionLibrary("libwarehouseext");
		TEST_ASSERT_NOT_NULL(library);
	}

	{

		twServiceDef * generateSalutationService;
		twDataShapeEntry * entry;
		twServiceDef * addProductService;
		twDataShape * addProductServiceParamsShape;
		twList* addProductListItems;
		twServiceDef * currentInventoryService;
		twDataShape * currentInventoryOutputShape;
		twList* currentInventoryListItems;
		twDataShape * generateSalutationServiceParamsShape;
		twList* salutationListItems;

		TW_MAKE_THING("NewCThing","WarehouseTemplate","InventoryShape","AddressShape");

		/* Test for presence of all properties on the Template and the Shapes */
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"category"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"firstName"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"lastName"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"address"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"city"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"state"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"zip"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"yearsAtLocation"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"inventory"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"instanceName"));

		/* Tests for presence of all declared services */

		/* GenerateSalutation Service */
		generateSalutationService = findService(_tw_thing_name,"generateSalutation");
		TEST_ASSERT_NOT_NULL(generateSalutationService);
		TEST_ASSERT_EQUAL_STRING("Creates a salutation sentence.",generateSalutationService->description);
		/* Verify Input Parameter */
		TEST_ASSERT_EQUAL_INT(1,generateSalutationService->inputs->numEntries);
		generateSalutationServiceParamsShape = generateSalutationService->inputs;
		salutationListItems = generateSalutationServiceParamsShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(salutationListItems, 0));
		entry = twList_GetByIndex(salutationListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("title",entry->name);
		TEST_ASSERT_EQUAL(TW_STRING,entry->type);
		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_STRING,generateSalutationService->outputType);


		/* AddProduct Service */
		addProductService = findService(_tw_thing_name,"addProduct");
		TEST_ASSERT_NOT_NULL(addProductService);
		TEST_ASSERT_EQUAL_INT(2,addProductService->inputs->numEntries);
		addProductServiceParamsShape = addProductService->inputs;
		addProductListItems = addProductServiceParamsShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(addProductListItems, 0));
		entry = twList_GetByIndex(addProductListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("description",entry->name);
		TEST_ASSERT_EQUAL(TW_STRING,entry->type);
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(addProductListItems, 1));
		entry = twList_GetByIndex(addProductListItems, 1)->value;
		TEST_ASSERT_EQUAL_STRING("price",entry->name);
		TEST_ASSERT_EQUAL(TW_NUMBER,entry->type);

		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_NOTHING,addProductService->outputType);


		/* currentInventory Service */
		currentInventoryService = findService(_tw_thing_name,"currentInventory");
		TEST_ASSERT_NOT_NULL(currentInventoryService);
		TEST_ASSERT_NULL(currentInventoryService->inputs);

		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_INFOTABLE,currentInventoryService->outputType);

		/* Verify output parameter DataShape */
		currentInventoryOutputShape = currentInventoryService->outputDataShape;
		currentInventoryListItems = currentInventoryOutputShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(currentInventoryListItems, 0));
		entry = twList_GetByIndex(currentInventoryListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("description",entry->name);
		TEST_ASSERT_EQUAL(TW_STRING,entry->type);
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(currentInventoryListItems, 1));
		entry = twList_GetByIndex(currentInventoryListItems, 1)->value;
		TEST_ASSERT_EQUAL_STRING("price",entry->name);
		TEST_ASSERT_EQUAL(TW_NUMBER,entry->type);
	}

}

/**
 * Test Plan: Instantiate multiple things using a template and shapes. Verify its metadata.
 */
TEST(unit_twExt_CreateThingFromTemplate,test_CreateSimpleThingsWithTemplate) {
	void * library=NULL;
	/* Load this shape library either statically or dynamically */
	if(!isShared()) {
		/* Load Statically
		  * This test is not used when statically linking
		  */
		return;
	} else {
		/* Load Dynamically */
		{
			char buffer[256];
			char* extDirectory = twGetPreferedExtensionLoadingDirectory();
			TEST_ASSERT_NOT_NULL(extDirectory);
			snprintf(buffer,256,"TWXLIB=%s/",extDirectory);
			TW_FREE(extDirectory);
			putenv(buffer);
		}
		library = twExt_LoadExtensionLibrary("libwarehouseext");
		TEST_ASSERT_NOT_NULL(library);
	}

	/* CThing1 */
	{

		twServiceDef * generateSalutationService;
		twDataShapeEntry * entry;
		twServiceDef * addProductService;
		twDataShape * addProductServiceParamsShape;
		twList* addProductListItems;
		twServiceDef * currentInventoryService;
		twDataShape * currentInventoryOutputShape;
		twList* currentInventoryListItems;
		twDataShape * generateSalutationServiceParamsShape;
		twList* salutationListItems;

		TW_MAKE_THING("CThing1", "WarehouseTemplate", "InventoryShape", "AddressShape");


		/* Test for presence of all properties on the Template and the Shapes */
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "category"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "firstName"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "lastName"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "address"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "city"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "state"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "zip"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "yearsAtLocation"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "inventory"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name, "instanceName"));

		/* Tests for presence of all declared services */

		/* GenerateSalutation Service */
		generateSalutationService = findService(_tw_thing_name, "generateSalutation");

		TEST_ASSERT_NOT_NULL(generateSalutationService);
		TEST_ASSERT_EQUAL_STRING("Creates a salutation sentence.", generateSalutationService->description);
		/* Verify Input Parameter */
		TEST_ASSERT_EQUAL_INT(1, generateSalutationService->inputs->numEntries);
		generateSalutationServiceParamsShape = generateSalutationService->inputs;
		salutationListItems = generateSalutationServiceParamsShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(salutationListItems, 0));
		entry = twList_GetByIndex(salutationListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("title", entry->name);
		TEST_ASSERT_EQUAL(TW_STRING, entry->type);
		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_STRING, generateSalutationService->outputType);


		/* AddProduct Service */
		addProductService = findService(_tw_thing_name, "addProduct");
		TEST_ASSERT_NOT_NULL(addProductService);
		TEST_ASSERT_EQUAL_INT(2, addProductService->inputs->numEntries);
		addProductServiceParamsShape = addProductService->inputs;
		addProductListItems = addProductServiceParamsShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(addProductListItems, 0));
		entry = twList_GetByIndex(addProductListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("description", entry->name);
		TEST_ASSERT_EQUAL(TW_STRING, entry->type);
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(addProductListItems, 1));
		entry = twList_GetByIndex(addProductListItems, 1)->value;
		TEST_ASSERT_EQUAL_STRING("price", entry->name);
		TEST_ASSERT_EQUAL(TW_NUMBER, entry->type);

		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_NOTHING, addProductService->outputType);


		/* currentInventory Service */
		currentInventoryService = findService(_tw_thing_name, "currentInventory");
		TEST_ASSERT_NOT_NULL(currentInventoryService);
		TEST_ASSERT_NULL(currentInventoryService->inputs);

		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_INFOTABLE, currentInventoryService->outputType);

		/* Verify output parameter DataShape */
		currentInventoryOutputShape = currentInventoryService->outputDataShape;
		currentInventoryListItems = currentInventoryOutputShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(currentInventoryListItems, 0));
		entry = twList_GetByIndex(currentInventoryListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("description", entry->name);
		TEST_ASSERT_EQUAL(TW_STRING, entry->type);
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(currentInventoryListItems, 1));
		entry = twList_GetByIndex(currentInventoryListItems, 1)->value;
		TEST_ASSERT_EQUAL_STRING("price", entry->name);
		TEST_ASSERT_EQUAL(TW_NUMBER, entry->type);
	}

	/* CThing2 */
	{

		twServiceDef * generateSalutationService;
		twDataShapeEntry * entry;
		twServiceDef * addProductService;
		twDataShape * addProductServiceParamsShape;
		twList* addProductListItems;
		twServiceDef * currentInventoryService;
		twDataShape * currentInventoryOutputShape;
		twList* currentInventoryListItems;
		twDataShape * generateSalutationServiceParamsShape;
		twList* salutationListItems;

		TW_MAKE_THING("CThing2","WarehouseTemplate","InventoryShape","AddressShape");

		/* Test for presence of all properties on the Template and the Shapes */
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"category"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"firstName"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"lastName"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"address"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"city"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"state"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"zip"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"yearsAtLocation"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"inventory"));
		TEST_ASSERT_NOT_NULL(findProperty(_tw_thing_name,"instanceName"));

		/* Tests for presence of all declared services */

		/* GenerateSalutation Service */
		generateSalutationService = findService(_tw_thing_name,"generateSalutation");
		TEST_ASSERT_NOT_NULL(generateSalutationService);
		TEST_ASSERT_EQUAL_STRING("Creates a salutation sentence.",generateSalutationService->description);
		/* Verify Input Parameter */
		TEST_ASSERT_EQUAL_INT(1,generateSalutationService->inputs->numEntries);
		generateSalutationServiceParamsShape = generateSalutationService->inputs;
		salutationListItems = generateSalutationServiceParamsShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(salutationListItems, 0));
		entry = twList_GetByIndex(salutationListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("title",entry->name);
		TEST_ASSERT_EQUAL(TW_STRING,entry->type);
		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_STRING,generateSalutationService->outputType);


		/* AddProduct Service */
		addProductService = findService(_tw_thing_name,"addProduct");
		TEST_ASSERT_NOT_NULL(addProductService);
		TEST_ASSERT_EQUAL_INT(2,addProductService->inputs->numEntries);
		addProductServiceParamsShape = addProductService->inputs;
		addProductListItems = addProductServiceParamsShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(addProductListItems, 0));
		entry = twList_GetByIndex(addProductListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("description",entry->name);
		TEST_ASSERT_EQUAL(TW_STRING,entry->type);
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(addProductListItems, 1));
		entry = twList_GetByIndex(addProductListItems, 1)->value;
		TEST_ASSERT_EQUAL_STRING("price",entry->name);
		TEST_ASSERT_EQUAL(TW_NUMBER,entry->type);

		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_NOTHING,addProductService->outputType);


		/* currentInventory Service */
		currentInventoryService = findService(_tw_thing_name,"currentInventory");
		TEST_ASSERT_NOT_NULL(currentInventoryService);
		TEST_ASSERT_NULL(currentInventoryService->inputs);

		/* Verify Response Parameter */
		TEST_ASSERT_EQUAL(TW_INFOTABLE,currentInventoryService->outputType);

		/*( Verify output parameter DataShape */
		currentInventoryOutputShape = currentInventoryService->outputDataShape;
		currentInventoryListItems = currentInventoryOutputShape->entries;
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(currentInventoryListItems, 0));
		entry = twList_GetByIndex(currentInventoryListItems, 0)->value;
		TEST_ASSERT_EQUAL_STRING("description",entry->name);
		TEST_ASSERT_EQUAL(TW_STRING,entry->type);
		TEST_ASSERT_NOT_NULL(twList_GetByIndex(currentInventoryListItems, 1));
		entry = twList_GetByIndex(currentInventoryListItems, 1)->value;
		TEST_ASSERT_EQUAL_STRING("price",entry->name);
		TEST_ASSERT_EQUAL(TW_NUMBER,entry->type);
	}

}
