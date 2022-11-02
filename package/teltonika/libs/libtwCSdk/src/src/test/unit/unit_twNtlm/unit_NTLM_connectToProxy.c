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

TEST_GROUP(unit_NTLM_connectToProxy);

TEST_SETUP(unit_NTLM_connectToProxy) {
	eatLogs();
	twApi_Initialize(TW_HOST, (uint16_t)TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	twNtlm_s = twSocket_Create(TEST_PROXY_HOST, TEST_PROXY_PORT, 0);
	TEST_ASSERT_NOT_NULL(twNtlm_s);
}

TEST_TEAR_DOWN(unit_NTLM_connectToProxy) {
	twSocket_Delete(twNtlm_s);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_NTLM_connectToProxy) {
	RUN_TEST_CASE(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode);
	RUN_TEST_CASE(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_NTLM_sendType1Msg);
	RUN_TEST_CASE(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_twSocket_WaitFor);
	RUN_TEST_CASE(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_bytesRead);
	RUN_TEST_CASE(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_authHeader);
	RUN_TEST_CASE(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_authHeader2);
	RUN_TEST_CASE(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_NTLM_parseType2Msg);
	RUN_TEST_CASE(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_Response);
}

/* Test plan: NTLM_connetToProxy function will fail to connect to proxy as we are not passing
*  the appropriate arguments to the function. */
TEST(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n\r\n";
	char *twNtlm_resp = "407";
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(NULL, NULL, NULL, NULL, NULL));
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(NULL, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, NULL, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, NULL, TEST_PROXY_USER, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, NULL, TEST_PROXY_PASS));
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, NULL));
}

/* The intent of this mock socket read is to connect to twNtlm_connectToProxy and flush data in the socket. */
int mock_twNtlm_connectToProxy_twSocket_Read(twSocket * s, char * buf, int len, int timeout) {
	return 0;
}

/* The intent of this mock socket is to send -1 as a return value. */
int mock_twNtlm_connectToProxy_twSocket_Write1(twSocket * s, char * buf, int len, int timeout) {
	return -1;
}

/* The intent of this mock is to connect to twNtlm_connectToProxy and send data to NTLM_sendType1Msg. */
int mock_NTLM_connectToProxy_sendType1Msg(twSocket * sock, const char * req, char * domain, char * user, char * password) {
	return 1;
}

int mock_NTLM_connectToProxy_sendType1Msg1(twSocket * sock, const char * req, char * domain, char * user, char * password) {
	return 0;
}

int mock_twNtml_twSocket_WaitFor2(twSocket * s, int timeout) {
	return 0;
}

int mock_twNtml_twSocket_WaitFor(twSocket * s, int timeout) {
	return 1;
}

/* The intent of this mock socket read is to connect to twNtlm_connectToProxy and contain at least Authenticate. */
int mock_twNtlm_connectToProxy_twSocket_Read2(twSocket * s, char * buf, int len, int timeout) {
	static int readCount = 0;
	if(readCount++ == 1) {
		strncpy(buf, "abcdefghi200Authenticate", 24);
		return 224;
	}
	else
		return 0;
}

/* The intent of this mock socket read is to connect to twNtlm_connectToProxy and clear the buffer. */
int mock_twNtlm_connectToProxy_twSocket_Read5(twSocket * s, char * buf, int len, int timeout) {
	static int readCount1 = 0;
	if(readCount1++ == 1) {
		strncpy(buf, NULL, 0);
		return 224;
	}
	else
		return 0;
}

/* The intent of this mock socket read is to connect to twNtlm_connectToProxy and contain some string. */
int mock_twNtlm_connectToProxy_twSocket_Read3(twSocket * s, char * buf, int len, int timeout) {
	static int readCount3 = 0;
	if(readCount3++ == 1) {
		strncpy(buf, "Authenticate: NTLM TlRMTVNTUAABAAAABaIAAAAAAAAAAAAAAAAAAAAAAAA=\r\n\r\n", 72);
		return 224;
	}
	else
		return 0;
}

int mock_twNtlm_connectToProxy_NTLM_parseType2Msg(twSocket * sock, const char * req, char * resp, char * domain, char * username, char * password) {
	return 1;
}

/* The intent of this mock is to drain the buffer. */
char * mock_duplicateString(const char * input) {
	return NULL;
}

int mock_twNtlm_connectToProxy_twSocket_ReadForResponse(twSocket * s, char * buf, int len, int timeout) {
	static int readCount3 = 0;
	if(readCount3++ == 1) {
		strncpy(buf, "Authenticate: NTLM TlRMTVNTUAABAAAABaIAAAAAAAAAAAAAAAAAAAAAAAA=\r\n\r\n", 72);
		return 224;
	}
	else
		return 0;
}

/* Test plan: NTLM_connectToProxy function will fail to connect to the proxy server as there is an error
*  while sending NTLM Type 1 Message to the server. */
TEST(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_NTLM_sendType1Msg) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n";
	char *twNtlm_resp = "407";
	twApi_stub->NTLM_sendType1Msg = mock_NTLM_connectToProxy_sendType1Msg;
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
}


/* Test plan: NTLM_connetToProxy function will fail while attempting to receive the respose of NTLM Type 1 Message.
*  Timeout problem occurs . */
TEST(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_twSocket_WaitFor) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n";
	char *twNtlm_resp = "407";
	twApi_stub->NTLM_sendType1Msg = mock_NTLM_connectToProxy_sendType1Msg1;
	twApi_stub->twSocket_WaitFor = mock_twNtml_twSocket_WaitFor2;
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test Plan: NTLM_connetToProxy function fails while reading data from the socket. */
TEST(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_bytesRead) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n";
	char *twNtlm_resp = "407";
	twApi_stub->twSocket_WaitFor = mock_twNtml_twSocket_WaitFor;
	twApi_stub->NTLM_sendType1Msg = mock_NTLM_connectToProxy_sendType1Msg1;
	twApi_stub->twSocket_Read = mock_twNtlm_connectToProxy_twSocket_Read;
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: The test will fail because socket buffer must contain Athenticate string in it due to which
*  Authentication header is NULL. */
TEST(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_authHeader) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n";
	char *twNtlm_resp = "407";
	twApi_stub->twSocket_WaitFor = mock_twNtml_twSocket_WaitFor;
	twApi_stub->twSocket_Read = mock_twNtlm_connectToProxy_twSocket_Read5;
	twApi_stub->NTLM_sendType1Msg = mock_NTLM_connectToProxy_sendType1Msg1;
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: Test will failing because Authentication header is expecting for NTLM substring in it. */
TEST(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_authHeader2) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n";
	char *twNtlm_resp = "407";
	twApi_stub->twSocket_WaitFor = mock_twNtml_twSocket_WaitFor;
	twApi_stub->twSocket_Read = mock_twNtlm_connectToProxy_twSocket_Read2;
	twApi_stub->NTLM_sendType1Msg = mock_NTLM_connectToProxy_sendType1Msg1;
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan: The test will fail while parsing NTLM Type 2 Message. */
TEST(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_NTLM_parseType2Msg) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n";
	char *twNtlm_resp = "407";
	twApi_stub->twSocket_WaitFor = mock_twNtml_twSocket_WaitFor;
	twApi_stub->twSocket_Read = mock_twNtlm_connectToProxy_twSocket_Read3;
	twApi_stub->NTLM_parseType2Msg = mock_twNtlm_connectToProxy_NTLM_parseType2Msg;
	twApi_stub->NTLM_sendType1Msg = mock_NTLM_connectToProxy_sendType1Msg1;
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
}

/* Test plan:NTLM_connectToProxy function fails to connect because the buffer is drained. */
TEST(unit_NTLM_connectToProxy, test_twNtlm_connectToProxy_ErrorCode_Response) {
	char *twNtlm_req = "CONNECT localhost:443 HTTP/1.1\r\nHost: 55.55.55.55\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n";
	char *twNtlm_resp = "407";
	twApi_stub->twSocket_WaitFor = mock_twNtml_twSocket_WaitFor;
	twApi_stub->duplicateString = mock_duplicateString;
	twApi_stub->NTLM_sendType1Msg = mock_NTLM_connectToProxy_sendType1Msg1;
	twApi_stub->twSocket_Read = mock_twNtlm_connectToProxy_twSocket_ReadForResponse;
	TEST_ASSERT_EQUAL(-1, NTLM_connectToProxy(twNtlm_s, twNtlm_req, twNtlm_resp, TEST_PROXY_USER, TEST_PROXY_PASS));
}