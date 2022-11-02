#include "twApi.h"
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"
#include "twApiStubs.h"
#include "twThreads.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif


/** function prototype for twOfflineMsgStore internal helper functions **/
int getOfflineMsgListCount();
char isOfflineMsgListEnabled();
char * getOfflineMsgDirectory();
uint64_t getOfflineMsgSize();

TEST_GROUP(OfflineMsgStoreIntegration);

void test_OfflineMsgStoreIntegration_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

/* global message buffer to be used when formatting assert messages */
#define ASSERT_FAIL_MESSAGE_MAX_SIZE 512
char assert_fail_message[ASSERT_FAIL_MESSAGE_MAX_SIZE];
char offline_message_path[FILE_NAME_BUF_SIZE];
char offline_message_file[FILE_NAME_BUF_SIZE];
char offline_message_path_different[FILE_NAME_BUF_SIZE];
char offline_message_file_different[FILE_NAME_BUF_SIZE];

/* global variables to predict message size and number of chunks when calculating expected offline message store size and list count */
uint64_t	predict_bin_size;		/* global count for expected offline message store size, used when writing multiple messages to the offline message store on disk */
uint64_t	predict_size;			/* global count for individual message size, used when writing a single message to the offline message store on disk */
int			predict_list_count;		/* global count for expected entries in the offline message list, used when writing multiple messages to the offline message store in ram */
int			predict_chunk_count;	/* global count for expected chunks that will be stored per message used when writing a single message to the offline message store in ram */

/* global variables for api and message handling threads */
twThread *apiThreadOfflineMsgStoreIntegration = NULL;
twThread *workerThreadsOfflineMsgStoreIntegration[NUM_WORKER_THREADS] = {NULL};

/***** START: helper function prototypes, enums, and structs: USE THESE IN INTEGRATION TESTS *****/

/* integration tests modes available when testing the offline message store write functions */
typedef enum OfflineStoreMemoryMode {
	MEMORY_MODE_IN_RAM, /** write a single message of configurable size to the message store then flush **/
	MEMORY_MODE_ON_DISK,  /** write messages of configurable size until the message store returns full **/
}OfflineStoreMemoryMode;

/* integration tests modes available when testing the offline message store write functions */
typedef enum OfflineStoreWriteMode {
	WRITE_MODE_SINGLE_MESSAGE, /** write a single message of configurable size to the message store then flush **/
	WRITE_MODE_FILL_ENTIRE_STORE,  /** write messages of configurable size until the message store returns full **/
}OfflineStoreWriteMode;

/* integration test modes available when testing the offline message store flush functionality */
typedef enum OfflineStoreFlushMode {
	FLUSH_MODE_DISCONNECT_CONNECT_FLUSH, /** disconect from server, write to offline message store, connect, then fully flush the store **/
	FLUSH_MODE_DELETE_CREATE_FLUSH, /** disconect from server, write to offline message store, delete the api, re-initialize the api, connect then fully flush the store **/
}OfflineStoreFlushMode;

/* internal test struct used to pass info related to write then flush tests */
typedef struct test_write_flush {
	char * path;
	OfflineStoreMemoryMode memory_mode;
	OfflineStoreWriteMode write_mode;
	OfflineStoreFlushMode flush_mode;
}test_write_flush;

#define TEST_RELATIVE_CONFIG_SIZES 6
/* struct that can hold the convenience values for size calculations based on config values */
typedef struct relative_config_values {
	uint32_t max_messages_lesser;
	uint32_t max_messages_equal;
	uint32_t max_messages_greater;
	union {
		/* using a union so the sizes can be addressed individually OR accessed through the array (loop) */
		struct {
			/* struct to hold the individually addressable size values */
			uint32_t chunk_size_lesser;
			uint32_t chunk_size_equal;
			uint32_t chunk_size_greater;
			uint32_t max_message_size_lesser;
			uint32_t max_message_size_equal;
			uint32_t max_message_size_greater;
		} sizes;
		uint32_t size_array[TEST_RELATIVE_CONFIG_SIZES];
	} values;
}relative_config_values;

/* global test singleton to assist with relative config values */
relative_config_values * test_relative_config_values = NULL;

/* checks file size on disk against input size */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineFileSize(const char * store_path, uint64_t expected_size);

/* checks if the file is accesexistssible */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineFileExists(const char * store_path, char does_exist);

/* checks if the file accessible */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineFileAccessible(const char * store_path, char is_accessible);

/* checks file size on disk against input size */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineListCount(int expected_count) ;

/* checks if the file is accesexistssible */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineListExists(char does_exist);

/* quick helper function to determine if the current tw_offline_msg_store->offlineMsgFile exists on disk */
void twAssertCurrentOfflineFileExists();

/* quick helper function to determine if the current tw_offline_msg_store->offlineMsgFile does NOT exist on disk */
void twAssertCurrentOfflineFileDoesNotExist();

/* main write flush function */
/* this will test all possible size combinations detailed in the test plan comment */
void twAssertOfflineMsgWriteFlushTest(test_write_flush * test_write_flush_data);

/***** END: helper functions prototypes *****/

/***** START: internal Offline message store test methods: DO NOT USE DIRECTLY *****/

/* request types that can be used to check info about the offline message file */
typedef enum OfflineFileInfoRequest {
	OFFLINE_MSG_FILE_CHECK_ACCESS=0, /**< checks whether the file is accessible **/
	OFFLINE_MSG_FILE_CHECK_EXIST, /**< checks whether the file exists**/
	OFFLINE_MSG_FILE_CHECK_SIZE,  /**< checks the file size **/
	OFFLINE_MSG_FILE_CHECK_NOT_EMPTY  /**< checks the file size to see if it is not empty **/
}OfflineFileInfoRequest;

/* request types that can be used to check info about the offline message list */
typedef enum OfflineListInfoRequest {
	OFFLINE_MSG_LIST_CHECK_EXIST=0, /** check if the list exists **/
	OFFLINE_MSG_LIST_CHECK_COUNT, /** check the list count **/
	OFFLINE_MSG_LIST_CHECK_NOT_EMPTY /** check the list count **/
}OfflineListInfoRequest;

/* struct to comminicate file info relevant to the offline message tests */
/* path? does it exist? what is its size? is it accessible? */
/* note: struct should NOT own the path pointer */
typedef struct offline_store_info {
	const char * path;
	char exists;
	uint64_t size;
	char accessible;
} offline_store_info;

int STUB_twOfflineMsgStore_HandleRequest (twMessage **msg, twWs * ws, enum OfflineRequest request_type) {

	char byte;
	char stream_added = FALSE;
	char header[MSG_HEADER_SIZE + MULTIPART_MSG_HEADER_SIZE];
	twStream * s = NULL;
	twStream * bodyStream = NULL;
	size_t length = 0;
	size_t bodyBytesRemaining;
	size_t tmp;
	size_t numChunks = 0;
	size_t chunkNumber = 1;
	int res = 0;
	char headerSize = MSG_HEADER_SIZE;
	size_t effectiveChunkSize = twcfg_pointer->message_chunk_size - MSG_HEADER_SIZE - MULTIPART_MSG_HEADER_SIZE;
	if (OFFLINE_MSG_STORE_WRITE == request_type) {
		/* reset global test sizes */
		predict_size = 0;
		predict_chunk_count = 0;

		/* check params */
		if (!(*msg) || !(*msg)->body || !ws) {
			TW_LOG(TW_ERROR, "STUB_twOfflineMsgStore_HandleRequest: invalid params");
			return TW_INVALID_PARAM;
		}

		/* Get the length */
		if ((*msg)->type == TW_REQUEST) {
			length = ((twRequestBody *)((*msg)->body))->length;
			effectiveChunkSize -= 2 + strlen(((twRequestBody *)(*msg)->body)->entityName);
		} else {
			/* the test is not able to handle non-requests written to the offline message store */
			TW_LOG(TW_ERROR,"STUB_twOfflineMsgStore_HandleRequest: Unknown message code: %d", (*msg)->code);
			return TW_INVALID_MSG_TYPE;
		}

		/* Create a new stream for the body */
		bodyStream = twStream_Create();
		if (!bodyStream) {
			TW_LOG(TW_ERROR, "STUB_twOfflineMsgStore_HandleRequest: Error allocating stream");
			return TW_ERROR_ALLOCATING_MEMORY;
		}

		/* Create the header binary representation */
		header[0] = (*msg)->version;
		header[1] = (char)(*msg)->code;
		tmp = (*msg)->requestId;
		swap4bytes((char *)&tmp);
		memcpy(&header[2], (char *)&tmp, 4);
		tmp = (*msg)->endpointId;
		swap4bytes((char *)&tmp);
		memcpy(&header[6], (char *)&tmp, 4);
		if (ws->sessionId && (*msg)->type != TW_AUTH) (*msg)->sessionId = ws->sessionId;
		tmp = (*msg)->sessionId;
		swap4bytes((char *)&tmp);
		memcpy(&header[10], (char *)&tmp, 4);

		if (length + MSG_HEADER_SIZE > twcfg_pointer->message_chunk_size) (*msg)->multipartMarker = TRUE;
		header[14] = (*msg)->multipartMarker;

		/* Add the beginning of the body */
		numChunks = length/effectiveChunkSize + 1;
		if ((*msg)->multipartMarker) {
			unsigned char chunkInfo[6];
			chunkInfo[0] = (unsigned char)(chunkNumber / 256);
			chunkInfo[1] = (unsigned char)(chunkNumber % 256);
			chunkInfo[2] = (unsigned char)(numChunks / 256);
			chunkInfo[3] = (unsigned char)(numChunks % 256);
			chunkInfo[4] = (unsigned char)(twcfg_pointer->message_chunk_size / 256);
			chunkInfo[5] = (unsigned char)(twcfg_pointer->message_chunk_size % 256);
			memcpy(&header[MSG_HEADER_SIZE], chunkInfo, 6);
			headerSize += MULTIPART_MSG_HEADER_SIZE;
		}
		if ((*msg)->type == TW_REQUEST) {
			twRequestBody_ToStream((twRequestBody *)(*msg)->body, bodyStream);
		}

		/* Start writing the message to offline message store */
		bodyBytesRemaining = length;
		while (chunkNumber <= numChunks) {
			/* Create a new stream for the body */
			size_t size = effectiveChunkSize;
			s = twStream_Create();
			if (!s) {
				TW_LOG(TW_ERROR, "STUB_twOfflineMsgStore_HandleRequest: Error allocating stream");
				twStream_Delete(bodyStream);
				return TW_ERROR_ALLOCATING_MEMORY;
			}
			res = twStream_AddBytes(s, header, headerSize);
			if (bodyBytesRemaining <= effectiveChunkSize) size = bodyBytesRemaining;
			if ((*msg)->multipartMarker) {
				/* Adjust the chunk number */
				s->data[MSG_HEADER_SIZE] = (unsigned char)(chunkNumber/256);
				s->data[MSG_HEADER_SIZE + 1] = (unsigned char)(chunkNumber%256);
				/* If this is a request we also need to add the entity info unless this is the first chunk which already has it */
				if (!res && ((*msg)->code == TWX_GET || (*msg)->code == TWX_PUT || (*msg)->code == TWX_POST || (*msg)->code == TWX_DEL) && chunkNumber != 1) {
					byte = (char)((twRequestBody *)(*msg)->body)->entityType;
					res = twStream_AddBytes(s, &byte, 1);
					stringToStream(((twRequestBody *)(*msg)->body)->entityName, s);
				}
			}
			/* Add the data */
			if (!res) res = twStream_AddBytes(s,&bodyStream->data[length - bodyBytesRemaining], size);
			if (res) {
				twStream_Delete(s);
				twStream_Delete(bodyStream);
				return res;
			}
			if (getOfflineMsgSize() + strlen(PERSISTED_MSG_SEPARATOR) + sizeof(s->length) + s->length < twcfg_pointer->offline_msg_queue_size) {
				/* add length for the separator the separator */
				/* (TW_FWRITE(PERSISTED_MSG_SEPARATOR, 1, sepLength, f) != sepLength) */
				predict_size += strlen(PERSISTED_MSG_SEPARATOR);

				/* add length for the length */
				/*(TW_FWRITE(&s->length, 1, sizeof(s->length), f) != sizeof(s->length)) */
				predict_size += sizeof(s->length);

				/* add length for the stream bytes */
				/* (TW_FWRITE(s->data, 1, s->length, f) != s->length) */
				predict_size += s->length;

				/* increment chunk count */
				predict_chunk_count ++;

				/* cleanup stream */
				twStream_Delete(s);
				s = NULL;

				/* update counters and continue */
				bodyBytesRemaining -= size;
				chunkNumber++;
			} else {
				/* no more bytes will be added, break from the loop */
				break;
			}
		}

		if (s && !stream_added) {
			twStream_Delete(s);
			s = NULL;
		}

		twStream_Delete(bodyStream);
		/* Reset the multipart marker so deletting the message doesn't get confused */
		(*msg)->multipartMarker = FALSE;

		/* increment total predicted sizes */
		predict_bin_size += predict_size;
		predict_list_count += predict_chunk_count;
	}
	/* execute the actual task */
	return twOfflineMsgStore_HandleRequest(msg, ws, request_type);
}

/*delete test structure */
void deleteTestWriteFlushData(test_write_flush * test_write_flush_data) {
	if (test_write_flush_data->path) TW_FREE(test_write_flush_data->path);
	TW_FREE(test_write_flush_data);
	return;
}

/* update test structure */
/* a null path can be passed in to keep the same path, but all other inputs are required */
void updateTestWriteFlushData(test_write_flush * test_write_flush_data, const char * path, OfflineStoreMemoryMode memory_mode, OfflineStoreWriteMode write_mode, OfflineStoreFlushMode flush_mode) {
	/* check memory alloc */
	if (test_write_flush_data) {
		/* set params */
		if (path) {
			/* free old value, then set the new one */
			if (test_write_flush_data->path) TW_FREE(test_write_flush_data->path);
			test_write_flush_data->path = duplicateString(path);
		}
		test_write_flush_data->memory_mode = memory_mode;
		test_write_flush_data->write_mode = write_mode;
		test_write_flush_data->flush_mode = flush_mode;
	}
}

/* create test structure */
test_write_flush * createTestWriteFlushData() {
	return (test_write_flush*)TW_CALLOC(sizeof(test_write_flush), 1);
}

/* internal helper function to delete test_relative_config_values singleton */
void deleteTestRelativeConfigValues() {
	/* check if singleton already exists */
	if (test_relative_config_values) {
		TW_FREE(test_relative_config_values);
		test_relative_config_values = NULL;
	}
}

/* internal helper function to update test_relative_config_values singleton */
int updateTestRelativeConfigValues() {
	int res = TW_OK;

	/* check if singleton already exists */
	if (test_relative_config_values) {
		/* check if twcfg values exist */
		if (twcfg_pointer) {
			test_relative_config_values->values.sizes.chunk_size_lesser = twcfg_pointer->message_chunk_size/2;
			test_relative_config_values->values.sizes.chunk_size_equal = twcfg_pointer->message_chunk_size;
			test_relative_config_values->values.sizes.chunk_size_greater = twcfg_pointer->message_chunk_size*2;
			test_relative_config_values->max_messages_lesser = twcfg_pointer->max_messages/2;
			test_relative_config_values->max_messages_equal = twcfg_pointer->max_messages;
			test_relative_config_values->max_messages_greater = twcfg_pointer->max_messages*2;
			test_relative_config_values->values.sizes.max_message_size_lesser = twcfg_pointer->max_message_size/2;
			test_relative_config_values->values.sizes.max_message_size_equal = twcfg_pointer->max_message_size;
			test_relative_config_values->values.sizes.max_message_size_greater = twcfg_pointer->max_message_size*2;

			res = TW_OK;
		} else {
			TW_LOG(TW_ERROR,"twcfg_pointer not initialized");
			res = TW_UNKNOWN_ERROR;
		}
	} else {
		TW_LOG(TW_ERROR,"test_relative_config_values not initialized");
		res = TW_UNKNOWN_ERROR;
	}

	return res;
}

/* internal helper function to create test_relative_config_values singleton */
int createTestRelativeConfigValues() {
	int res = TW_OK;

	/* check if singleton already exists */
	if (!test_relative_config_values) {
		test_relative_config_values = (relative_config_values*)TW_CALLOC(sizeof(relative_config_values), 1);
		if (test_relative_config_values) {
			res = updateTestRelativeConfigValues();
		} else {
			res = TW_ERROR_ALLOCATING_MEMORY;
		}
	}

	if (res) {
		/* if there was an error during the update, delete the singleton so we can try again */
		TW_LOG(TW_ERROR,"error initializing test_relative_config_values: %d, deleting memory in case another init attempt will be made", res);
		deleteTestRelativeConfigValues();
	}

	return res;
}

/* returns TW status, and sets store_info outparam */
int get_offline_file_info(offline_store_info ** store_info) {
	uint64_t f_size = 0;
	DATETIME l_mod;
	char is_dir = FALSE;
	char rd_only = FALSE;
	int err = 0;
	offline_store_info * tmp_store_info = NULL;

	/* check inputs */
	if (!store_info || !*store_info || !(*store_info)->path) {
		err = TW_INVALID_PARAM;
	} else {
		tmp_store_info = *store_info;
		err = twDirectory_GetFileInfo((char*)(tmp_store_info->path), &f_size, &l_mod, &is_dir, &rd_only);

		/* return -1 on error, otherwise return file size */
		if (err == 2) {
			TW_LOG(TW_ERROR,"test_offline_file: file not found: %s", tmp_store_info->path);
			tmp_store_info->accessible = TRUE;
			tmp_store_info->exists = FALSE;
			tmp_store_info->size = 0;
			err = TW_FILE_NOT_FOUND;
		} else if (err) {
			TW_LOG(TW_ERROR,"test_offline_file: error getting file info for %s", tmp_store_info->path);
			tmp_store_info->accessible = FALSE;
			tmp_store_info->exists = FALSE;
			tmp_store_info->size = 0;
			err = TW_UNKNOWN_ERROR;
		} else {
			tmp_store_info->accessible = TRUE;
			tmp_store_info->exists = TRUE;
			tmp_store_info->size = f_size;
			err = TW_OK;
		}
	}
	return err;
}

/* checks to see if the offline message file exists on disk */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int test_offline_file(const char * store_path, void * value, OfflineFileInfoRequest type) {
	offline_store_info * store_info = NULL;
	int result = TW_UNKNOWN_ERROR;
	uint64_t * size = NULL;
	char * exists = NULL;
	char * accessible = NULL;

	/* check inputs */
	if (!store_path || !value) {
		/* null check */
		TW_LOG(TW_ERROR,"test_offline_file: invalid store_path (%s) or null value (%s)", store_path ? store_path : "NULL", value ? "NOT NULL" : "NULL");
		result = TW_INVALID_PARAM;
	} else {

		/* allocate struct */
		store_info = (offline_store_info*) TW_MALLOC(sizeof(offline_store_info));
		if (!store_info) {
			TW_LOG(TW_ERROR,"test_offline_file: could not allocate store_info structure");
			result = TW_ERROR_ALLOCATING_MEMORY;
		} else {
			/* set path */
			store_info->path = store_path;
			/* get file info */
			result = get_offline_file_info(&store_info);
			/* determine request type */
			switch (type) {
			case OFFLINE_MSG_FILE_CHECK_SIZE:
				/* extract uint64_t value from intput */
				size = (uint64_t *)value;
				if (*size == store_info->size) {
					/* success */
					result = TW_OK;
				} else {
					/* failure */
					TW_LOG(TW_ERROR,"test_offline_file: size (%d) does not match expected size (%d)", store_info->size, *size);
					/* set error based on result value */
					if(!result) result = TW_UNKNOWN_ERROR;
				}
				break;
			case OFFLINE_MSG_FILE_CHECK_NOT_EMPTY:
				if (0 < store_info->size) {
					/* success */
					result = TW_OK;
				} else {
					/* failure */
					TW_LOG(TW_ERROR,"test_offline_file: file is empty", store_info->size);
					/* set error based on result value */
					if(!result) result = TW_UNKNOWN_ERROR;
				}
				break;
			case OFFLINE_MSG_FILE_CHECK_EXIST:
				/* extract boolean value from input */
				exists = (char*)value;
				if (*exists == store_info->exists) {
					/* success */
					result = TW_OK;
				} else {
					/* failure */
					TW_LOG(TW_ERROR,"test_offline_file: store_info->exists (%s) does not match expected exist value (%s)", store_info->exists ? "TRUE" : "FALSE", *exists ? "TRUE" : "FALSE");
					/* set error based on result value */
					if(!result) result = TW_UNKNOWN_ERROR;
				}
				break;
			case OFFLINE_MSG_FILE_CHECK_ACCESS:
				/* extract boolean value from input */
				accessible = (char*)value;
				if (*accessible == store_info->accessible) {
					/* success */
					result = TW_OK;
				} else {
					/* failure */
					TW_LOG(TW_ERROR,"test_offline_file: store_info->accessible (%s) does not match expected size (%s)", store_info->accessible ? "TRUE" : "FALSE", *accessible ? "TRUE" : "FALSE");
					/* set error based on result value */
					if(!result) result = TW_UNKNOWN_ERROR;
				}
				break;
			default:
				/* failure */
				TW_LOG(TW_ERROR,"test_offline_file: check service inputs, there appears to be nothing to check, type: %d", type);
				result = TW_UNKNOWN_ERROR;
			}
			TW_FREE(store_info);
		}
	}
	return result;
}

int test_offline_list(void * value, OfflineListInfoRequest type) {
	int result;
	char actual_exists;
	char * expected_exists = (char*)value;
	int actual_count;
	int * expected_count = (int*)value;

	/* check inputs */
	if (!value) {
		/* null check */
		TW_LOG(TW_ERROR,"test_offline_list: invalid input: value (%s)", value ? "NOT NULL" : "NULL");
		result = TW_INVALID_PARAM;
	} else {
		switch (type) {
		case OFFLINE_MSG_LIST_CHECK_EXIST:
			/* extract boolean value from input */
			expected_exists = (char*)value;

			/* get if list exists */
			actual_exists = isOfflineMsgListEnabled();

			if (*expected_exists == actual_exists) {
				/* success */
				result = TW_OK;
			} else {
				/* failure */
				TW_LOG(TW_ERROR,"test_offline_list: store_info->exists (%s) does not match expected exist value (%s)", actual_exists ? "TRUE" : "FALSE", *expected_exists ? "TRUE" : "FALSE");
				result = TW_UNKNOWN_ERROR;
			}
			break;
		case OFFLINE_MSG_LIST_CHECK_COUNT:
			/* extract int value from input */
			expected_count = (int*)value;

			/* get list count */
			actual_count = getOfflineMsgListCount();

			/* check values */
			if (*expected_count == actual_count) {
				/* success */
				result = TW_OK;
			} else {
				/* failure */
				TW_LOG(TW_ERROR,"test_offline_list: store_info->exists (%s) does not match expected exist value (%s)", actual_count ? "TRUE" : "FALSE", *expected_count ? "TRUE" : "FALSE");
				result = TW_UNKNOWN_ERROR;
			}
			break;
		case OFFLINE_MSG_LIST_CHECK_NOT_EMPTY:

			/* get list count */
			actual_count = getOfflineMsgListCount();

			/* check values */
			if (0 < actual_count) {
				/* success */
				result = TW_OK;
			} else {
				/* failure */
				TW_LOG(TW_ERROR,"test_offline_list: offline message list is empty");
				result = TW_UNKNOWN_ERROR;
			}
			break;
		default:
			/* failure */
			TW_LOG(TW_ERROR,"test_offline_list: check service inputs, there appears to be nothing to check, type: %d", type);
			result = TW_UNKNOWN_ERROR;
		}
	}

	return result;
}

/* utility function to send a message of a specific size */
int test_send_property_write (uint32_t msg_size) {
	int res = 0;
	int i = 0;
	twPrimitive * value = NULL;
	char * stringValue = NULL;

	/* allocate the string */
	/* use calloc to ensure null terminator */
	stringValue = (char*)TW_CALLOC(msg_size, 1);

	/* populate string with chars */
	memset(stringValue, 'a', msg_size-1);

	/* write a single message to the offline message store with a string of size msg_size*/
	value = twPrimitive_CreateFromString(stringValue, FALSE);
	res = twApi_WriteProperty(TW_THING, "OfflineMessageStoreIntegrationTestThing", "OfflineMsgStore_TestProperty", value, DEFAULT_MESSAGE_TIMEOUT, FALSE);

	/* cleanup result */
	if(value) {
		twPrimitive_Delete(value);
		value = NULL;
	}

	return res;
}

/* utility function that disconnects from an already conencted api and writes messages to the offline msg store */
void test_disconnect_then_write (test_write_flush * test_write_flush_data, uint32_t msg_size) {
	int res = TW_OK;
	char timed_out = FALSE;
	uint64_t expected_size = 0;
	int loop_count = 0;
	int tmp = 0;

	/* verify input */
	TEST_ASSERT_TRUE_MESSAGE(test_write_flush_data, "test_disconnect_then_write: error accessing test_write_flush_data structure");

	/* verify connection */
	TEST_ASSERT_TRUE_MESSAGE(twApi_isConnected(), "test_disconnect_then_write: Error: api was disconnected before test started");

	/* verify empty message store */
	if ( test_write_flush_data->memory_mode == MEMORY_MODE_IN_RAM) {
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineListCount(0), "test_disconnect_then_write: offline message store is not empty");
	} else if (test_write_flush_data->memory_mode == MEMORY_MODE_ON_DISK) {
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineFileSize(test_write_flush_data->path, 0), "test_disconnect_then_write: offline message store is not empty");
	}
	/* reset predicted offline list and bin sizes just in case */
	predict_bin_size = 0;
	predict_list_count = 0;

	/* disconnect */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Disconnect("disconnect for offline msg test"), "test_disconnect_then_write: error disconnecting api");

	/* write messages depending on input params */
	switch (test_write_flush_data->write_mode) {
	case WRITE_MODE_SINGLE_MESSAGE:
		/* write a single message to the store */
		res = test_send_property_write(msg_size);

		/* verify result */
		if (TW_WROTE_TO_OFFLINE_MSG_STORE == res) {
			/* verify the offline message store is not empty */
			if ( test_write_flush_data->memory_mode == MEMORY_MODE_IN_RAM) {
				TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineListCount(predict_chunk_count), "test_disconnect_then_write: offline message store is empty");
			} else if (test_write_flush_data->memory_mode == MEMORY_MODE_ON_DISK) {
				TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineFileSize(test_write_flush_data->path, predict_size), "test_disconnect_then_write: offline message store is empty");
			}
		} else if (TW_PRECONDITION_FAILED == res) {
			/* verify that this is an error related to max message size */
			TEST_ASSERT_TRUE_MESSAGE(msg_size >= twcfg_pointer->max_message_size, "test_disconnect_then_write: precondition failed");
		} else {
			/* if the res is not a succesful write to the message store, verify that the store is full and that we did not experience a different error */
			TEST_ASSERT_TRUE_MESSAGE(TW_ERROR_OFFLINE_MSG_STORE_FULL == res, "test_disconnect_then_write: error writing single message to offline message store");
		}

		break;
	case WRITE_MODE_FILL_ENTIRE_STORE:
		/* verify that one message will not fill the entire store */
		if (msg_size >= twcfg_pointer->offline_msg_queue_size || msg_size >= twcfg_pointer->max_message_size) {
			/* get a result anyway to make sure the test does not crash */
			res = test_send_property_write(msg_size);
			if (TW_PRECONDITION_FAILED == res) {
				TW_LOG(TW_TRACE,"msg_size is greater than the max message size: %d", msg_size, twcfg_pointer->max_message_size);
			} else if (TW_ERROR_OFFLINE_MSG_STORE_FULL == res) {
				TW_LOG(TW_TRACE,"msg_size is greater than the offline msg queue size: %d", msg_size, twcfg_pointer->offline_msg_queue_size);
			}

		} else {
			/* fill entire message store */
			/* TODO: add timeout */
			while (TW_ERROR_OFFLINE_MSG_STORE_FULL != res && !timed_out) {
				res = test_send_property_write(msg_size);
				if (res == TW_WROTE_TO_OFFLINE_MSG_STORE) {
					/* success */
					loop_count++;
				} else {
					TEST_ASSERT_EQUAL_MESSAGE(TW_ERROR_OFFLINE_MSG_STORE_FULL, res, "test_disconnect_then_write: error writing many messages to offline message store");
				}
			}

			/* verify the offline message store is not empty */
			if ( test_write_flush_data->memory_mode == MEMORY_MODE_IN_RAM) {
				TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineListCount(predict_list_count), "test_disconnect_then_write: offline message store count does not match expected count");
			} else if (test_write_flush_data->memory_mode == MEMORY_MODE_ON_DISK) {
				TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineFileSize(test_write_flush_data->path, predict_bin_size), "test_disconnect_then_write: offline message store is empty");
			}}
		break;
	default:
		TW_LOG(TW_ERROR,"test_disconnect_write: incorrect input option: %d", test_write_flush_data->write_mode);
		res = TW_INVALID_PARAM;
	}
}

/* utility function that connects an already disconnected api then flushes the offline message store data */
void test_connect_then_write(test_write_flush * test_write_flush_data, uint32_t msg_size) {

	int res = 0;

	/* verify input */
	TEST_ASSERT_TRUE_MESSAGE(test_write_flush_data, "test_connect_then_write: error accessing test_write_flush_data structure");

	/* verify connection */
	TEST_ASSERT_FALSE_MESSAGE(twApi_isConnected(), "test_connect_then_write: Error: api was disconnected before test started");

	/* connect */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES), "test_connect_then_write: could not connect to server");

	/* send a successful message to confirm connection */
	res = test_send_property_write(msg_size);
	if (res == TW_PRECONDITION_FAILED) {
		/* if the precondition failed, make sure this was expected because of the max message size and not due to something else */
		TEST_ASSERT_TRUE_MESSAGE(msg_size >= twcfg_pointer->max_message_size,"test_connect_then_write: precondition failed");
	} else {
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK, test_send_property_write(msg_size), "test_connect_then_write: could not send message after connection");
	}

	/* verify empty message store */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineFileSize(test_write_flush_data->path, 0), "test_connect_then_write: offline message store is not empty");

	/* reset predicted offline list and bin sizes just in case */
	predict_bin_size = 0;
	predict_list_count = 0;
}

/* utility function that enables the message store based on the test params */
void test_enable_msg_store(test_write_flush * test_write_flush_data){
	switch (test_write_flush_data->memory_mode){
	case MEMORY_MODE_IN_RAM:
		/* enable in ram offline message store */
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK, enableOfflineMsgStore(TRUE, FALSE), "test_enable_msg_store: error: could not enable in RAM message store");
		break;

	case MEMORY_MODE_ON_DISK:
		/* enable on disk offline message store */
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK, enableOfflineMsgStore(TRUE, TRUE), "test_enable_msg_store: error: could not enable on disk message store");
		enableOfflineMsgStore(TRUE, TRUE);
		break;

	default:
		/* invalid option given */
		TEST_FAIL_MESSAGE("twAssertOfflineMsgWriteFlushTest: invalid memory mode selected");
		break;
	}
}

/* utility function that restarts the api, message handling threads, and re-enables the offline msg store */
void test_restart_api(test_write_flush * test_write_flush_data) {

}

/***** END: internal Offline message store test methods*****/


TEST_SETUP(OfflineMsgStoreIntegration) {
	int res = 0;
	int i = 0;
	char *currentDirectory = NULL;
	eatLogs();

	currentDirectory = getCurrentDirectory();
	/* use etc file as offline message store */
	if(strlen(currentDirectory) + strlen("thingworks/offline_msgs.bin") >= FILE_NAME_BUF_SIZE) {
		TEST_FAIL_MESSAGE("offline message directory path too large");
	}

	snprintf(offline_message_path,FILE_NAME_BUF_SIZE, "%s%s",currentDirectory,"/thingworx/");
	snprintf(offline_message_path_different,FILE_NAME_BUF_SIZE, "%s%s",currentDirectory,"/thingworks/");

	offline_message_path[strlen(currentDirectory) + strlen("/thingworx/")] = NULL; /* adding null terminator */
	offline_message_path_different[strlen(currentDirectory) + strlen("/thingworks/")] = NULL; /* adding null terminator */

	snprintf(offline_message_file,FILE_NAME_BUF_SIZE, "%s%s",offline_message_path,"offline_msgs.bin");
	snprintf(offline_message_file_different,FILE_NAME_BUF_SIZE, "%s%s",offline_message_path_different,"offline_msgs.bin");

	offline_message_file[strlen(offline_message_path) + strlen("offline_msgs.bin")] = NULL; /* adding null terminator */
	offline_message_file_different[strlen(offline_message_path_different) + strlen("offline_msgs.bin")] = NULL; /* adding null terminator */

	twDirectory_DeleteDirectory(offline_message_path);
	twDirectory_DeleteDirectory(offline_message_path_different);
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory(offline_message_path), "failed to create offline msg store test directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory(offline_message_path_different), "failed to create offline msg store test directory");

	/* ensure offline message file is accessible */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineFileAccessible(offline_message_file, TRUE), "TEST_SETUP:OfflineMsgStoreIntegration: Offline message store file is not accessible");

	/* delete any previous message store binaries */
	/* accept "file does not exist"(2) as a result */
	res = twDirectory_DeleteFile(offline_message_file);
	TEST_ASSERT_TRUE_MESSAGE(TW_OK == res || 2 == res, "TEST_SETUP:OfflineMsgStoreIntegration: Error deleting old message store");
	res = twDirectory_DeleteFile(offline_message_file_different);
	TEST_ASSERT_TRUE_MESSAGE(TW_OK == res || 2 == res, "TEST_SETUP:OfflineMsgStoreIntegration: Error deleting different old message store");

	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;

	/* set offline msg store file */
	twcfg_pointer->offline_msg_store_dir = offline_message_path;

	/* init or set relative config values */
	if (test_relative_config_values) {
		deleteTestRelativeConfigValues();
	}
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, createTestRelativeConfigValues(), "TEST_SETUP:OfflineMsgStoreIntegration: Error creating test config values");

	/* initialize */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegration_callback, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	/* set connection params */
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();

	/* attempt to connect, then check connection */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	apiThreadOfflineMsgStoreIntegration = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		workerThreadsOfflineMsgStoreIntegration[i] = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);
	}

	/* import generic thing entity for testing */
	importEntityFileFromEtc("Things_OfflineMessageStoreIntegrationTestThing.xml");

	/* set stub for offline message handling to assist in size calculations  */
	twApi_stub->twOfflineMsgStore_HandleRequest = STUB_twOfflineMsgStore_HandleRequest;
}

TEST_TEAR_DOWN(OfflineMsgStoreIntegration) {
	int i = 0;
	twStubs_Reset();
	/* disconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("ServiceIntegrationTests Complete"));
	TEST_ASSERT_FALSE(twApi_isConnected());

	/* delete mh threads */
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		twThread_Delete(workerThreadsOfflineMsgStoreIntegration[i]);
	}

	/* delete api threads */
	twThread_Delete(apiThreadOfflineMsgStoreIntegration);

	/* delete api */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	deleteTestRelativeConfigValues();
}

/* test plan
*  enabled
*  disabled
*	In RAM msg store
*	On Disk msg store
*		Write single message
*		Fill entire store
*           flush model mode 1 (init sdk->disconnect->write to store->connect->flush store)
*			flush model mode 2 (init sdk->disconnect->write to store->tear down sdk->init sdk->connect->flush store)
*			flush partial mode 1 (disconnect during flush)
*			flush partial mode 2 (tear down api during flush)
*			    Write small messages (< chunk size)
*			    Write chunk messages (== chunk size)
*			    Write large messages (> chunk size)
*			    Write extra large messages (> max_message_size)
*				       twcfg.offline_msg_store_queue_size < chunk size,
*				       twcfg.offline_msg_store_queue_size == chunk size,
*				       twcfg.offline_msg_store_queue_size > chunk size,
*				       twcfg.offline_msg_store_queue_size < max message size,
*				       twcfg.offline_msg_store_queue_size == max message size,
*				       twcfg.offline_msg_store_queue_size > max message size
*
*				   # of messages written to store < max_messages
*				   # of messages written to store == max_messages,
*				   # of messages written to store > max_messages
*/

/* the _Dynamic tests will test multiple test cases in the test tree above, see individual test definition for more info */
TEST_GROUP_RUNNER(OfflineMsgStoreIntegration) {
	RUN_TEST_CASE(OfflineMsgStoreIntegration, twOfflineMsgStore_MsgStore_Disabled);
	RUN_TEST_CASE(OfflineMsgStoreIntegration, twOfflineMsgStore_ChangePath_Success);
	RUN_TEST_CASE(OfflineMsgStoreIntegration, twOfflineMsgStore_ChangePath_Error);

#ifdef ENABLE_IGNORED_TESTS
	RUN_TEST_CASE(OfflineMsgStoreIntegration, twOfflineMsgStore_MsgStore_Test_Dynamic_On_Disk);
	RUN_TEST_CASE(OfflineMsgStoreIntegration, twOfflineMsgStore_MsgStore_Test_Dynamic_In_Ram);
#endif

}

/* write to offline msg store and flush - disabled*/
TEST(OfflineMsgStoreIntegration, twOfflineMsgStore_MsgStore_Disabled) {
	if (OFFLINE_MSG_STORE == 0) {
		/* create result infotable to use while making requests */
		twPrimitive * result = NULL;

		/* make sure the offline message store file does not exist */
		twAssertCurrentOfflineFileDoesNotExist();

		/* disconnect */
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Disconnect("going down for offline message test"), "Could Not disconnect");

		/* send a failed message */
		/* ensure it gets dropped */
		TEST_ASSERT_EQUAL_MESSAGE(TW_SERVICE_UNAVAILABLE, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE), "Unexpected error attempting to send message");
		if(result) {
			twPrimitive_Delete(result);
			result = NULL;
		}
	} else {
		/* if the OFFLINE_MSG_STORE is not 0, then the build has defined the message store as on or invalid, therefore ignore these tests */
		TEST_IGNORE_MESSAGE("Ignoring disabled offline message store tests because the offline message store has been defined during the build");
	}
}

/* test message store through a variety of cases mentioned in the test plan comment*/
TEST(OfflineMsgStoreIntegration, twOfflineMsgStore_MsgStore_Test_Dynamic_On_Disk) {
	test_write_flush * test_write_flush_data = NULL;

	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("Ignoring until CI failures can be fixed, these do not occur locally");
	}

	if (OFFLINE_MSG_STORE == 0 || OFFLINE_MSG_STORE == 2) {

		/* create test params*/
		test_write_flush_data = createTestWriteFlushData();
		TEST_ASSERT_TRUE_MESSAGE(test_write_flush_data, "Could not create test data structure ");

		/*
		* test offline messages store on disk writes and flushes
		* using only one message of variable size
		* while keeping the api running the whole time
		*/
		/* update test params and run test */
		updateTestWriteFlushData(test_write_flush_data, offline_message_file, MEMORY_MODE_ON_DISK, WRITE_MODE_SINGLE_MESSAGE, FLUSH_MODE_DISCONNECT_CONNECT_FLUSH);
		twAssertOfflineMsgWriteFlushTest(test_write_flush_data);


		/*
		* test offline messages store on disk writes and flushes
		* using many messages of variable size
		* while keeping the api running the whole time
		*/
		/* update test params and run test */
		updateTestWriteFlushData(test_write_flush_data, offline_message_file, MEMORY_MODE_ON_DISK, WRITE_MODE_FILL_ENTIRE_STORE, FLUSH_MODE_DISCONNECT_CONNECT_FLUSH);
		twAssertOfflineMsgWriteFlushTest(test_write_flush_data);


		/*
		* test offline messages store on disk writes and flushes
		* using only one message of variable size
		* with a complete api delete and init before flushing (simulates restarting the SDK)
		*/
		/* update test params and run test */
		updateTestWriteFlushData(test_write_flush_data, offline_message_file, MEMORY_MODE_ON_DISK, WRITE_MODE_SINGLE_MESSAGE, FLUSH_MODE_DELETE_CREATE_FLUSH);
		twAssertOfflineMsgWriteFlushTest(test_write_flush_data);

		/*
		* test offline messages store on disk writes and flushes
		* using many messages of variable size
		* with a complete api delete and init before flushing (simulates restarting the SDK)
		*/
		/* update test params and run test */
		updateTestWriteFlushData(test_write_flush_data, offline_message_file, MEMORY_MODE_ON_DISK, WRITE_MODE_FILL_ENTIRE_STORE, FLUSH_MODE_DELETE_CREATE_FLUSH);
		twAssertOfflineMsgWriteFlushTest(test_write_flush_data);

		/* cleanup test struct memory */
		deleteTestWriteFlushData(test_write_flush_data);
	} else {
		/* if the OFFLINE_MSG_STORE is not 0 or 2, then the build has defined the message store as in RAM or invalid, therefore ignore these tests */
		TEST_IGNORE_MESSAGE("Ignoring on disk offline message tests since the build has defined the message store as in RAM");
	}
}

/* test message store through a variety of cases mentioned in the test plan comment*/
TEST(OfflineMsgStoreIntegration, twOfflineMsgStore_MsgStore_Test_Dynamic_In_Ram) {
	test_write_flush * test_write_flush_data = NULL;

	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("Ignoring until CI failures can be fixed, these do not occur locally");
	}

	if (OFFLINE_MSG_STORE == 0 || OFFLINE_MSG_STORE == 1) {
		/* create test params*/
		test_write_flush_data = createTestWriteFlushData();
		TEST_ASSERT_TRUE_MESSAGE(test_write_flush_data, "Could not create test data structure ");

		/*
		* test offline messages store in ram writes and flushes
		* using only one message of variable size
		* while keeping the api running the whole time
		*/
		/* update test params and run test */
		updateTestWriteFlushData(test_write_flush_data, offline_message_file, MEMORY_MODE_IN_RAM, WRITE_MODE_SINGLE_MESSAGE, FLUSH_MODE_DISCONNECT_CONNECT_FLUSH);
		twAssertOfflineMsgWriteFlushTest(test_write_flush_data);

		/*
		* test offline messages store in ram writes and flushes
		* using many messages of variable size
		* while keeping the api running the whole time
		*/
		/* update test params and run test */
		updateTestWriteFlushData(test_write_flush_data, offline_message_file, MEMORY_MODE_IN_RAM, WRITE_MODE_FILL_ENTIRE_STORE, FLUSH_MODE_DISCONNECT_CONNECT_FLUSH);
		twAssertOfflineMsgWriteFlushTest(test_write_flush_data);

		/* cleanup test struct memory */
		deleteTestWriteFlushData(test_write_flush_data);

	} else {
		/* if the OFFLINE_MSG_STORE is not 0 or 1, then the build has defined the message store as on disk or invalid, therefore ignore these tests */
		TEST_IGNORE_MESSAGE("Ignoring in RAM offline message tests since the build has defined the message store as on disk");
	}
}

/* change the offline message store path at runtime in various conditions*/
TEST(OfflineMsgStoreIntegration, twOfflineMsgStore_ChangePath_Success) {
	/* create static vairbales for message size and number or offline write iterations */
	int i = 0;
	const int message_const_size = 52;
	const int message_const_iterations = 3;

	/* create string to hold the current offline message file at various points during the test */
	char * current_store_dir = NULL;

	/* create large number to hold expected store size */
	uint64_t expected_value = 0;

	/* create result infotable to use while making requests */
	twPrimitive * result = NULL;

	if(OFFLINE_MSG_STORE == 1) {
		TEST_IGNORE_MESSAGE("Ignoring change offline message path tests since the offline store is defined in RAM");
	}

	/* enable on disk offline message store */
	enableOfflineMsgStore(TRUE, TRUE);

	/* check current offline message store file */
	twAssertCurrentOfflineFileExists();

	/* change offline message store */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_SetOfflineMsgStoreDir(offline_message_path_different), "could not change offline message store directory - before connect");

	/* check current offline message store file */
	twAssertCurrentOfflineFileExists();

	/* disconnect */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Disconnect("going down for offline message test"), "Could Not disconnect");

	/* acrue some messages, free result each time so it can be reused without leaking (3 because why not?)*/
	for (i = 0; i < message_const_iterations; i ++) {
		TEST_ASSERT_EQUAL(TW_WROTE_TO_OFFLINE_MSG_STORE, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE));
		if(result) {
			twPrimitive_Delete(result);
			result = NULL;
		}
	}
	/* ensure that the offline message store is being written too */
	expected_value = message_const_size * message_const_iterations;
	twTestOfflineFileSize(offline_message_path_different, expected_value);

	/* connect and flush */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES), "Could Not connect");
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, TRUE));
	if(result) {
		twPrimitive_Delete(result);
		result = NULL;
	}

	/* ensure that the offline message store is flushed too */
	expected_value = 0;
	twTestOfflineFileSize(offline_message_path_different, expected_value);

	/* change offline message store */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_SetOfflineMsgStoreDir(offline_message_path), "could not change offline message store directory - after flush");

	/* check current offline message store file */
	twAssertCurrentOfflineFileExists();
}

/* change the offline message store path at runtime in various conditions*/
TEST(OfflineMsgStoreIntegration, twOfflineMsgStore_ChangePath_Error) {

	/* create static vairbales for message size and number or offline write iterations */
	int i = 0;
	const int message_const_size = 52;
	const int message_const_iterations = 3;

	/* create string to hold the current offline message file at various points during the test */
	char * current_store_dir = NULL;

	/* create large number to hold expected store size */
	uint64_t expected_value = 0;

	/* create result infotable to use while making requests */
	twPrimitive * result = NULL;

	if(OFFLINE_MSG_STORE == 1) {
		TEST_IGNORE_MESSAGE("Ignoring change offline message path tests since the offline store is defined in RAM");
	}

	/* enable on disk offline message store */
	enableOfflineMsgStore(TRUE, TRUE);

	/* check current offline message store file */
	twAssertCurrentOfflineFileExists();

	/* disconnect */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Disconnect("going down for offline message test"), "Could Not disconnect");

	/* acrue some messages, free result each time so it can be reused without leaking (3 because why not?)*/
	for (i = 0; i < message_const_iterations; i ++) {
		TEST_ASSERT_EQUAL(TW_WROTE_TO_OFFLINE_MSG_STORE, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE));
		if(result) {
			twPrimitive_Delete(result);
			result = NULL;
		}
	}

	/* ensure that the offline message store is being written too */
	expected_value = message_const_size * message_const_iterations;
	twTestOfflineFileSize(offline_message_file, expected_value);

	/* change offline message store - THIS SHOULD FAIL */
	TEST_ASSERT_EQUAL_MESSAGE(TW_MSG_STORE_FILE_NOT_EMPTY, twApi_SetOfflineMsgStoreDir(offline_message_path_different), "offline message store was set when it was not supposed to");

	/* connect and flush */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES), "Could Not connect");
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, TRUE));
	if(result) {
		twPrimitive_Delete(result);
		result = NULL;
	}

	/* ensure that the offline message store is flushed too */
	expected_value = 0;
	twTestOfflineFileSize(offline_message_file, expected_value);

	/* check current offline message store file */
	twAssertCurrentOfflineFileExists();
}


/***** START: helper functions for offline message store tests: USE THESE IN INTEGRATION TESTS *****/

/* checks file size on disk against input string size */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineFileSize(const char * store_path, uint64_t expected_size) {
	return test_offline_file(store_path, &expected_size, OFFLINE_MSG_FILE_CHECK_SIZE);
}

/* checks file size on disk against to make sure the length is > 0 */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineFileIsNotEmpty(const char * store_path) {
	uint64_t expected_size = 0;
	return test_offline_file(store_path, &expected_size, OFFLINE_MSG_FILE_CHECK_NOT_EMPTY);
}


/* checks if the file is accesexistssible */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineFileExists(const char * store_path, char does_exist) {
	return test_offline_file(store_path, &does_exist, OFFLINE_MSG_FILE_CHECK_EXIST);
}

/* checks if the file accessible */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineFileAccessible(const char * store_path, char is_accessible) {
	return test_offline_file(store_path, &is_accessible, OFFLINE_MSG_FILE_CHECK_ACCESS);
}

/* checks file size on disk against input size */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineListCount(int expected_count) {
	return test_offline_list(&expected_count, OFFLINE_MSG_LIST_CHECK_COUNT);
}

/* checks if the file is accesexistssible */
/* returns TW_OK on success, reference twErrors.h for anything else*/
int twTestOfflineListExists(char does_exist) {
	return test_offline_list(&does_exist, OFFLINE_MSG_LIST_CHECK_EXIST);
}

/* quick helper function to determine if the current tw_offline_msg_store->offlineMsgFile exists on disk */
void twAssertCurrentOfflineFileExists() {
	char * current_store_dir = NULL;

	/* check current offline message store file */
	current_store_dir = getOfflineMsgDirectory();
	TEST_ASSERT_TRUE_MESSAGE(current_store_dir, "Could not get current offline message store directory - before connect");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineFileExists(current_store_dir, TRUE), "Error determining if offline message store exists");

	/* cleanup memory */
	if (current_store_dir) TW_FREE(current_store_dir);
}

/* quick helper function to determine if the current tw_offline_msg_store->offlineMsgFile does NOT exist on disk */
void twAssertCurrentOfflineFileDoesNotExist() {
	char * current_store_dir = NULL;

	/* check current offline message store file */
	current_store_dir = getOfflineMsgDirectory();
	TEST_ASSERT_EQUAL_MESSAGE(NULL, current_store_dir, "the offline message store file is set even though it should be NULL");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTestOfflineFileExists(offline_message_file, FALSE), "Error determining if offline message store exists");

	/* cleanup memory */
	if (current_store_dir) TW_FREE(current_store_dir);
}

/* main write flush function */
/* this will test all possible size combinations detailed in the test plan comment */
void twAssertOfflineMsgWriteFlushTest(test_write_flush * test_write_flush_data) {
	uint32_t size;
	int i,j;

	/* enable message store */
	test_enable_msg_store(test_write_flush_data);

	/* nested for loops to test all combinations of message size and offline message queue size */
	for (j = 0; j <TEST_RELATIVE_CONFIG_SIZES; j++ ) {
		/* set offline message queue size */
		twcfg_pointer->offline_msg_queue_size = test_relative_config_values->values.size_array[j];

		for(i = 0; i < TEST_RELATIVE_CONFIG_SIZES; i++) {

			TW_LOG(TW_DEBUG, "offline msg queue size = %d, msg body size = %d", test_relative_config_values->values.size_array[j], test_relative_config_values->values.size_array[i]);

			/* disconnect and write messages */
			test_disconnect_then_write(test_write_flush_data, test_relative_config_values->values.size_array[i]);

			/* restart Api if necessary */
			if (test_write_flush_data->flush_mode == FLUSH_MODE_DELETE_CREATE_FLUSH) {
				test_restart_api(test_write_flush_data);
			}

			/* connect and flush */
			test_connect_then_write(test_write_flush_data, test_relative_config_values->values.size_array[i]);

			/* cleanup */
		}
	}
}

/***** END: helper functions for offline message store tests *****/
