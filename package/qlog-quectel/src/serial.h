#include "platform_def.h"
//串口相关操作
int serial_connect(const char *str_port);
void set_baudrate(struct termios* newtio);
int serial_autoconnect();
int serial_disconnect();
int serial_flush();
unsigned long serial_write(unsigned char *p_out_buffer, unsigned long out_buffer_size);
unsigned long serial_write_hexstring(const char *str_in_buffer);
//int serial_read(unsigned char *p_in_buffer, unsigned long in_buffer_size, unsigned long *p_bytes_read);
int serial_read(unsigned char *p_in_buffer, unsigned int in_buffer_size, unsigned int *p_bytes_read);

