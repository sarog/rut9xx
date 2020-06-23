
#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__
#include <stdio.h>
#include <stdlib.h>
#include "stdarg.h"
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#define PATH_SIZE    1024//路径长度
#define PORT_NAME_PREFIX    "/dev/ttyUSB" //linux下端口
#define  MAX_PATH    1024
#define QUALCOMM_QXDM_LOG_BUFFER_SIZE   		        (1 * 1024 * 1024)
typedef struct
{
	int baudrate;//波特率
	int port;//端口号
	char savepath[PATH_SIZE];//保存文件路径
	char filename[PATH_SIZE];//文件名称
	int cache;//缓存大小(kb)
	int maxsize;//单个文件大小
}config_type;
extern int g_log_cache;//缓存大小
extern int g_log_size;//单个文件最大值
extern int g_log_baudrate;//设置默认的波特率
extern int g_default_port;
extern int g_log_baudrate;
extern int g_fd_port;
extern int g_fd_port;
extern int g_is_serial_connect;
extern int g_error_programe;
extern FILE *g_log_file;//日志句柄
#endif

