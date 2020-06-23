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
#include "windows.h"
static char const * libname = "warehouseext.dll";
#elif __APPLE__
static char const * libname = "libwarehouseext.dylib";
#else
static char const * libname = "libwarehouseext.so";
#endif

TEST_GROUP(unit_twExt_LoadExtensionLibrary);

TEST_SETUP(unit_twExt_LoadExtensionLibrary){
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

TEST_TEAR_DOWN(unit_twExt_LoadExtensionLibrary){
	char* extDirectory = twGetPreferedExtensionLoadingDirectory ();
	char libCopy[255];
	twApi_Delete();
	TEST_ASSERT_NOT_NULL (extDirectory);
	/* Delete the created file */
	snprintf(libCopy, 255, "%s/../%s", extDirectory, libname);
	twDirectory_DeleteFile(libCopy);
	TW_FREE (extDirectory);
}
TEST_GROUP_RUNNER(unit_twExt_LoadExtensionLibrary) {
	RUN_TEST_CASE(unit_twExt_LoadExtensionLibrary, test_LoadALibraryWindowsRegistry);
	RUN_TEST_CASE(unit_twExt_LoadExtensionLibrary, test_LoadALibraryThatDoesNotExist);
	RUN_TEST_CASE(unit_twExt_LoadExtensionLibrary, test_LoadALibraryInvalidRelativePath);
	RUN_TEST_CASE(unit_twExt_LoadExtensionLibrary, test_LoadALibraryValidRelativePath);
	RUN_TEST_CASE(unit_twExt_LoadExtensionLibrary, test_LoadExtensionWithoutEnvVarSet);

}

extern char* programName;

/**
 * Test Plan: Attempt to load a valid library that is outside the sandbox.
 * Verify that this library fails to load.
 */
TEST(unit_twExt_LoadExtensionLibrary,test_LoadALibraryInvalidRelativePath) {
	void * library=NULL;
	if(!isShared()) {
		return;
	}
	{
		char buffer[256];
		char* extDirectory = twGetPreferedExtensionLoadingDirectory ();
		TEST_ASSERT_NOT_NULL (extDirectory);
		snprintf (buffer, 256, "TWXLIB=%s/", extDirectory);
		TW_FREE (extDirectory);
		putenv (buffer);
	}
	/* Try to load a DLL outside of the root,
	 * In windows both forward and back slashes are valid path separators
	 */
#ifdef _WIN32
	library = twExt_LoadExtensionLibrary("..\\libwarehouseext");
	TEST_ASSERT_NULL(library);
#endif
	library = twExt_LoadExtensionLibrary("../libwarehouseext");
	TEST_ASSERT_NULL(library);
	/* Use different types of encoding, path traversal encodings found here:
	 * https://www.gracefulsecurity.com/path-traversal-cheat-sheet-linux/
	 * The rest of the string is libwarehouseext encoded
	 */
	library = twExt_LoadExtensionLibrary("%2e%2e%2f%6C%69%62%77%61%72%65%68%6F%75%73%65%65%78%74");
	TEST_ASSERT_NULL(library);
	library = twExt_LoadExtensionLibrary("%252e%252e%252f%256C%2569%2562%2577%2561%2572%2565%2568%256F%2575%2573%2565%2565%2578%2574");
	TEST_ASSERT_NULL(library);
	library = twExt_LoadExtensionLibrary("%c0%ae%c0%ae%c0%af%6C%69%62%77%61%72%65%68%6F%75%73%65%65%78%74");
	TEST_ASSERT_NULL(library);
	library = twExt_LoadExtensionLibrary("%uff0e%uff0e%u2215%6C%69%62%77%61%72%65%68%6F%75%73%65%65%78%74");
	TEST_ASSERT_NULL(library);
	library = twExt_LoadExtensionLibrary("%uff0e%uff0e%u2216%6C%69%62%77%61%72%65%68%6F%75%73%65%65%78%74");
	TEST_ASSERT_NULL(library);
}

/**
 * Test Plan: Attempt to load a library that uses a windows path.
 * Clear this applications environment entry and create a registry entry before attempting.
 */
TEST(unit_twExt_LoadExtensionLibrary,test_LoadALibraryWindowsRegistry) {
#ifdef _WIN32
    void * library=NULL;
	LPCSTR sk = "SOFTWARE\\Wow6432Node\\Thingworx";
	HKEY default_key;
	auto status;
	char* extDirectory;
	if(!isShared()) {
		return;
	}


	//TEST_IGNORE_MESSAGE("Ignoring test because it must be run as an administrator.");
	{
		char buffer[256];
		snprintf (buffer, 256, "TWXLIB=");
		putenv (buffer);
	}
	/* Validate the dll cannot be loaded from the environment */
	library = twExt_LoadExtensionLibrary("libwarehouseext");
	TEST_ASSERT_NULL(library);

	/* Create registry entry "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Thingworx\TWXLIB" */
		status = RegCreateKeyExA(HKEY_LOCAL_MACHINE, sk, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &default_key, NULL);
		TEST_ASSERT_EQUAL(status, ERROR_SUCCESS);
		extDirectory = twGetPreferedExtensionLoadingDirectory();
		TEST_ASSERT_NOT_NULL(extDirectory);

		status = RegSetValueExA(default_key, "TWXLIB", 0, REG_EXPAND_SZ, (LPCBYTE)extDirectory, strlen(extDirectory) + 1);
		TEST_ASSERT_EQUAL(status, ERROR_SUCCESS);
		RegCloseKey(default_key);
		TW_FREE(extDirectory);

	/* validate that by adding this registry entry, we now can load this library */
	library = twExt_LoadExtensionLibrary("libwarehouseext");
	TEST_ASSERT_NOT_NULL(library);

	/* Delete Registry Key */
	status = RegCreateKeyExA(HKEY_LOCAL_MACHINE, sk, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &default_key, NULL);
	TEST_ASSERT_EQUAL(status, ERROR_SUCCESS);
	status = RegDeleteValueA(default_key, "TWXLIB");
	TEST_ASSERT_EQUAL(status, ERROR_SUCCESS);
	RegCloseKey(default_key);

#else
	/* This test should only be run on windows */
	return;
#endif
}


/**
 * Test Plan: Attempt to load a library that uses a relative path.
 * Library won't load due to having a path.
 */
TEST(unit_twExt_LoadExtensionLibrary,test_LoadALibraryValidRelativePath) {
	void * library=NULL;
	if(!isShared()) {
		return;
	}
	{
		char buffer[256];
		char* extDirectory = twGetPreferedExtensionLoadingDirectory ();
		TEST_ASSERT_NOT_NULL (extDirectory);
		snprintf (buffer, 256, "TWXLIB=%s/", extDirectory);
		TW_FREE (extDirectory);
		putenv (buffer);
	}
	/* Validate the dll is present */
	library = twExt_LoadExtensionLibrary("libwarehouseext");
	TEST_ASSERT_NOT_NULL(library);
	/* In windows both forward and back slashes are valid path separators */
#ifdef _WIN32
	library = twExt_LoadExtensionLibrary("..\\ext\\libwarehouseext");
	TEST_ASSERT_NOT_NULL(library);
#endif
	library = twExt_LoadExtensionLibrary("../ext/libwarehouseext");
	TEST_ASSERT_NOT_NULL(library);
}

/**
 * Test Plan: Attempt to load a library that cannot possibly exist.
 * Verify that this library fails to load.
 */
TEST(unit_twExt_LoadExtensionLibrary,test_LoadALibraryThatDoesNotExist) {
	void * library=NULL;
	/* Load this shape library either statically or dynamically */
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}
	putenv("TWXLIB=/dunsil");
	library = twExt_LoadExtensionLibrary("libfoobar");
	TEST_ASSERT_NULL(library);

}

TEST(unit_twExt_LoadExtensionLibrary,test_LoadExtensionWithoutEnvVarSet) {
	if(NULL != getenv("TWXLIB")) {
		putenv("TWXLIB=");
	}
	TEST_ASSERT_NULL(twExt_LoadExtensionLibrary("libsimpleext"));
}