#ifndef TW_C_SDK_INTEGRATIONUTILITIES_H
#define TW_C_SDK_INTEGRATIONUTILITIES_H

#include "twInfoTable.h"
#include "twThreads.h"
#include "twTls.h"
extern char * test_host;
extern int * test_port;
extern char * test_app_key;
extern twInfoTable* testConfigInfoTable;
extern char * configurationDirectory;

#define TW_HOST test_host
#define TW_PORT test_port
#define TW_APP_KEY test_app_key
#define TEST_THINGNAME "UnityIntegrationTestThing"
#define SUBSCRIBED_PROP_THINGNAME "One_Int_Prop"
#define TEST_REPONAME "UnityIntegrationTestRepo"
#define THREAD_RATE 5

#define TEST_SUBJECT_CN "54.86.136.65"
#define TEST_SUBJECT_O "ThingWorx"
#define TEST_SUBJECT_OU NULL
#define TEST_ISSUER_CN "intermediate_ca"
#define TEST_ISSUER_O  "ThingWorx"
#define TEST_ISSUER_OU NULL

#define TEST_SUBJECT_O_1 "alphaTestSubjectO"
#define TEST_SUBJECT_OU_1 "alphaTestSubjectOU"
#define TEST_SUBJECT_CN_1 "alphaTestSubjectCN"
#define TEST_ISSUER_O_1 "alphaTestIssuerO"
#define TEST_ISSUER_OU_1 "alphaTestIssuerOU"
#define TEST_ISSUER_CN_1 "alphaTestIssuerCN"

#define TEST_CA_CERT_FILE "root_cert.pem"
#define TEST_CA_CERT_FILE_XL "root_cert_xl.pem"
#define TEST_CA_CERT_FILE_1 "alphaTestCACertFile"

#define TW_PORT_SELF_SIGNED 8443
#define TW_PORT_NO_ROOT_CERT_IN_CHAIN 4443
#define TW_PORT_NO_SSL 8080
#define TW_PORT_DEFAULT_CIPHERS 4445
#define TW_PORT_NO_FIPS_CIPHERS 4444

#endif //TW_C_SDK_INTEGRATIONTESTUTILITIES_H

