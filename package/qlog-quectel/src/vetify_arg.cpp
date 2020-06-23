#include "vetify_arg.h"
//验证字符串是否有数字组成
int vertifyAllnum(char* ch)
{
	int re=1;
	int i;
	for (i=0;i<strlen(ch);i++)
	{
		if(isdigit(*(ch+i))==0)
		{
			return 0;
		}
	}
	return re;
}
void Resolve_port(char *chPort,int* nPort )
{
	*nPort=-1;
	char chPortNum[10];
	char restr[7];
	if(strlen(chPort)<sizeof("ttyUSB**")&&strlen(chPort)>6)
	{
		memcpy(restr,chPort,6);
		restr[6]='\0';
		if(strcmp(restr,"ttyUSB")==0)
		{
			memset(chPortNum,0,sizeof(chPortNum));
			memcpy(chPortNum,chPort+(sizeof("ttyUSB")-1),(strlen(chPort)-(sizeof("ttyUSB")-1)));
			if(vertifyAllnum(chPortNum)&&*chPortNum!=0)
			{
				*nPort=atoi(chPortNum);
			}
		}
	}
}
//验证参数是否正确，是否包含空格
int vertify_parameter(char** argv,int argc,int nPlace)
{
	if((nPlace+2)<argc)
	{
		if(strcmp(argv[nPlace+2],"-p")==0||strcmp(argv[nPlace+2],"-s")==0||strcmp(argv[nPlace+2],"-f")==0||strcmp(argv[nPlace+2],"-c")==0||strcmp(argv[nPlace+2],"-m")==0||strcmp(argv[nPlace+2],"-b")==0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}
//端口验证：参数说明：验证参数是否正确，命令参数，命令参数个数，当前查找的命令参数的位置
void vertify_port(int* nContinuRun,char** argv,int argc,int nPlace)
{
	if(vertify_parameter(argv,argc,nPlace)==0)
	{
		log_message("[ERROR]Parameter format error");
		*nContinuRun=0;
	}
	else
	{
		if((nPlace+1)<argc)
		{
			Resolve_port(argv[nPlace+1],&g_default_port);
			if(g_default_port==-1)
			{
				log_message("[ERROR]Port is incorrect");
				*nContinuRun=0;
			}
		}
		else
		{
			log_message("[ERROR]Missing parameter");
			*nContinuRun=0;
		}
	}
}
void vertify_filename(int* nContinuRun,char** argv,int argc,int nPlace,config_type* config_info)
{
	if(vertify_parameter(argv,argc,nPlace)==0)
		{
			log_message("[ERROR]Parameter format error");
			*nContinuRun=0;
		}
		else
		{
			if((nPlace+1)<argc)
			{
				if(strlen(argv[nPlace+1])<PATH_SIZE)
				{
					if(access(argv[nPlace+1],F_OK))
					{
						log_message("[ERROR]File not found");
						*nContinuRun=0;
					}
					else
					{
						memset(config_info->filename,0,sizeof(config_info->filename));
						sprintf(config_info->filename, "%s",argv[nPlace+1]);
					}

				}
				else
				{
					log_message("[ERROR]File name length is out of range");
					*nContinuRun=0;
				}
			}
			else
			{
				log_message("[ERROR]Missing parameter");
				*nContinuRun=0;
			}
		}
}
void vertify_savepath(int* nContinuRun,char** argv,int argc,int nPlace,config_type* config_info)
{
	if(vertify_parameter(argv,argc,nPlace)==0)
	{
			log_message("[ERROR]Parameter format error");
			*nContinuRun=0;
	}
	else
	{
		if((nPlace+1)<argc)
		{
			if(strlen(argv[nPlace+1])<PATH_SIZE)
			{
				memset(config_info->savepath,0,sizeof(config_info->savepath));
				sprintf(config_info->savepath, "%s",argv[nPlace+1]);
			}
			else
			{
				log_message("[ERROR]The path length is out of range");
				*nContinuRun=0;
			}
		}
		else
		{
			log_message("[ERROR]Missing parameter");
			*nContinuRun=0;
		}
	}
}
void vertify_cache(int* nContinuRun,char** argv,int argc,int nPlace,config_type* config_info)
{
	if(vertify_parameter(argv,argc,nPlace)==0)
	{
			log_message("[ERROR]Parameter format error");
			*nContinuRun=0;
	}
	else
	{
		if((nPlace+1)<argc)
		{
			if(vertifyAllnum(argv[nPlace+1]))
			{
				config_info->cache=atoi(argv[nPlace+1]);
				g_log_cache=config_info->cache;
				if((config_info->cache*1024)>=QUALCOMM_QXDM_LOG_BUFFER_SIZE)//缓存不能超过1m
				{
					log_message("[ERROR]Cache size can not exceed 1024KB");
					*nContinuRun=0;
				}
			}
			else
			{
				log_message("[ERROR]Cache size format error");
				*nContinuRun=0;
			}
		}
		else
		{
			log_message("[ERROR]Missing parameter");
			*nContinuRun=0;
		}
	}
}
void vertify_maxsize(int* nContinuRun,char** argv,int argc,int nPlace,config_type* config_info)
{
	if(vertify_parameter(argv,argc,nPlace)==0)
	{
			log_message("[ERROR]Parameter format error");
			*nContinuRun=0;
	}
	else
	{
		if((nPlace+1)<argc)
		{
			if(vertifyAllnum(argv[nPlace+1]))
			{
				config_info->maxsize=atoi(argv[nPlace+1]);
				g_log_size=config_info->maxsize;
				if(config_info->maxsize==0)
				{
					log_message("[ERROR]File size cannot be 0");
					*nContinuRun=0;
				}
			}
			else
			{
				log_message("[ERROR]File size format error");
				*nContinuRun=0;
			}
		}
		else
		{
			log_message("[ERROR]Missing parameter");
			*nContinuRun=0;
		}
	}
}
int testbaudrat(char *baudrate)
{
	int ret=0;
	if(strcmp(baudrate,"4800")==0||
	strcmp(baudrate,"9600")==0||
	strcmp(baudrate,"19200")==0||
	strcmp(baudrate,"38400")==0||
	strcmp(baudrate,"57600")==0||
	strcmp(baudrate,"115200")==0||
	strcmp(baudrate,"230400")==0||
	strcmp(baudrate,"460800")==0||
	strcmp(baudrate,"921600")==0
	)
	{
	ret=1;

	}
	return ret;
}
void vertify_baudrate(int* nContinuRun,char** argv,int argc,int nPlace)
{
	if(vertify_parameter(argv,argc,nPlace)==0)
	{
		log_message("[ERROR]Parameter format error");
		*nContinuRun=0;
	}
	else
	{
		if((nPlace+1)<argc)
		{
			if(testbaudrat(argv[nPlace+1]))
			{
				g_log_baudrate=atoi(argv[nPlace+1]);

			}
			else
			{
				log_message("[ERROR]Baudrate size format error");
				*nContinuRun=0;
			}
		}
		else
		{
			log_message("[ERROR]Missing parameter");
			*nContinuRun=0;
		}
	}
}
void vertify_arg(int* nContinuRun,char** argv,int argc,int nPlace)
{
	if(nPlace>0)
	{
		if(*argv[nPlace]=='-')
		{
			if(strlen(argv[nPlace])==2)
			{
				if(*(argv[nPlace]+1)!='p'&&(*argv[nPlace]+1)!='s'&&(*argv[nPlace]+1)!='f'&&(*argv[nPlace]+1)!='c'&&(*argv[nPlace]+1)!='m'&&(*argv[nPlace]+1)!='b')
				{
					log_message("[ERROR]Parameter not recognized");
					*nContinuRun=0;
				}
			}
			else
			{
				log_message("[ERROR]Parameter not recognized");
				*nContinuRun=0;
			}


		}
	}
}

//验证串口是否存在
int vertify_serailport()
{
	 char str_port_name[MAX_PATH];
	 memset(str_port_name, 0, sizeof(str_port_name));
	 sprintf(str_port_name, "%s%d", PORT_NAME_PREFIX, g_default_port);
	 if(access(str_port_name, F_OK) == 0)
	  {
	      return 1;
	  }
	  else
	  {
		  return 0;
	  }
}
int log_file(const char * fmt,...)
{
	va_list args;
	char* buffer = (char*)malloc(300);
	int result = 0;
	struct   tm     *ptm;
	long       ts;
	int         y,m,d,h,n,s;
	ts   =   time(NULL);
	ptm   =   localtime(&ts);
	y   =   ptm-> tm_year+1900;     
	m   =   ptm-> tm_mon+1;             
	d   =   ptm-> tm_mday;               
	h   =   ptm-> tm_hour;               
	n   =   ptm-> tm_min;                 
	s   =   ptm-> tm_sec;
	char* newfmt = (char*)malloc(strlen(fmt)+30);
	sprintf(newfmt,"[%d-%02d-%02d %02d:%02d:%02d]%s",y,m,d,h,n,s,fmt);
	va_start(args, newfmt);
	vsprintf(buffer, newfmt, args);
	strcat(buffer,"\r\n");
	va_end(args);

	if(buffer == NULL)
		return  result;
	if(g_log_file != NULL){
		result = fwrite((void *)buffer,sizeof(char),strlen((const char *)buffer)-1,g_log_file);
		fflush(g_log_file);
	}

	free(newfmt);
	free(buffer);
	return result;
}
int create_log(char * filename)
{
		g_log_file = fopen(filename,"wt");
		if(g_log_file == NULL){
			return (int) NULL;
		}
		return 1;
}
int open_log(char * filename)
{
		g_log_file = fopen(filename,"at+");
		if(g_log_file == NULL){
			return (int) NULL;
		}
		return 1;
}
int close_log(void)
{
	if(g_log_file!=NULL)
	{
		fclose(g_log_file);
		g_log_file = NULL;
	}
	return 1;
}
void log_message(const char * str_fmt,...)
{
    static char log_trace[256];
    va_list ap;

    va_start(ap, str_fmt);
    vsnprintf(log_trace, sizeof(log_trace), str_fmt, ap);
    va_end(ap);


    printf("Q_LOG: %s\r\n", log_trace);
}

