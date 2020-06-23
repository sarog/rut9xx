/*==========================================================================
  *
  *Quectel Wireless Solutions Co.,Ltd. All rights reserved.
  *
  *----------------------------------------------------------------------------------------
  *
  * PROJECT           ANDROID
  *
  * LANGUAGE          ANSI C
  *
  *
  * DESCRIPTION
  *
  *
  *---------------------------------------------------------------------------------------
  *
  * HOSTORY
  *
  *       when            who                        what, where, why
  *
  * 2013/06/26           arno.wang                   initial version.
  *
  *========================================================================*/

/*-------------------------------------------------------------------------------------*/
/*        INCLUDE FILE                                                                                              */
/*-------------------------------------------------------------------------------------*/
#include "main.h"


/*-------------------------------------------------------------------------------------*/
/*        TYPE DEFINITION                                                                                    */
/*-------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------*/
/*        GLOBAL VARIABLE DEFINITION                                                                                    */
/*-------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------*/
/*        LOCAL VARIABLE DEFINITION                                                                                    */
/*-------------------------------------------------------------------------------------*/
int g_fd_port = -1;
int g_is_serial_connect = 0;
int g_is_qxdm_logging = 0;
pthread_t g_tid_log;
char g_str_qualcomm_log_dir[MAX_PATH];
char g_str_qualcomm_log_file[MAX_PATH];
char g_str_logfile[MAX_PATH];
int g_default_port=0;
pthread_t g_tid_logtest;
pthread_t log_stop;
int g_log_cache=64;
int g_log_size=20;
int g_log_baudrate=115200;
int g_error_programe=1;
static int g_model_restart=0;
int g_restart_log=0;
FILE *g_log_file;
log_dat_info Log_dat_info;

void sleepms(long msec)
{
#if 0
    struct timespec ts;
    int err;

    ts.tv_sec = (msec / 1000);
    ts.tv_nsec = (msec % 1000) * 1000 * 1000;

    do
    {
        err = nanosleep (&ts, &ts);
    } while (err < 0 && errno == EINTR);
#else
    long long us = msec;
    us *= 1000;
    usleep(us);
#endif
}


int qualcomm_serial_logstop()
{

    if(serial_autoconnect() <0)
    {
        goto error_exit;
    }

    if(g_is_qxdm_logging == 1)
    {
        if(!serial_write_hexstring("60 00 12 6a  7e"))
            goto error_exit;
        sleepms(100);
        if(!serial_write_hexstring("73 00 00 00  00 00 00 00 da 81 7e"))
            goto error_exit;
        sleepms(100);
        if(!serial_write_hexstring("7d 5d 05 00  00 00 00 00 00 74 41 7e"))
            goto error_exit;
        sleepms(100);
        if(!serial_write_hexstring("4b 04 0e 00  0d d3 7e"))
            goto error_exit;
        sleepms(100);
        if(!serial_write_hexstring("4b 04 0e 00  0d d3 7e"))
            goto error_exit;
        sleepms(100);
        g_is_qxdm_logging = 0;

        log_message("stop catching log, directory<%s>.", g_str_qualcomm_log_dir);
        log_file("stop catching log, directory<%s>.", g_str_qualcomm_log_dir);
    }
    serial_disconnect();
    return 1;
error_exit:

    log_message("Log stop error.");
    log_file("Log stop error.");
    serial_disconnect();
    g_is_qxdm_logging = 0;
    return 0;
}
void *qualcomm_serial_log_stop_thread(void *arg)
{
	char recive[100];
	while(g_is_qxdm_logging)
	{
		scanf("%s",recive);
		if(strcmp(recive,"stoplog")==0)
		{
			log_message("program is coming to the end...");
			log_file("program is coming to the end...");
			qualcomm_serial_logstop();

			break;
		}
	}
	return 0;
}
int CheckLogFile(FILE** h_logfile,FILE** g_log_file)
{
start_creat_file:
	if(access(g_str_qualcomm_log_dir, F_OK)!=0)
	{
		char str_dir[MAX_PATH];
		for (int i = 1; i < MAX_PATH && g_str_qualcomm_log_dir[i] != 0; i++)
		{
			  if (g_str_qualcomm_log_dir[i] == '\\'  || g_str_qualcomm_log_dir[i] == '/' )
			   {
	                strcpy(str_dir, g_str_qualcomm_log_dir);
	                str_dir[i] = '\0';
	                mkdir(str_dir, 0700);
			   }
		}//创建目录
	}
	if(access(g_str_logfile, F_OK) != 0)//文件不存在，创建
	{
		if(*h_logfile!=NULL)
		{
			fclose(*h_logfile);
				*h_logfile=NULL;
		}

		*h_logfile = fopen(g_str_logfile, "wb");
		if(*h_logfile==NULL)
		{
			if(access(g_str_qualcomm_log_dir, F_OK)!=0)
				goto start_creat_file;
			log_message("Create log failed");
			return 0;
		}
	}
	if(access(g_str_qualcomm_log_file, F_OK) != 0)
	{
		if(*g_log_file!=NULL)
		{
			fclose(*g_log_file);
			*g_log_file=NULL;
		}

		*g_log_file=fopen(g_str_qualcomm_log_file,"wt");
		if(*g_log_file==NULL)
		{
		if(access(g_str_qualcomm_log_dir, F_OK)!=0)
			goto start_creat_file;
		log_message("Create log file failed");
		return 0;
		}
		log_file("log start");
	}
	return 1;
}
void *qualcomm_serial_log_thread(void *arg)
{
    unsigned char *rx_buffer;
    int cur_pos;
    long long file_size;
    int recved_bytes;
    unsigned int retlen;
    int ret;
    FILE *h_logfile = NULL;
    int r_count=5;
    rx_buffer = (unsigned char *)malloc(QUALCOMM_QXDM_LOG_BUFFER_SIZE);
    if(!rx_buffer)
    {
         log_message("malloc %d bytes error",QUALCOMM_QXDM_LOG_BUFFER_SIZE);
         log_file("malloc %d bytes error, errno(%d)!", QUALCOMM_QXDM_LOG_BUFFER_SIZE, errno);
         return NULL;
    }

    log_file( "log started!");

    recved_bytes = 0;
    while(g_is_qxdm_logging)
    {
        if(h_logfile)
        {
            /* close the file */
            cur_pos = ftell(h_logfile);
            fseek(h_logfile, 0, SEEK_END);
            file_size = ftell(h_logfile);
            fseek(h_logfile, cur_pos, SEEK_SET);
            long long file_max_size=g_log_size*1024*1024LL;

            if(file_size > file_max_size)
            {

        		fflush(h_logfile);
        		fclose(h_logfile);
        		h_logfile = NULL;
            }
        }

        if(!h_logfile)
        {
            struct statfs disk_info;
            struct tm *tm;
            time_t t;

            /* get disk spaces */
            if(statfs(g_str_qualcomm_log_dir, &disk_info) >= 0)
            {
                unsigned long long total_size = disk_info.f_bavail * disk_info.f_bsize;
                //log_message("df -fl");
                unsigned long long disk_count=(unsigned long long)QUALCOMM_QXDM_LOG_FILE_SIZE*3;//磁盘小于3M提示磁盘空间不足
                if(total_size<disk_count)
                {
                    log_message("[%s]no enough disk space!", g_str_qualcomm_log_dir);
                    log_file("malloc %d bytes error, errno(%d)!", "[%s, 0x%x] no space! total_size: %llu bytes!", g_str_qualcomm_log_dir, disk_info.f_type, disk_info.f_type, total_size);
                    g_error_programe=0;
                    break;
                }
            }
            /* create log file */
            if(g_model_restart==0)
            {
            	 memset(g_str_logfile,0,sizeof(g_str_logfile));
            	t = time(NULL);
				tm = localtime(&t);
				sprintf(g_str_logfile, "%s%d%02d%02d%02d%02d%02d.qmdl",
						g_str_qualcomm_log_dir,
						(tm->tm_year+1900),
						(tm->tm_mon+1),
						tm->tm_mday,
						tm->tm_hour,
						tm->tm_min,
						tm->tm_sec);
				h_logfile = fopen(g_str_logfile, "wb");
            }
            else
            {
            	h_logfile = fopen(g_str_logfile, "ab+");
            	g_model_restart=0;
            }
            if(!h_logfile)
            {
                log_message("create log file failed.");
                log_file("create log file failed, errno(%d).", errno);
                break;
            }

            log_message("log file(%s)", g_str_logfile);
            log_file("log file(%s)", g_str_logfile);
        }

        /* read log to file */
        //
        if(CheckLogFile(&h_logfile,&g_log_file)==0)
        {
        	g_error_programe=0;
        	break;
        }
        ret = serial_read(rx_buffer + recved_bytes, QUALCOMM_QXDM_LOG_BUFFER_SIZE - recved_bytes, &retlen);
        if(ret == 0)
	        recved_bytes += retlen;

        if(retlen==0)
       {
	       	if(vertify_serailport()==0)
	       	{
	       		serial_disconnect();
	       		g_model_restart=1;
	       		g_restart_log=1;
	       		char str_port_name[MAX_PATH];
	       				memset(str_port_name, 0, sizeof(str_port_name));
	       				sprintf(str_port_name, "%s%d", PORT_NAME_PREFIX, g_default_port);
	       		log_message("port[%s] disconnected",str_port_name);
	       		break;
	       	}
       }
        if(retlen<0)
        {
        	g_error_programe=0;
        	break;
        }
        if(recved_bytes > 0 /*(g_log_cache*1024)*/)//缓存大小
        {
        	 fwrite(rx_buffer, recved_bytes, 1, h_logfile);
        	 fflush(h_logfile);
        	 recved_bytes = 0;
        }
        /* print current status *///打印当前状态，注释
        {
            static unsigned long start_tick = 0;
            static unsigned long cur_tick = 0;
            static unsigned long reced_bytes = 0;
            unsigned long periodtick = 0;

            reced_bytes += retlen;
            if(start_tick == 0)
                start_tick = time(NULL);
            else
            {
                cur_tick = time(NULL);
                periodtick = cur_tick - start_tick;
                if(periodtick >=10)
                {
                    log_message("received(%.2fKB), tick(%us), %.2fKB/s", (float)reced_bytes/(float)1000, periodtick, (float)reced_bytes/float(1000*periodtick));
                    start_tick = cur_tick;
                    if(reced_bytes==0)
                    {
                    	//r_count--;
                    }
                    else
                        r_count=5;
                    if(r_count==0)
                    {
                         g_restart_log=1;
                         break;
                    }
                    reced_bytes = 0;
                }
            }

        }
    }
 //stop_log:

    if(recved_bytes > 0)
    {
    	 fwrite(rx_buffer, recved_bytes, 1, h_logfile);
    	 fflush(h_logfile);
    	 recved_bytes = 0;
    }
    if(h_logfile)
    {
        fflush(h_logfile);
        fclose(h_logfile);
    }

    if(rx_buffer)
        free(rx_buffer);
    log_file( "log stopped!");
    return NULL;
}
int qualcomm_dat_read(log_dat_info* Log_dat_info,const char *str_config_file)
{

	if(serial_autoconnect() <0)
	{
		return -1;
	}

	/* 1: flush serial buffer. */
	serial_flush();

	/* 2: read config file and write to serial. */
	Log_dat_info->h_config_file = fopen(str_config_file, "r");
	if(!Log_dat_info->h_config_file)
    {
        log_message("read configuration file <%s> failed, errno(%d)!", str_config_file, errno);
       return -1;
    }

    /* get file size */
    fseek(Log_dat_info->h_config_file, 0, SEEK_END);
    Log_dat_info->config_file_size = ftell(Log_dat_info->h_config_file);
    fseek(Log_dat_info->h_config_file, 0, SEEK_SET);
    if(Log_dat_info->config_file_size <= 0)
    {
        log_message("get configuration file size failed, size(%d)!", Log_dat_info->config_file_size);

        return -1;
    }

    /* malloc buffer for writting configuration file */
    Log_dat_info->p_config_file_buffer = (char *)malloc(Log_dat_info->config_file_size);
    if(!Log_dat_info->p_config_file_buffer)
    {
        log_message("malloc (%d) bytes buffer for configuration file failed! errno[%d]", Log_dat_info->config_file_size, errno);

        return -1;
    }
    Log_dat_info->config_file_size = fread(Log_dat_info->p_config_file_buffer, 1, Log_dat_info->config_file_size, Log_dat_info->h_config_file);
    if(Log_dat_info->config_file_size <= 0)
    {
        log_message("read configuration file failed, errno(%d)!", errno);

        return -1;
    }
   //log_message("config file <%s>, size<%.2fKB>", str_config_file, (float)Log_dat_info->config_file_size/(float)1000);
   return 1;
}
static void print_hex(unsigned char *ptr, int len)
{
#if 0
	int i;
	printf("[ ");
	for(i = 0; i < len; i++)
	{
		printf("%02X", ptr[i]);
	}
	
	printf(" ]\n");
#endif	
}
static int write_hdlc_pkt(unsigned char *ptr, int len)
{
	int written;
	unsigned int read;
	int ret;
	int try_times = 10;
	unsigned char tx_buff[1024];
	serial_flush();
	while(try_times--)
	{
		written = serial_write(ptr, len);
		if(written != len)
		{
			log_message("write %d bytes failed. in fact write %d bytes ,send again.\n", len, written);
			sleep(8);
		}else
		{	
			if(serial_read(tx_buff, 1024, &read) == 0)
			{
				print_hex(tx_buff, read);
				return 0;
			}
			
		}
	}
	return -1;
}
int qualcomm_send_datstart(log_dat_info* Log_dat_info)
{
#define MAX_LINE_CNT	1024 * 5
#define FLAG			0x7E
	int ret;
	unsigned char line[MAX_LINE_CNT];
	int start = 0;
	int start_line = 0;
	int file_size = Log_dat_info->config_file_size;
	char* config_buffer_ptr = Log_dat_info->p_config_file_buffer;
	int ok_cnt = 0;
	int fail_cnt = 0;

	while(start < file_size)
	{
		line[start_line] = config_buffer_ptr[start++];
		if(line[start_line] == FLAG)
		{
			if(start_line > (MAX_LINE_CNT - 1))
			{
				start_line = 0;
			}
			line[++start_line] = 0;			//for debug
			ret = write_hdlc_pkt(line, start_line);
			start_line = 0;						
		}else
		{
			start_line++;
		}		
	}	
	return 1;
}

void qualcomm_create_logfile(const char *str_config_file,const char *str_log_dir)
{

	char str_dir[MAX_PATH];
	int i, len;

	strcpy(str_dir, str_log_dir);
	len = strlen(str_dir);
	if(str_dir[len - 1] == '\\' || str_dir[len - 1] == '/')
	{
	str_dir[len - 1] = '\0';
	}
	struct tm *tm;
	time_t t;
	t = time(NULL);
	tm = localtime(&t);
	sprintf(g_str_qualcomm_log_dir, "%s/qxdmlog_%d%02d%02d%02d%02d%02d/",
							str_log_dir,
							(tm->tm_year+1900),
							(tm->tm_mon+1),
							tm->tm_mday,
							tm->tm_hour,
							tm->tm_min,
							tm->tm_sec);
	sprintf(g_str_qualcomm_log_file, "%s/qxdmlog_%d%02d%02d%02d%02d%02d/DIAG%d%02d%02d%02d%02d%02d.log",
						str_log_dir,
						(tm->tm_year+1900),
						(tm->tm_mon+1),
						tm->tm_mday,
						tm->tm_hour,
						tm->tm_min,
						tm->tm_sec,
						(tm->tm_year+1900),
						(tm->tm_mon+1),
						tm->tm_mday,
						tm->tm_hour,
						tm->tm_min,
						tm->tm_sec);

    for (i = 1; i < MAX_PATH && g_str_qualcomm_log_dir[i] != 0; i++)
    {
        if (g_str_qualcomm_log_dir[i] == '\\'  || g_str_qualcomm_log_dir[i] == '/' )
        {
            strcpy(str_dir, g_str_qualcomm_log_dir);
            str_dir[i] = '\0';
            mkdir(str_dir, 0700);
        }
    }

    log_message("start to catch log, configuration file<%s>, log directory<%s>", str_config_file, g_str_qualcomm_log_dir);
    create_log(g_str_qualcomm_log_file);
    log_file("start to catch log, configuration file<%s>, log directory<%s>", str_config_file, g_str_qualcomm_log_dir);
}
void Stop(int signo)
{
	qualcomm_serial_logstop();
	close_log();
	sleep(1);
	log_message("end of program!");
	_exit(0);
}
int qualcomm_serial_logstart(const char *str_config_file, const char *str_log_dir,int type)
{
    int ret;
    int ret_s;
    pthread_attr_t attr;
    struct sched_param param;
    Log_dat_info.h_config_file=NULL;
    Log_dat_info.p_config_file_buffer=NULL;
    
    g_is_qxdm_logging = 1;
    if(type==0)
    	qualcomm_create_logfile(str_config_file,str_log_dir);
    else
    	open_log(g_str_qualcomm_log_file);
    log_file( "the file of max size is %dM",g_log_size);
    log_file( "the cache  is %dK",g_log_cache);
    if(qualcomm_dat_read(&Log_dat_info,str_config_file)==-1)
    {
    	goto error_exit;
    }
    if(qualcomm_send_datstart(&Log_dat_info)==0)
    {
    	goto error_exit;
    }

    //create thread
    pthread_attr_init (&attr);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority=99;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&g_tid_log, &attr, qualcomm_serial_log_thread, &attr);
    ret_s= pthread_create(&log_stop, NULL, qualcomm_serial_log_stop_thread, NULL);
    if (ret_s < 0)
    {
        log_message("pthread stop create failed");
        log_file("pthread stop create failed");
        goto error_exit;
    }
    if (ret < 0)
    {
        log_message("pthread_create log thread create failed.  errno[%d]", errno);
        log_file("pthread_create log thread create failed.  errno[%d]", errno);
        goto error_exit;
    }
    pthread_attr_destroy(&attr);

    return 0;

error_exit:
	g_error_programe=0;
    log_message("start to catch log error.");
    log_file("start to catch log error.");
    serial_disconnect();

    if(Log_dat_info.h_config_file!=NULL)
        fclose(Log_dat_info.h_config_file);
    if(Log_dat_info.p_config_file_buffer!=NULL)
        free(Log_dat_info.p_config_file_buffer);
    return -1;
}
int main(int argc, char **argv)
{
	int nContinuRun=1;
     config_type config_info;
	config_info.baudrate=11520;
	config_info.port=0;
	config_info.cache=g_log_cache;
	config_info.maxsize=g_log_size;
	
	log_message("QLog Version: LTE_QLog_Linux&Android_V1.1.0\n"); 
	log_file("QLog Version: LTE_QLog_Linux&Android_V1.1.0\n"); 

	
	strcpy(config_info.filename,"diag_start.dat");
	strcpy(config_info.savepath,"./qxdmlog");
	int error_warn_count=0;
	//------------------------------------
	int i;
	signal(SIGINT,Stop);
	for(i=0;i<argc;i++)
	{
		if(strcmp(argv[i],"p")==0||strcmp(argv[i],"s")==0||strcmp(argv[i],"f")==0||strcmp(argv[i],"c")==0||strcmp(argv[i],"m")==0||strcmp(argv[i],"b")==0)
		{
			log_message("[ERROR]:Invalid Parameter %s",argv[i]);
			nContinuRun=0;
		}
		if(strcmp(argv[i],"-p")==0)
		{
			vertify_port(&nContinuRun,argv,argc,i);
		}
		else if(strcmp(argv[i],"-s")==0)
		{
			vertify_savepath(&nContinuRun,argv,argc,i,&config_info);
		}
		else if(strcmp(argv[i],"-f")==0)
		{
			vertify_filename(&nContinuRun,argv,argc,i,&config_info);
		}
		else if(strcmp(argv[i],"-c")==0)
		{
			vertify_cache(&nContinuRun,argv,argc,i,&config_info);
		}
		else if(strcmp(argv[i],"-m")==0)
		{
			vertify_maxsize(&nContinuRun,argv,argc,i,&config_info);
		}
		else if(strcmp(argv[i],"-b")==0)
		{
			vertify_baudrate(&nContinuRun,argv,argc,i);
		}
		else
			vertify_arg(&nContinuRun,argv,argc,i);

	}
	//programe start
	char str_port_name[MAX_PATH];
	memset(str_port_name, 0, sizeof(str_port_name));
	sprintf(str_port_name, "%s%d", PORT_NAME_PREFIX, g_default_port);
	if(nContinuRun==1)
	{
		qualcomm_serial_logstart(config_info.filename,config_info.savepath,0);
		while(g_is_qxdm_logging)
		{
			if(g_error_programe)
			{
				if(g_restart_log==1)
				{
					if(g_model_restart==0)
					{
						qualcomm_serial_logstop();
						log_message("port[%s] receive no data ",str_port_name);
						break;
					}
					else
					{
						if(vertify_serailport()==1)
						{
							close_log();
							sleep(1);
							qualcomm_serial_logstart(config_info.filename,config_info.savepath,1);
							g_restart_log=0;
						}
						else
						{						
							log_message("waiting...");
							sleep(10);
						}
					}
				}
				else
				{
					error_warn_count=0;
					sleep(10);
				}
			}
			else
			break;
		}
		close_log();
		log_message("end of program!");
	}
    return 0;
}


