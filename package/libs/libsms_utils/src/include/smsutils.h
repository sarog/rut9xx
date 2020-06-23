#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libtlt_uci/libtlt_uci.h>
#include <dirent.h>
#include <fcntl.h>
#include <libeventslog/libevents.h>
#include <libtlt_base/base.h>
#include <sys/stat.h>

#define MY_PIPE "/tmp/my_pipe"
#define NET_TIMEOUT 30

int smsutils_sendEmail(char *smtpip, char *smtpport, char *senderemail, char *recipients, char *subject, char *text, char *user, char *pass, int secureconnection);

void Turn_Wifi(int updown, int sig);
void Change_profile(char *pro);
void Mobile_Data(int updown, int sig);
