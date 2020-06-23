/*
 * Copyright (C), 2017 Teltonika.
 */

#ifndef CURLCMD_H
#define CURLCMD_H

int realloc_char(char **orig, int size);
int shell_escape(const char *input, char **output);
int curl_cmd(char *url, char *req_type, int conn_timeout, int verify_cert,
		char **data_name, char **data_value, int data_count);

#endif
