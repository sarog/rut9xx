#include "twTls.h"
#include "cfuhash.h"
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twNtlm.h"
#include "twApiStubs.h"

int32_t NTLM_sendType1Msg(twSocket * sock, const char * req, char * domain, char * user, char * password);
int32_t NTLM_parseType2Msg(twSocket * sock, const char * req, char * resp, char * domain, char * username, char * password);
static twSocket * twNtlm_s = NULL;

TEST_GROUP(unit_NTLM_sendType1Msg);

TEST_SETUP(unit_NTLM_sendType1Msg) {
	eatLogs();
	twApi_Initialize(TW_HOST, (uint16_t)TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	twNtlm_s = twSocket_Create(TEST_PROXY_HOST, TEST_PROXY_PORT, 0);
	TEST_ASSERT_NOT_NULL(twNtlm_s);
}

TEST_TEAR_DOWN(unit_NTLM_sendType1Msg) {
	twSocket_Delete(twNtlm_s);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_NTLM_sendType1Msg) {
	RUN_TEST_CASE(unit_NTLM_sendType1Msg, test_twNtlm_sendType1Msg_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_sendType1Msg, test_twNtlm_type1Msg_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_sendType1Msg, test_twNtlm_base64Encode_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_sendType1Msg, test_twNtlm_BytesSent_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_sendType1Msg, test_twNtlm_sendType1Msg);
}


/* The intent of this mock socket is to send -1 as a return value. */
int mock_twNtlm_sendType1Msg_twSocket_Write1(twSocket * s, char * buf, int len, int timeout) {
	return -1;
}

/* The intent of this mocked GenerateType1Msg is to contain some string in its buffer. */
int32_t mock_GenerateType1Msg(char **buffer, uint32_t *length) {
	char data[4] = "foo";
	uint32_t val = 2;
	char *str = data;
	*buffer = (char *)TW_MALLOC(4);
	strncpy(*buffer, str, 4);
	*length = val;
	return 0;
}

/* The intent of this mocked GenerateType1Msg is to flush the buffer. */
int32_t mock_GenerateType1Msg1(char **buffer, uint32_t *length) {
	return 0;
}

int mock_base64_sendType1Msg_encode(const unsigned char *in,  unsigned long inlen, unsigned char *out, unsigned long *outlen) {
	return 1;
}

int mock_base64_sendType1Msg_encode1(const unsigned char *in,  unsigned long inlen, unsigned char *out, unsigned long *outlen) {
	return 0;
}

int mock_twNtlm_sendType1Msg_twSocket_For_sendType1Msg(twSocket * s, char * buf, int len, int timeout) {
	return 182;
}

/* Test plan: NTLM_sendType1Msg function will fail as we are not sending the appropriated arguments to the function. */
TEST(unit_NTLM_sendType1Msg, test_twNtlm_sendType1Msg_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	TEST_ASSERT_EQUAL(-1, NTLM_sendType1Msg(NULL, NULL, NULL, NULL, NULL));
	TEST_ASSERT_EQUAL(-1, NTLM_sendType1Msg(NULL, twNtlm_req, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_sendType1Msg(twNtlm_s, NULL, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_sendType1Msg(twNtlm_s, twNtlm_req, NULL, NULL, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_sendType1Msg(twNtlm_s, twNtlm_req, NULL, TEST_PROXY_USER, NULL));
}

/* Test plan: Test will fail as there was an error while create Type1 Message. */
TEST(unit_NTLM_sendType1Msg, test_twNtlm_type1Msg_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	twApi_stub->GenerateType1Msg = mock_GenerateType1Msg1;
	TEST_ASSERT_EQUAL(-1, NTLM_sendType1Msg(twNtlm_s, twNtlm_req, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: Test will fail because an error occur while encoding the NTLM Type 1 Message. */
TEST(unit_NTLM_sendType1Msg, test_twNtlm_base64Encode_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	twApi_stub->GenerateType1Msg = mock_GenerateType1Msg;
	twApi_stub->base64_encode = mock_base64_sendType1Msg_encode;
	TEST_ASSERT_EQUAL(-1, NTLM_sendType1Msg(twNtlm_s, twNtlm_req, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: Test will fail as there was an error occurs while sending NTLM type1 Message. */
TEST(unit_NTLM_sendType1Msg, test_twNtlm_BytesSent_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	twApi_stub->GenerateType1Msg = mock_GenerateType1Msg;
	twApi_stub->base64_encode = mock_base64_sendType1Msg_encode1;
	twApi_stub->twSocket_Write = mock_twNtlm_sendType1Msg_twSocket_Write1;
	TEST_ASSERT_EQUAL(-1, NTLM_sendType1Msg(twNtlm_s, twNtlm_req, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: NTLM_sendType1Msg function will success as its making a complete connection. */
TEST(unit_NTLM_sendType1Msg, test_twNtlm_sendType1Msg) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	twApi_stub->GenerateType1Msg = mock_GenerateType1Msg;
	twApi_stub->base64_encode = mock_base64_sendType1Msg_encode1;
	twApi_stub->twSocket_Write = mock_twNtlm_sendType1Msg_twSocket_For_sendType1Msg;
	TEST_ASSERT_EQUAL(0, NTLM_sendType1Msg(twNtlm_s, twNtlm_req, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}