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

#define UCI_LOOKUP_COMPLETE_EX (1 << 1)

#define AUTH_FILE_PATH "/tmp/cotauth"

using namespace std;

int getdir(string dir, vector<string> &files);
void createAuthFile(void);
void updateAuthConfig(void);
string getConfigOption(const char *p, const char *s, const char *o);
void setConfigOption(const char *p, const char *s, 
        const char *o, const char *v);
string getCommandOutput(string cmd);

int main() {
    const string C_CM_MAIN_PATH = "/usr/lib/lua/cot";

    // get device info
    string serial = getConfigOption("hwinfo", "hwinfo", "serial");
    string rev = getConfigOption("hwinfo", "hwinfo", "hwver");
    string model = getConfigOption("hwinfo", "hwinfo", "mnf_code").substr(0, 6);
    string fwversion = getCommandOutput("cat /etc/version");
    string fwname = "RUT9XX";
    string fwurl = "https://wiki.teltonika-networks.com/view/RUT9xx_Firmware";

    if (model.find("RUT2") != string::npos) {
        fwname = "RUT2XX";
        fwurl = "https://wiki.teltonika-networks.com/view/RUT2xx_Firmware";
    }

    string imei = getCommandOutput("gsmctl -i");
    string cell = getCommandOutput("gsmctl -C");
    string icc = getCommandOutput("gsmctl -J");
    string imsi = getCommandOutput("gsmctl -x");
    string oper = getCommandOutput("gsmctl -o");
    string type = getCommandOutput("gsmctl -t");
    string mnc = getCommandOutput("gsmctl -f | awk '{print substr($1,1,3)}'");
    string lac = getCommandOutput(
        "gsmctl -A AT+CREG? | awk '{split($0,a,\"\\\"\"); print a[2]}'");


    // get server address and port
    string address = getConfigOption("cot", "cumulocity", "server");

    address = "https://" + address;

    const char* const server = address.c_str();
    const char* const deviceID = serial.c_str();

    syslog(LOG_INFO, "Device Serial: %s", deviceID);
    syslog(LOG_INFO, "Server Address: %s", server);

    // create auth file
    createAuthFile();

    srLogSetLevel(SRLOG_INFO); // set log level to info

    Integrate igt;
    SrAgent agnt(server, deviceID, &igt); // instantiate SrAgent

    // bootstrap to Cumulocity
    if (agnt.bootstrap(AUTH_FILE_PATH)) {
        return 0;
    }

    // update auth config
    updateAuthConfig();

    // integrate to Cumulocity
    string ver, tpl;
    string tplpath = C_CM_MAIN_PATH + "/srtemplate.txt";
    if (readSrTemplate(tplpath.c_str(), ver, tpl) != 0) {
        return 0;
    }

    if (agnt.integrate(ver, tpl)) {
        return 0;
    }

    // update hardware
    string regeq = "104," + agnt.ID();

    regeq += ",RouterDevice"; // name
    regeq += ",Router";       // type

    // hardware
    regeq += "," + model;     // model
    regeq += "," + rev;       // revision
    regeq += "," + serial;    // serial

    // firmware
    regeq += "," + fwname;    // name
    regeq += "," + fwversion; // version
    regeq += "," + fwurl;     // url

    // mobile
    regeq += "," + imei;      // imei
    regeq += "," + cell;      // cellid
    regeq += "," + icc;       // iccid
    regeq += "," + imsi;      // imsi
    regeq += "," + oper;      // currentOperator
    regeq += "," + type;      // connType
    regeq += "," + mnc;       // mnc
    regeq += "," + lac;       // lac

    agnt.send(regeq);

    SrLuaPluginManager lua(agnt);
    
    // add given path to Lua package.path
    string libpath = C_CM_MAIN_PATH + "/?.lua";
    lua.addLibPath(libpath.c_str());

    // load lua scripts
    vector<string> files = vector<string>();
    getdir(C_CM_MAIN_PATH, files);

    for (unsigned int i = 0; i < files.size(); i++) {
        if (files[i].find(".lua") != string::npos) {
            lua.load(files[i].c_str());
        }
    }

    // start the reporter thread
    SrReporter reporter(server, agnt.XID(), agnt.auth(), 
        agnt.egress,  agnt.ingress);
    if (reporter.start() != 0) {
        return 0;
    }

    // start the device push thread
    SrDevicePush push(server, agnt.XID(), agnt.auth(), agnt.ID(), agnt.ingress);
    if (push.start() != 0) {
        return 0;
    }

    agnt.loop();
    return 0;
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

void setConfigOption(const char *p, const char *s, 
        const char *o, const char *v) {
    struct uci_context *ctx;
    struct uci_ptr ptr;
    char *cmd;
    size_t len;

    len = strlen(p) + strlen(s) + strlen(o) + 3;

    if (v != NULL) {
        len += strlen(v) + 1;
    }

    cmd = (char *)malloc(len);

    if (!cmd) {
        return;
    }

    if (!v) {
        snprintf(cmd, len, "%s.%s=%s", p, s, o);
    } else {
        snprintf(cmd, len, "%s.%s.%s=%s", p, s, o, v);
    }

    ctx = uci_alloc_context();
    if (!ctx) {;
        free(cmd);
		return;
	}
    
    if (uci_lookup_ptr(ctx, &ptr, cmd, true) != UCI_OK) {
        uci_free_context(ctx); 
        free(cmd);
		return;
    }

    free(cmd);

    if (uci_set(ctx, &ptr) != UCI_OK) {
        uci_free_context(ctx); 
		return;
    }

    uci_save(ctx, ptr.p);
    uci_commit(ctx, &ptr.p, true);
    uci_free_context(ctx);
}

string getCommandOutput(string cmd) {
	FILE *fp;
	string result;
	char buffer[256];
	cmd.append(" 2>&1");

	fp = popen(cmd.c_str(), "r");
	if (!fp) {
		return result;
	}

	while (fgets(buffer, sizeof(buffer), fp)) {
		result.append(buffer);
	}
		
	pclose(fp);

	result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
	return result;
}
