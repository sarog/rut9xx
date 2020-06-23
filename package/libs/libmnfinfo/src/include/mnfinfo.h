#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int start();
char *get_hex(char *off, char *len);
void get_eth_mac(char *mac);
void set_simpin(char *off, char *len, char *val, char *pin);
char *read_flash_chunk(char *off, char *len);
int mnfinfo_get_mac(char **output);
int mnfinfo_get_maceth(char **output);
int mnfinfo_get_name(char **output);
int mnfinfo_get_wps(char **output);
int mnfinfo_get_sn(char **output);
int mnfinfo_get_batch(char **output);
int mnfinfo_get_hwver(char **output);
int mnfinfo_get_simpin(char **output, int set, char *pin, char *offset);

