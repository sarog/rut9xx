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

TEST_GROUP(unit_NTLM_parseType2Msg);

TEST_SETUP(unit_NTLM_parseType2Msg) {
	eatLogs();
	twApi_Initialize(TW_HOST, (uint16_t)TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	twNtlm_s = twSocket_Create(TEST_PROXY_HOST, TEST_PROXY_PORT, 0);
	TEST_ASSERT_NOT_NULL(twNtlm_s);
}

TEST_TEAR_DOWN(unit_NTLM_parseType2Msg) {
	twSocket_Delete(twNtlm_s);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_NTLM_parseType2Msg) {
	RUN_TEST_CASE(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_ErrorCode_Decode);
	RUN_TEST_CASE(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_GenerateType3Msg_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_base64Encode_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_ByteSent_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg);
}

/* The intent of this mock socket read is to connect to twNtlm_connectToProxy and flush data in the socket. */
int mock_twNtlm_parseType2Msg_twSocket_Read(twSocket * s, char * buf, int len, int timeout) {
	return 0;
}

/* The intent of this mock socket is to send -1 as a return value. */
int mock_twNtlm_parseType2Msg_twSocket_Write1(twSocket * s, char * buf, int len, int timeout) {
	return -1;
}


int mock_base64_parseType2Msg_encode(const unsigned char *in,  unsigned long inlen, unsigned char *out, unsigned long *outlen) {
	return 1;
}

int mock_base64_parseType2Msg_encode1(const unsigned char *in,  unsigned long inlen, unsigned char *out, unsigned long *outlen) {
	return 0;
}

/* The intent of this mocked GeneratedType3Msg is to flush the buffer. */
int mock_twNtlm_parseType2Msg_GenerateType3Msg(const char * domain, const char * username, const char * password,
                                 const void *challenge, uint32_t challengeLength, char **outputBuf, uint32_t *outputLength) {
	return 0;
}

/* The intent of this mocked GenerateType3Msg2 is to contain some string in its buffer. */
int mock_twNtlm_parseType2Msg_GenerateType3Msg2(const char * domain, const char * username, const char * password,
                                  const void *challenge, uint32_t challengeLength, char **outputBuf, uint32_t *outputLength) {
	char data[4] = "foo";
	uint32_t val = 2;
	char *str = data;
	*outputBuf = (char *)TW_MALLOC(4);
	strncpy(*outputBuf, str, 4);
	*outputLength = val;
	return 0;
}

int mock_twNtlm_parseType2Msg_twSocket_Write3(twSocket * s, char * buf, int len, int timeout) {
	return 182;
}

/* Test plan: NTLM_parseType2Msg function fails as we are not providing the actual arguments to the function. */
TEST(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	char *twNtlm_resp = "407";
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(NULL, NULL, NULL, NULL, NULL, NULL));
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(NULL, twNtlm_req, twNtlm_resp, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(twNtlm_s, NULL, twNtlm_resp, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(twNtlm_s, twNtlm_req, NULL, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(twNtlm_s, twNtlm_req, twNtlm_resp, NULL, NULL, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(twNtlm_s, twNtlm_req, twNtlm_resp, NULL, TEST_PROXY_USER, NULL));
}

/* Test plan: NTLM_parseType2Msg function will fail due to an error occurrence while decoding the challenge string. */
TEST(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_ErrorCode_Decode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	char *twNtlm_resp = "407";
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(twNtlm_s, twNtlm_req, twNtlm_resp, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: Test will fail due to an error occur while creating Type 3 Message because buffer is having Null value. */
TEST(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_GenerateType3Msg_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	char *twNtlm_resp = "abcd";
	twApi_stub->GenerateType3Msg = mock_twNtlm_parseType2Msg_GenerateType3Msg;
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(twNtlm_s, twNtlm_req, twNtlm_resp, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: Test will fail while encoding Type 3 Message. */
TEST(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_base64Encode_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	char *twNtlm_resp = "this string is going to test after decoding.aaaa";
	twApi_stub->GenerateType3Msg = mock_twNtlm_parseType2Msg_GenerateType3Msg2;
	twApi_stub->base64_encode = mock_base64_parseType2Msg_encode;
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(twNtlm_s, twNtlm_req, twNtlm_resp, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: Test will fail due to an error occur while sending NTLM Type 2 Message. */
TEST(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg_ByteSent_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	char *twNtlm_resp = "this string is going to test after decoding.aaaa";
	twApi_stub->GenerateType3Msg = mock_twNtlm_parseType2Msg_GenerateType3Msg2;
	twApi_stub->base64_encode = mock_base64_parseType2Msg_encode1;
	twApi_stub->twSocket_Write = mock_twNtlm_parseType2Msg_twSocket_Write1;
	TEST_ASSERT_EQUAL(-1, NTLM_parseType2Msg(twNtlm_s, twNtlm_req, twNtlm_resp, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test Plan: NTLM_parseType2Msg function will pass and make connection to parse Type 2 Message. */
TEST(unit_NTLM_parseType2Msg, test_twNtlm_parseType2Msg) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	char *twNtlm_resp = "this string is going to test after decoding.aaaa";
	twApi_stub->GenerateType3Msg = mock_twNtlm_parseType2Msg_GenerateType3Msg2;
	twApi_stub->base64_encode = mock_base64_parseType2Msg_encode1;
	twApi_stub->twSocket_Write = mock_twNtlm_parseType2Msg_twSocket_Write3;
	TEST_ASSERT_EQUAL(0, NTLM_parseType2Msg(twNtlm_s, twNtlm_req, twNtlm_resp, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
}