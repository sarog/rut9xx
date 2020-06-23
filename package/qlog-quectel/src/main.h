#define PATH_SIZE    1024//路径长度
/*-------------------------------------------------------------------------------------*/
/*        MACRO DEFINITION                                                                                    */
/*-------------------------------------------------------------------------------------*/

#define QUALCOMM_QXDM_LOG_REQ_FLAG   			"pkt req"
#define QUALCOMM_QXDM_LOG_RSP_FLAG   			"pkt rsp"
#define QUALCOMM_QXDM_LOG_END_FLAG   			"pkt end"
#define QUALCOMM_QXDM_LOG_LINE_SIZE   			(512 * 1024)
#define QUALCOMM_QXDM_LOG_PKG_SIZE   			        (512 * 1024)
#define QUALCOMM_QXDM_LOG_CONFIGFILE_SIZE   			(512 * 1024)

#define QUALCOMM_QXDM_LOG_FILE_SIZE   			(1 * 1024 * 1024)
#define QUALCOMM_QXDM_LOG_WRITE_MIN 			(64 * 1024)
#include "vetify_arg.h"
#include "serial.h"
typedef struct
{
	 FILE *h_config_file;
	 int config_file_size;
	 char *p_config_file_buffer;
#if 0	 
	 char *p_config_req;
	 char *p_config_req_line;
	 unsigned char *p_config_rsp_line;
#endif	 
}log_dat_info;

int qualcomm_send_datstart(log_dat_info* Log_dat_info);

