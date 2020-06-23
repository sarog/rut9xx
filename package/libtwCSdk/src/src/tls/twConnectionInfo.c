/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Connection settings structure
 */

#include "twOSPort.h"
#include "twConnectionInfo.h"
#include "stringUtils.h"

twConnectionInfo * twConnectionInfo_Create(twConnectionInfo * copy) {
	twConnectionInfo * c = NULL;
	c = (twConnectionInfo *)TW_CALLOC(sizeof(twConnectionInfo), 1);
	if (c && copy) {
		c->ca_cert_file = duplicateString(copy->ca_cert_file);
		c->client_cert_file = duplicateString(copy->client_cert_file);
		c->client_key_file = duplicateString(copy->client_key_file);
		c->client_key_passphrase = copy->client_key_passphrase;
		c->disableEncryption = copy->disableEncryption;
		c->doNotValidateCert = copy->doNotValidateCert;
		c->issuer_cn = duplicateString(copy->issuer_cn);
		c->issuer_o = duplicateString(copy->issuer_o);
		c->issuer_ou = duplicateString(copy->issuer_ou);
		c->subject_cn = duplicateString(copy->subject_cn);
		c->subject_o = duplicateString(copy->subject_o);
		c->subject_ou = duplicateString(copy->subject_ou);
		c->proxy_host = duplicateString(copy->proxy_host);
		c->proxy_port = copy->proxy_port;
		c->proxy_pwd = copy->proxy_pwd;
		c->selfsignedOk = copy->selfsignedOk;
		c->appkeyFunction = copy->appkeyFunction;
		c->ws_host = duplicateString(copy->ws_host);
		c->ws_port = copy->ws_port;
	}
	return c;
}

void twConnectionInfo_Delete(void * info) {
	twConnectionInfo * i = (twConnectionInfo *) info;
	if (!i) return;
	if (i->proxy_host) TW_FREE(i->proxy_host);
	if (i->proxy_user) TW_FREE(i->proxy_user);
	if (i->subject_cn) TW_FREE(i->subject_cn);
	if (i->subject_o) TW_FREE(i->subject_o);
	if (i->subject_ou) TW_FREE(i->subject_ou);
	if (i->issuer_cn) TW_FREE(i->issuer_cn);
	if (i->issuer_o) TW_FREE(i->issuer_o);
	if (i->issuer_ou) TW_FREE(i->issuer_ou);
	if (i->ca_cert_file) TW_FREE(i->ca_cert_file);
	if (i->client_cert_file) TW_FREE(i->client_cert_file);
	if (i->client_key_file) TW_FREE(i->client_key_file);
	if (i->ws_host) TW_FREE(i->ws_host);
	TW_FREE (info);
}

