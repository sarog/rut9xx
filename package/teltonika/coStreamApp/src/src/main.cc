/*
 * Copyright (C) 2015-2017 Cumulocity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <fstream>
#include <string> 
#include <cstdlib>
#include <uci.h>
#include <dirent.h>
#include <syslog.h>
#include <libsera/sragent.h>
#include <libsera/srreporter.h>
#include <libsera/srlogger.h>
#include <libsera/srdevicepush.h>
#include <libsera/srutils.h>
#include <libsera/srluapluginmanager.h>
#include "integrate.h"
#ifdef __cplusplus
extern "C"
{
	#include <libgsm.h>
	#include <libmnfinfo/mnfinfo.h>
}
#endif

#define UCI_LOOKUP_COMPLETE_EX (1 << 1)

#define AUTH_FILE_PATH "/tmp/coauth"

using namespace std;

int getdir(string dir, vector<string> &files);
void createAuthFile(void);
void updateAuthConfig(void);
string getConfigOption(const char *p, const char *s, const char *o);
void setConfigOption(const char *p, const char *s, const char *o, const char *t);
string getValues(SrAgent agnt, modem_dev device, char* modem, string serial);

int main() {
	Integrate igt;
	string ver, tpl, regeq, libpath;
	vector<string> files;
	const string C_CM_MAIN_PATH = "/usr/lib/lua/co";
	unsigned int device;
	char *serial;

	// get server address and port
	string isTLS = getConfigOption("cot", "cumulocity", "ssl");
	string address = getConfigOption("cot", "cumulocity", "server");
	string modem_str = getConfigOption("cot", "cumulocity", "modem");

	serial = lmnfinfo_get_sn();
	string serial_str(serial);

	address = "https://" + address;

	const char* const server = address.c_str();
	const char* const deviceID = serial_str.c_str();
	char* modem = modem_str.empty() ? get_default_modem() : (char*)modem_str.c_str();

	if (modem) {
		device = get_modem(modem);
	}

	syslog(LOG_INFO, "Device Serial: %s", deviceID);
	syslog(LOG_INFO, "Server Address: %s", server);

	// create auth file
	createAuthFile();

	srLogSetLevel(SRLOG_INFO); // set log level to info

	SrAgent agnt(server, deviceID, &igt); // instantiate SrAgent

	// bootstrap to Cumulocity
	if (agnt.bootstrap(AUTH_FILE_PATH)) {
		return 0;
	}

	// update auth config
	updateAuthConfig();

	// integrate to Cumulocity
	string tplpath = C_CM_MAIN_PATH + "/srtemplate.txt";
	if (readSrTemplate(tplpath.c_str(), ver, tpl) != 0) {
		return 0;
	}

	if (agnt.integrate(ver, tpl)) {
		return 0;
	}

	regeq = getValues(agnt, static_cast<modem_dev>(device), modem, serial_str);

	agnt.send(regeq);

	SrLuaPluginManager lua(agnt);

	// add given path to Lua package.path
	libpath = C_CM_MAIN_PATH + "/?.lua";
	lua.addLibPath(libpath.c_str());

	// load lua scripts
	files = vector<string>();
	getdir(C_CM_MAIN_PATH, files);

	for (unsigned int i = 0; i < files.size(); i++) {
		if (files[i].find(".lua") != string::npos) {
			lua.load(files[i].c_str());
		}
	}

	// start the reporter thread
	SrReporter reporter(server, agnt.XID(), agnt.auth(), agnt.egress,  agnt.ingress);
	if (reporter.start() != 0) {
		return 0;
	}

	// start the device push thread
	SrDevicePush push(server, agnt.XID(), agnt.auth(), agnt.ID1(), agnt.ingress);
	if (push.start() != 0) {
		return 0;
	}

	agnt.loop();
	return 0;
}

string getValues(SrAgent agnt, modem_dev device, char* modem, string serial) {
	char *buffer, *mnf;
	buffer = (char *) malloc(100 * sizeof(char));
	buffer[0] = 0;

	mnf = lmnfinfo_get_hwver();
	string rev_str(mnf);

	mnf = lmnfinfo_get_name();
	string model_str(mnf);

	string fwversion = getConfigOption("system", "system", "device_fw_version");
	string fwname = getConfigOption("system", "system", "routername");
	string fwurl = "https://wiki.teltonika-networks.com/";

	if (modem && gsmctl_get_imei(device, buffer, modem) == 0) {
		buffer[strlen(buffer)-2] = 0;
	} else {
		strncpy(buffer, "N/A", sizeof(buffer));
	}
	string imei_str(buffer);
	buffer[0] = 0;

	if (modem && gsmctl_get_iccid(device, buffer, modem) == 0) {
		buffer[strlen(buffer)-1] = 0;
	} else {
		strncpy(buffer, "N/A", sizeof(buffer));
	}
	string icc_str(buffer);
	buffer[0] = 0;

	if (modem && gsmctl_get_signal_cell_id(device, buffer, modem) == 0) {
		buffer[strlen(buffer)-1] = 0;
	} else {
		strncpy(buffer, "N/A", sizeof(buffer));
	}
	string cell_str(buffer);
	buffer[0] = 0;

	if (modem && gsmctl_get_imsi(device, buffer, modem) == 0) {
		buffer[strlen(buffer)-2] = 0;
	} else {
		strncpy(buffer, "N/A", sizeof(buffer));
	}
	string imsi_str(buffer);
	buffer[0] = 0;

	if (modem && gsmctl_get_operator(device, buffer, modem) == 0) {
		buffer[strlen(buffer)-1] = 0;
	} else {
		strncpy(buffer, "N/A", sizeof(buffer));
	}
	string oper_str(buffer);
	buffer[0] = 0;

	if (modem && gsmctl_get_conntype(device, buffer, modem) == 0) {
		buffer[strlen(buffer)-1] = 0;
	} else {
		strncpy(buffer, "N/A", sizeof(buffer));
	}
	string type_str(buffer);
	buffer[0] = 0;

	if (modem && gsmctl_get_opernum(device, buffer, modem) == 0) {
		buffer[strlen(buffer)-1] = 0;
	} else {
		strncpy(buffer, "N/A", sizeof(buffer));
	}
	string mnc_str(buffer);
	buffer[0] = 0;

	// update hardware
	string regeq = "104," + agnt.ID1();

	regeq += ",RouterDevice"; // name
	regeq += ",Router";       // type

	// hardware
	regeq += "," + model_str;     // model
	regeq += "," + rev_str;       // revision
	regeq += "," + serial;    // serial

	// firmware
	regeq += "," + fwname;    // name
	regeq += "," + fwversion; // version
	regeq += "," + fwurl;     // url

	// mobile
	regeq += "," + imei_str;      // imei
	regeq += "," + cell_str;      // cellid
	regeq += "," + icc_str;       // iccid
	regeq += "," + imsi_str;       // imsi
	regeq += "," + oper_str;       // currentOperator
	regeq += "," + type_str;       // connType
	regeq += "," + mnc_str;        // mnc
	regeq += ",-";        // lac

	free(buffer);

	return regeq;
}

int getdir(string dir, vector<string> &files) {
	DIR *dp;
	struct dirent *dirp;

	if ((dp = opendir(dir.c_str())) == NULL) {
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		files.push_back(dir + "/" + string(dirp->d_name));
	}

	closedir(dp);
	return 0;
}

void createAuthFile(void) {
	string tenant = getConfigOption("cot", "cumulocity", "tenant");
	string user = getConfigOption("cot", "cumulocity", "username");
	string pass = getConfigOption("cot", "cumulocity", "password");

	if (tenant.empty() || user.empty() || pass.empty()) {
		syslog(LOG_INFO, "Auth file cannot be updated due missing details.");
		return;
	}

	ofstream f(AUTH_FILE_PATH, ios::out | ofstream::trunc);

	if (!f.is_open()) {
		syslog(LOG_INFO, "Unable to open %s file.", AUTH_FILE_PATH);
		return;
	}

	f << tenant << endl;
	f << user << endl;
	f << pass;

	f.close();
}

void updateAuthConfig(void) {
	ifstream f(AUTH_FILE_PATH);

	if (!f.is_open()) {
		syslog(LOG_INFO, "Unable to open %s file.", AUTH_FILE_PATH);
		return;
	}

	string tenant;
	string user;
	string pass;

	f >> tenant;
	f >> user;
	f >> pass;

	f.close();

	setConfigOption("cot", "cumulocity", "tenant", tenant.c_str());
	setConfigOption("cot", "cumulocity", "username", user.c_str());
	setConfigOption("cot", "cumulocity", "password", pass.c_str());
}

string getConfigOption(const char *p, const char *s, const char *o) {
	struct uci_context *ctx;
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	const char *value = NULL;
	string result;

	memset(&ptr, 0, sizeof(ptr));

	ptr.package = p;
	ptr.section = s;
	ptr.option = o;

	ctx = uci_alloc_context();
	if (!ctx) {
		return result;
	}

	if (uci_lookup_ptr(ctx, &ptr, NULL, false)) {
		uci_free_context(ctx);
		return result;
	}

	if (!(ptr.flags & UCI_LOOKUP_COMPLETE_EX)) {
		uci_free_context(ctx);
		return result;
	}

	e = ptr.last;

	switch (e->type) {
		case UCI_TYPE_SECTION:
			value = uci_to_section(e)->type;
			break;

		case UCI_TYPE_OPTION:
			switch(ptr.o->type) {
				case UCI_TYPE_STRING:
					value = ptr.o->v.string;
					break;
				default:
					value = NULL;
					break;
			}
			break;
		default:
			return 0;
	}

	uci_free_context(ctx);
	result = value;
	return result;
}

void setConfigOption(const char *p, const char *s, const char *o, const char *t) {
	struct uci_context *ctx;
	struct uci_ptr ptr;

	ctx = uci_alloc_context();
	if (!ctx) {
		return;
	}

	memset(&ptr, 0, sizeof(ptr));
	ptr.package = p;
	ptr.section = s;
	ptr.option = o;
	ptr.value = t;
	uci_lookup_ptr(ctx, &ptr, NULL, false);
	uci_set(ctx, &ptr);
	uci_commit(ctx, &ptr.p, false);
	uci_free_context(ctx);
}
