/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twApi_ZipExtractFile()
*/

#include <fcntl.h>
#include "twExt.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twZipTar.h"

TEST_GROUP(unit_twApi_ZipExtractFile);

TEST_SETUP(unit_twApi_ZipExtractFile){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

TEST_TEAR_DOWN(unit_twApi_ZipExtractFile){
	twApi_Delete();
}

TEST_GROUP_RUNNER(unit_twApi_ZipExtractFile){
	RUN_TEST_CASE(unit_twApi_ZipExtractFile, test_SimpleUnzip);
	RUN_TEST_CASE(unit_twApi_ZipExtractFile, test_UnzipFileThatDoesNotExist);
	RUN_TEST_CASE(unit_twApi_ZipExtractFile, test_SimpleUnTgz);
	RUN_TEST_CASE(unit_twApi_ZipExtractFile, test_UnTgzFileThatDoesNotExist);
}

TEST(unit_twApi_ZipExtractFile, test_SimpleUnzip) {
	char buffer[25];
	size_t len;
	FILE* fp;
	size_t bytes_read;
	char fileNameBuffer[255];
	char zipFileSourcePath[255];
	char zipFileTargetDirectory[255];
	char* etcDirectory = getEtcDirectory();
	char *simplePayloadFile = "simplePayload";
	snprintf(zipFileSourcePath, 255, "./%s.zip", simplePayloadFile);
	snprintf(zipFileTargetDirectory, 255, "./%s/", simplePayloadFile);

	/* Delete Any existing file or matching directory at target*/
	twDirectory_DeleteDirectory(zipFileTargetDirectory);
	twDirectory_DeleteFile(zipFileSourcePath);

	/* Move a copy of this file to CWD */
	snprintf(fileNameBuffer, 255, "%s/%s.zip", etcDirectory, simplePayloadFile);
	TEST_ASSERT_EQUAL(TW_OK,twDirectory_CopyFile(fileNameBuffer,zipFileSourcePath));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ZipExtractFile(zipFileSourcePath));

	/* Verify three files are present */
	snprintf(fileNameBuffer, 255, "./%s/file1.txt", simplePayloadFile);
	TEST_ASSERT_TRUE(twDirectory_FileExists(fileNameBuffer));
	snprintf(fileNameBuffer, 255, "./%s/file2.txt", simplePayloadFile);
	TEST_ASSERT_TRUE(twDirectory_FileExists(fileNameBuffer));
	snprintf(fileNameBuffer, 255, "./%s/file3.txt", simplePayloadFile);
	TEST_ASSERT_TRUE(twDirectory_FileExists(fileNameBuffer));

	/* Open one and verify contents */
	len = 25;
	fp = TW_FOPEN("./simplePayload/file1.txt", "rb");
	bytes_read = TW_FREAD(buffer, len, 1, fp);
	TEST_ASSERT_TRUE(bytes_read != -1);
	TEST_ASSERT_EQUAL(0,strncmp("This is file 1",buffer,13));
	fclose(fp);

	/* Clean Up */
	twDirectory_DeleteDirectory(zipFileTargetDirectory);
	twDirectory_DeleteFile(zipFileSourcePath);

}

TEST(unit_twApi_ZipExtractFile, test_UnzipFileThatDoesNotExist) {
	char fileNameBuffer[255];
	char zipFileSourcePath[255];
	char zipFileTargetDirectory[255];
	char *simplePayloadFile = "noPayload";
	snprintf(zipFileSourcePath, 255, "./%s.zip", simplePayloadFile);
	snprintf(zipFileTargetDirectory, 255, "./%s/", simplePayloadFile);

	/* Delete Any existing file or matching directory at target*/
	twDirectory_DeleteDirectory(zipFileTargetDirectory);
	twDirectory_DeleteFile(zipFileSourcePath);

	TEST_ASSERT_NOT_EQUAL(TW_OK, twApi_ZipExtractFile(zipFileSourcePath));

	/* Verify three files are present */
	snprintf(fileNameBuffer, 255, "./%s/file1.txt", simplePayloadFile);
	TEST_ASSERT_FALSE(twDirectory_FileExists(fileNameBuffer));
	snprintf(fileNameBuffer, 255, "./%s/file2.txt", simplePayloadFile);
	TEST_ASSERT_FALSE(twDirectory_FileExists(fileNameBuffer));
	snprintf(fileNameBuffer, 255, "./%s/file3.txt", simplePayloadFile);
	TEST_ASSERT_FALSE(twDirectory_FileExists(fileNameBuffer));

	/* Clean Up */
	twDirectory_DeleteDirectory(zipFileTargetDirectory);
	twDirectory_DeleteFile(zipFileSourcePath);
}


TEST(unit_twApi_ZipExtractFile, test_SimpleUnTgz) {
	char buffer[25];
	size_t len;
	FILE* fp;
	size_t bytes_read;
	char fileNameBuffer[255];
	char tgzFileSourcePath[255];
	char tgzFileTargetDirectory[255];
	char* etcDirectory = getEtcDirectory();
	char *simplePayloadFile = "simplePayload";
	snprintf(tgzFileSourcePath, 255, "./%s.tgz", simplePayloadFile);
	snprintf(tgzFileTargetDirectory, 255, "./%s/", simplePayloadFile);

	/* Delete Any existing file or matching directory at target*/
	twDirectory_DeleteDirectory(tgzFileTargetDirectory);
	twDirectory_DeleteFile(tgzFileSourcePath);

	/* Move a copy of this file to CWD */
	snprintf(fileNameBuffer, 255, "%s/%s.tgz", etcDirectory, simplePayloadFile);
	TEST_ASSERT_EQUAL(TW_OK,twDirectory_CopyFile(fileNameBuffer,tgzFileSourcePath));
	TEST_ASSERT_EQUAL(TW_OK, twApi_TgzExtractFile(tgzFileSourcePath));

	/* Verify three files are present */
	snprintf(fileNameBuffer, 255, "./%s/file1.txt", simplePayloadFile);
	TEST_ASSERT_TRUE(twDirectory_FileExists(fileNameBuffer));
	snprintf(fileNameBuffer, 255, "./%s/file2.txt", simplePayloadFile);
	TEST_ASSERT_TRUE(twDirectory_FileExists(fileNameBuffer));
	snprintf(fileNameBuffer, 255, "./%s/file3.txt", simplePayloadFile);
	TEST_ASSERT_TRUE(twDirectory_FileExists(fileNameBuffer));

	/* Open one and verify contents */
	len = 25;
	fp = TW_FOPEN("./simplePayload/file1.txt", "rb");
	bytes_read = TW_FREAD(buffer, len, 1, fp);
	TEST_ASSERT_TRUE(bytes_read != -1);
	TEST_ASSERT_EQUAL(0,strncmp("This is file 1",buffer,13));
	fclose(fp);

	/* Clean Up */
	twDirectory_DeleteDirectory(tgzFileTargetDirectory);
	twDirectory_DeleteFile(tgzFileSourcePath);
}

TEST(unit_twApi_ZipExtractFile, test_UnTgzFileThatDoesNotExist) {
	char fileNameBuffer[255];
	char tgzFileSourcePath[255];
	char tgzFileTargetDirectory[255];
	char *simplePayloadFile = "noPayload";
	snprintf(tgzFileSourcePath, 255, "./%s.tgz", simplePayloadFile);
	snprintf(tgzFileTargetDirectory, 255, "./%s/", simplePayloadFile);

	/* Delete Any existing file or matching directory at target*/
	twDirectory_DeleteDirectory(tgzFileTargetDirectory);
	twDirectory_DeleteFile(tgzFileSourcePath);

	TEST_ASSERT_TRUE(TW_OK != twApi_TgzExtractFile(tgzFileSourcePath));

	/* Verify three files are not present */
	snprintf(fileNameBuffer, 255, "./%s/file1.txt", simplePayloadFile);
	TEST_ASSERT_FALSE(twDirectory_FileExists(fileNameBuffer));
	snprintf(fileNameBuffer, 255, "./%s/file2.txt", simplePayloadFile);
	TEST_ASSERT_FALSE(twDirectory_FileExists(fileNameBuffer));
	snprintf(fileNameBuffer, 255, "./%s/file3.txt", simplePayloadFile);
	TEST_ASSERT_FALSE(twDirectory_FileExists(fileNameBuffer));

	/* Clean Up */
	twDirectory_DeleteDirectory(tgzFileTargetDirectory);
	twDirectory_DeleteFile(tgzFileSourcePath);
}

