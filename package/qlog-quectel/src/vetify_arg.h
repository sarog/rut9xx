#ifndef __VETIFY_ARG_H__
#define __VETIFY_ARG_H__
#include "platform_def.h"
//验证输入参数
void vertify_port(int* nContinuRun,char** argv,int argc,int nPlace);
void vertify_filename(int* nContinuRun,char** argv,int argc,int nPlace,config_type* config_info);
void vertify_savepath(int* nContinuRun,char** argv,int argc,int nPlace,config_type* config_info);
void vertify_cache(int* nContinuRun,char** argv,int argc,int nPlace,config_type* config_info);
void vertify_maxsize(int* nContinuRun,char** argv,int argc,int nPlace,config_type* config_info);
void vertify_baudrate(int* nContinuRun,char** argv,int argc,int nPlace);
void vertify_arg(int* nContinuRun,char** argv,int argc,int nPlace);
int vertify_serailport();
int vertifyAllnum(char* ch);
void Resolve_port(char *chPort,int* nPort );
//验证参数是否正确，是否包含空格
int vertify_parameter(char** argv,int argc,int nPlace);
//log文件记录和打印输出
int create_log(char * filename);
int close_log(void);
int log_file(const char * fmt,...);
void log_message(const char * str_fmt,...);
int open_log(char * filename);
#endif

