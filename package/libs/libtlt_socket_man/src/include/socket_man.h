#ifndef SOCKET_MAN_H
#define SOCKET_MAN_H
#include <libtlt_base/base.h>
// #define AF_UNIX		1	/* Unix domain sockets 		*/
/* includai socketui */
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

#define UNIX_SOCK_PATH "/tmp/gsmd.sock"

#define RX_BUF_LEN 128
#define BUFFER_SIZE 128

void close_socket(int socket_fd);
int open_socket();

#endif
