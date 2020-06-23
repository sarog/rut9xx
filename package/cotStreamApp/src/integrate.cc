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

#include <libsera/srnethttp.h>
#include <libsera/srutils.h>
#include "integrate.h"

using namespace std;

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

int Integrate::integrate(const SrAgent &agent, const string &srv, const string &srt)
{
    SrNetHttp http(agent.server() + "/s", srv, agent.auth());
    if (registerSrTemplate(http, xid, srt) != 0) // Step 1,2,3
    {
        return -1;
    }

    http.clear();

    if (http.post("100," + agent.deviceID()) <= 0) // Step 4
    {
        return -1;
    }

    SmartRest sr(http.response());
    SrRecord r = sr.next();
    if (r.size() && (r[0].second == "50"))
    { // Step 4: NO

        http.clear();

        if (http.post("101,RouterDevice") <= 0) // Step 5
        {
            return -1;
        }

        sr.reset(http.response());
        r = sr.next();
        if ((r.size() == 3) && (r[0].second == "501"))
        {
            id = r[2].second; // Step 7
            string s = "102," + id + "," + agent.deviceID();
            if (http.post(s) <= 0) // Step 8
            {
                return -1;
            }

            http.post("109," + id + ",CONNECTED");
            http.post("110," + id + "," + currentDateTime() + ",AVAILABLE");
            return 0;
        }
    } else if ((r.size() == 3) && (r[0].second == "500"))
    { // Step 4: YES
        id = r[2].second;                           // Step 6

        return 0;
    }

    return -1;
}
