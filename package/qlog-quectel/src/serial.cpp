#include "serial.h"
#include "vetify_arg.h"
int serial_connect(const char *str_port)
{
    //struct termios newtio;
    struct termios tio;    
    struct termios settings;
    int retval;
    if(!str_port)
    {
        log_message("invalid parameter!");
        return -1;
    }

    if(g_is_serial_connect == 1)
    {
        log_message("port[%s] opened already!", str_port);
        return 1;
    }

    g_fd_port = open (str_port, O_RDWR | O_SYNC | O_NONBLOCK);
    if(g_fd_port < 0)
    {
        log_message("port[%s] open failed!", str_port);
        goto error_exit;
    }

	memset(&tio,0,sizeof(tio));
	tio.c_iflag=0;        
	tio.c_oflag=0;        
	tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information        
	tio.c_lflag=0;        
	tio.c_cc[VMIN]=1;        
	tio.c_cc[VTIME]=5;        
	cfsetospeed(&tio,B115200);            // 115200 baud        
	cfsetispeed(&tio,B115200);            // 115200 baud        
	tcsetattr(g_fd_port, TCSANOW, &tio);

	retval = tcgetattr (g_fd_port, &settings);        
	if (-1 == retval) 
	{            
		log_message("termio settings could not be fetched Linux System Error:%s", strerror (errno));            
		//return false;        
	}
	cfmakeraw (&settings);
	settings.c_cflag |= CREAD | CLOCAL;
	tcflush (g_fd_port, TCIOFLUSH);
	retval = tcsetattr (g_fd_port, TCSANOW, &settings);
	if (-1 == retval) 
	{            
		log_message("Device could not be configured: Linux System Errno: %s", strerror (errno));            
		//return false;        
	}
    g_is_serial_connect = 1;

    log_message("port[%s] connected!", str_port);

    return 0;

error_exit:
	g_error_programe=0;
    if(g_fd_port >= 0)
    {
        close(g_fd_port);
        g_fd_port = -1;
    }

    return -1;
}

int serial_autoconnect()
{
    char str_port_name[MAX_PATH];
    int ret_val;
    int ret_val_mode;
    if(g_is_serial_connect == 1)
    {
        return 1;
    }

		memset(str_port_name, 0, sizeof(str_port_name));
        sprintf(str_port_name, "%s%d", PORT_NAME_PREFIX, g_default_port);
        if(access(str_port_name, F_OK) == 0)
        {
            log_message("port[%s] found!", str_port_name);

        }
        else
        {
            log_message("port[%s] not found!", str_port_name);
            log_file("port[%s] not found! errno[%d]", str_port_name, errno);
            goto error_exit;
        }
    struct stat st;
    ret_val_mode=stat(str_port_name,&st);
    if(ret_val_mode<0)
    {
    	log_message("failed to get <%s> mode!", str_port_name);
    }
    else
    {
    	if((st.st_mode&S_IRWXU)==S_IRWXU&&(st.st_mode&S_IRWXG)==S_IRWXG&&(st.st_mode&S_IRWXO)==S_IRWXO)
    	 {
    		log_file("chmod 777  ok");
    	 }
    	    else
 {
    	    	 ret_val = chmod(str_port_name, S_IRWXU | S_IRWXG | S_IRWXO);//chmod 777
    	    	    if(ret_val < 0)
    	    	    {
    	    	    	log_message("failed to change <%s> mode!", str_port_name);
    	    	    	log_file("failed to change <%s> mode!", str_port_name);
    	    	    }
    	    	    else
    	    	    {
    	    	    	log_message("change <%s> mode ok!", str_port_name);
    	    	    }
    	    }
    }
    if(serial_connect(str_port_name) < 0)
    {
        log_message("failed to open serial port <%s> !", str_port_name);
        log_file("failed to open serial port <%s> !", str_port_name);
        goto error_exit;
    }

    return 0;

error_exit:
    return -1;
}

int serial_disconnect()
{
    if(g_fd_port >= 0)
    {
        close(g_fd_port);
        g_fd_port = -1;
    }

    g_is_serial_connect = 0;

    return 0;
}

int serial_flush()
{
    if(g_fd_port >= 0)
    {
        tcflush(g_fd_port, TCIFLUSH);
        tcflush(g_fd_port, TCOFLUSH);
    }

    return 0;
}

unsigned long serial_write(unsigned char *p_out_buffer, unsigned long out_buffer_size)
{
    unsigned long cb_wrote = 0;

    if(g_fd_port < 0)
    {
        log_message("please open port first!");
        log_file("please open port first!");
        return 0;
    }

    if(out_buffer_size <= 0)
        return 0;

    /*just write data to device*/
    cb_wrote = write(g_fd_port, p_out_buffer, out_buffer_size);
    if(cb_wrote <= 0)
    {
        log_file("port[0x%x] write error! errno[%d]", g_fd_port, errno);
        log_message("Read/Write File descriptor returned error: %s, error code %d", strerror(errno), cb_wrote);
        return 0;
    }
    return cb_wrote;
}

unsigned long serial_write_hexstring(const char *str_in_buffer)
{
    unsigned long write_cnt = 0;
    unsigned long str_len = 0;
    char ch_high, ch_low;
    unsigned char *buffer_to_write = NULL;
    unsigned long cb_wrote = 0;

    if(g_fd_port < 0)
    {
        log_message("please open port first!");
        log_file("please open port first!");
        return 0;
    }

    str_len = (unsigned long)strlen(str_in_buffer);

    if(!str_in_buffer || str_len <= 0)
        goto error_exit;

    buffer_to_write = (unsigned char *)malloc(str_len);
    if(!buffer_to_write)
    {
        log_message("malloc %d bytes failed!", str_len);
        log_file("malloc %d bytes failed! errno[%d]", str_len, errno);
        goto error_exit;
    }
	while(*str_in_buffer != '\0' && (*str_in_buffer + 1) != '\0')
    {
        ch_high = tolower(*str_in_buffer);
        ch_low = tolower(*(str_in_buffer + 1));
        if ((('0' <= ch_high && '9' >= ch_high) || ('a' <= ch_high && 'f' >= ch_high)) &&
            (('0' <= ch_low && '9' >= ch_low) || ('a' <= ch_low && 'f' >= ch_low)))
        {
            if ('0' <= ch_high && '9' >= ch_high)
            {
                buffer_to_write[write_cnt] = (ch_high - '0') << 4;
            }
            else
            {
                buffer_to_write[write_cnt] = (0x0a + ch_high - 'a') << 4;
            }

            if ('0' <= ch_low && '9' >= ch_low)
            {
                buffer_to_write[write_cnt] |= (ch_low - '0');
            }
            else
            {
                buffer_to_write[write_cnt] |= (0x0a + ch_low - 'a');
            }
            write_cnt++;
            str_in_buffer += 2;
        }
        else
            str_in_buffer++;
    }
    if(write_cnt <= 0)
	{

        	goto error_exit;
	}
  /*just write data to device*/

    cb_wrote = write(g_fd_port, buffer_to_write, write_cnt);
    if(cb_wrote <= 0)
    {
        log_message("port[0x%x] write error! errno[%d]", g_fd_port);
        log_file("port[0x%x] write error! errno[%d]", g_fd_port, errno);
        goto error_exit;
    }
    if(buffer_to_write)
        free(buffer_to_write);
    return cb_wrote;
error_exit:

    if(buffer_to_write)
        free(buffer_to_write);

    return 0;
}

int rx_data(unsigned char *buffer, size_t bytes_to_read, size_t *bytes_read)
{
	fd_set rfds;    
	struct timeval tv;    
	int retval;
	FD_ZERO (&rfds);
	FD_SET (g_fd_port, &rfds);

	tv.tv_sec  = 3;
	tv.tv_usec = 0;

	retval = select (g_fd_port + 1, &rfds, NULL, NULL, &tv);
	if(0 == retval)
	{
		printf("timeout\n");
		return 2;
	}
	if(retval < 0)
	{
		return 1;
	}
	retval = read (g_fd_port, buffer, bytes_to_read);
	if( 0 == retval )
	{
		return 2;
	}
	else if(retval < 0)
	{
		if (EAGAIN == errno) {
			usleep(1000);
			return 0;
		}
		else
		{
			printf("Read/Write File descriptor returned error: %s, error code %d", strerror (errno), retval);
			return 2;
		}
	}
	*bytes_read = retval;

	return 0;
	
}
#if 0
unsigned long serial_read(unsigned char *p_in_buffer, unsigned long in_buffer_size, unsigned long *p_bytes_read)
{
    int read_len = 0;

    if(g_fd_port < 0)
    {
        log_message("please open port first!");
        log_file("please open port first!");
        return 0;
    }

    if(!p_in_buffer || in_buffer_size <= 0)
        return 0;
    read_len = read(g_fd_port, p_in_buffer, in_buffer_size);
    if(read_len < 0)
    {
         printf("Read/Write File descriptor returned error: %s, error code %d", strerror (errno), read_len);
    }

    if(p_bytes_read)
        *p_bytes_read = read_len;

    return read_len;
}
#else
int serial_read(unsigned char *buffer, unsigned int bytes_to_read, unsigned int *bytes_read)
{
	fd_set rfds;    
	struct timeval tv;    
	int retval;
	FD_ZERO (&rfds);
	FD_SET (g_fd_port, &rfds);

	tv.tv_sec  = 3;
	tv.tv_usec = 0;

	retval = select (g_fd_port + 1, &rfds, NULL, NULL, &tv);
	if(0 == retval)
	{
		printf("timeout\n");
		return 2;
	}
	if(retval < 0)
	{
		return 1;
	}
	retval = read (g_fd_port, buffer, bytes_to_read);
	if( 0 == retval )
	{
		return 2;
	}
	else if(retval < 0)
	{
		if (EAGAIN == errno) {
			usleep(1000);
			return 0;
		}
		else
		{
			printf("Read/Write File descriptor returned error: %s, error code %d", strerror (errno), retval);
			return 2;
		}
	}
	*bytes_read = retval;
	return 0;
}

#endif
