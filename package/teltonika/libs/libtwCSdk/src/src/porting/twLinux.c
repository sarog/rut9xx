/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Wrappers for OS Specific functionality
 */

#include "twOSPort.h"
#include "twHttpProxy.h"
#include "stringUtils.h"

#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <resolv.h>

#ifndef OS_IOS
#include <termios.h>
#include <ftw.h>
#endif

/* will be used to copy files during twDirectory_CopyFile and twDirectory_MoveFile */
#define TW_COPY_BUF_SIZE 8192

#ifndef __res_init
#define __res_init() res_init()
#endif

/* Logging Function */
void LOGGING_FUNCTION( enum LogLevel level, const char * timestamp, const char * message ) {
	printf("[%-5s] %s: %s\n", levelString(level), timestamp, message);
}

/* Time Functions */
char twTimeGreaterThan(DATETIME t1, DATETIME t2) {
	return (t1 > t2);
}

char twTimeLessThan(DATETIME t1, DATETIME t2) {
	return (t1 < t2);
}

DATETIME twAddMilliseconds(DATETIME t1, int32_t msec) {
	return t1 + msec;
}

DATETIME twGetSystemTime(char utc) {
	struct timeb timebuffer;
	ftime( &timebuffer );
	return ((DATETIME)timebuffer.time * 1000 + timebuffer.millitm);
}

uint64_t twGetSystemMillisecondCount() {
	return (uint64_t)twGetSystemTime(TRUE);
}

void twGetTimeString(DATETIME time, char * s, const char * format, int length, char msec, char utc) {
	struct tm timeinfo;
	time_t seconds;
	uint32_t mseconds;
	char millisec[8];
	mseconds = time % 1000;
	seconds = time / 1000;
	/* Convert this to a tm struct */
	if (utc) {
		localtime_r(&seconds, &timeinfo);
	}
	else {
		gmtime_r(&seconds, &timeinfo);
	}
	if (msec) {
		strftime (s,length - 4,format, &timeinfo);
		/* append the milliseconds */
		memset(millisec, 0, 8);
		sprintf(millisec,"%u", mseconds);
		if (strnlen(s, length) < length - 9) {
			strncat(s, ",", 1);
			strncat(s, millisec, 8);
		}
	} else strftime (s,length,format,&timeinfo);
}

void twGetSystemTimeString(char * s, const char * format, int length, char msec, char utc) {
	DATETIME t;
	t = twGetSystemTime(utc);
	twGetTimeString(t, s, format, length, msec, utc);
}

void twSleepMsec(int milliseconds) {
	usleep(milliseconds * 1000);
}

/* Mutex Functions */
TW_MUTEX twMutex_Create() {
	pthread_mutex_t * tmp = TW_MALLOC(sizeof(pthread_mutex_t));
	if (!tmp) return 0;
	pthread_mutex_init(tmp,0);
	return tmp;
}

void twMutex_Delete(TW_MUTEX m) {
	pthread_mutex_t * tmp = m;
	if (!tmp) return;
	m = 0;
	pthread_mutex_destroy(tmp);
	free(tmp);
}

void twMutex_Lock(TW_MUTEX m) {
	if (m) pthread_mutex_lock(m);
}

void twMutex_Unlock(TW_MUTEX m) {
	if (m) pthread_mutex_unlock (m);
}


/* Socket Functions */
int twPosixSocket_Create(int domain, int socktype, int protocol) {
	int sock = socket(domain, socktype, protocol);
#ifdef OS_IOS
	int value = 1;
	if (sock != -1) { setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value)); }
#endif
	return sock;
}

twSocket * twSocket_Create(const char * host, int16_t port, uint32_t options) {

	struct addrinfo hints, *p;
	int rval;
	char foundServer = 1;
	twSocket * res = NULL;
	char portStr[10];

	/* Allocate our twSocket */
	res = (twSocket *)malloc(sizeof(twSocket));
	if (!res) return 0;
	memset(res, 0, sizeof(twSocket));

	/* Set up our address structure and any proxy info */
	res->host = duplicateString(host);
	res->port = port;
	/* If we we have a host, try to resolve it */
	if ((host && strcmp(host,"")) && port) {
		memset(&hints, 0x00, sizeof(hints));
		hints.ai_family = TW_HINTS;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		snprintf(portStr, 10, "%u", port);
		if ((rval = getaddrinfo(host, portStr, &hints, &p)) != 0) {
			if(rval == EAI_AGAIN) {
				TW_LOG(TW_WARN, "getaddrinfo() returned EAI_AGAIN, re-initializing resolver");
				__res_init();
			}
			free(res);
			return NULL;
		}

		/* loop through all the results and connect to the first we can */
		res->addrInfo = p;
		for(; p != NULL; p = p->ai_next) {
			if ((res->sock = twPosixSocket_Create(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				continue;
			} else {
				foundServer = 1;
				memcpy(&res->addr, p, sizeof(struct addrinfo));
				break;
			}
		}
		if (!foundServer) {
			free(res);
			return NULL;
		}
	}
	res->state = CLOSED;
	return res;
}

int twSocket_Connect(twSocket * s) {
	int res;
	if (!s) return -1;

	if ((res = connect(s->sock, s->addr.ai_addr, s->addr.ai_addrlen)) == -1) {
		int err = 0;
		close(s->sock);
		s->sock = -1 ;
		err = twSocket_GetLastError();
		return err ? err : -1;
	}
#ifdef ENABLE_HTTP_PROXY_SUPPORT
	if (s->proxyHost && s->proxyPort > 0) {
		res = connectToProxy(s, NULL);
		if (res) {
			return res;
		}
	}
#endif
	s->state = OPEN;
	return res;
}

int twSocket_Reconnect(twSocket * s) {
	if (!s) return -1;
	twSocket_Close(s);
	if ((s->sock = twPosixSocket_Create(s->addr.ai_family, s->addr.ai_socktype, s->addr.ai_protocol)) == -1) {
		return -1;
	}
	return twSocket_Connect(s);
}

int twSocket_Close(twSocket * s) {
	if (!s) return -1;

    //If socket is not opened don't close it
    if (s->sock < 0){
        /*TW_LOG(TW_DEBUG, "twSocket_Close: s->sock ALREADY CLOSED");*/
        return -1;
    }

	close(s->sock);
	s->sock = -1;
	s->state = CLOSED;
	return 0;
}

int twSocket_Read(twSocket * s, char * buf, int len, int timeout) {
	int read = 0;
	int res = 0;
	fd_set readfds;
	struct timeval t;
	if (!s) return -1;
	/* Check for data so we don't block */
	FD_ZERO(&readfds);
	FD_SET(s->sock, &readfds);
	t.tv_sec = timeout / 1000;
	t.tv_usec = (timeout % 1000) * 1000;
	res = select(FD_SETSIZE, &readfds, 0, 0, (timeout < 0) ? 0 : &t);
	if (res < 0) {
		/*** printf("\n\n#################################### Error selecting on socket %d.  Error: %d\n\n", s->sock, twSocket_GetLastError()); ***/
		return res;
	}
	if (res == 0) {
		return 0;
	}
	/* Do our read */
	read = recv(s->sock, buf, len, 0);
	/*** TW_LOG_HEX(buf, "Rcvd Packet: ", read); ***/
	return read;
}

int twSocket_WaitFor(twSocket * s, int timeout) {
	fd_set readfds;
	struct timeval t;
	if (!s) return -1;
	if (-1 == s->sock)
		return -1;
	/* Check for data so we don't block */
	FD_ZERO(&readfds);
	FD_SET(s->sock, &readfds);
	t.tv_sec = timeout / 1000;
	t.tv_usec = (timeout % 1000) * 1000;
	if (select(FD_SETSIZE, &readfds, 0, 0, (timeout < 0) ? 0 : &t) <= 0) return 0;
	return 1;
}

int twSocket_Write(twSocket * s, char * buf, int len, int timeout) {
	if (!s) return -1;
	/*** TW_LOG_HEX(buf, "Sent Packet: ", len);  ***/
	return send(s->sock, buf, len, MSG_NOSIGNAL);
}

int twSocket_Delete(twSocket * s) {
	if (!s) return -1;
	twSocket_Close(s);
	freeaddrinfo(s->addrInfo);
	if (s->host) TW_FREE(s->host);
	if (s->proxyHost) TW_FREE(s->proxyHost);
	if (s->proxyUser) TW_FREE(s->proxyUser);
	free(s);
	return  0;
}

int twSocket_ClearProxyInfo(twSocket * s) {
	struct addrinfo hints, *p;
	int rval;
	char foundServer = 0;
	char portStr[10];
	foundServer = 1;

	if (!s) return TW_INVALID_PARAM;
	/* CLear out the proxy info */
	if (s->state != CLOSED) twSocket_Close(s);
	if (s->proxyHost) TW_FREE(s->proxyHost);
	if (s->proxyUser) TW_FREE(s->proxyUser);

	s->proxyHost = NULL;
	s->proxyPort = 0;
	s->proxyUser = NULL;
	s->proxyPassCallback = NULL;

	/* Create the new socket */
	if ((s->host && strcmp(s->host,"")) && s->port) {
		memset(&hints, 0x00, sizeof(hints));
		hints.ai_family = TW_HINTS;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		snprintf(portStr, 10, "%u", s->port);
		if ((rval = getaddrinfo(s->host, portStr, &hints, &p)) != 0) {
			return TW_INVALID_PARAM;
		}
		/* loop through all the results and connect to the first we can */
		if(s->addrInfo) freeaddrinfo(s->addrInfo);
		s->addrInfo = p;
		for(; p != NULL; p = p->ai_next) {
			if ((s->sock = twPosixSocket_Create(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				continue;
			} else {
				foundServer = 1;
				memcpy(&s->addr, p, sizeof(struct addrinfo));
				break;
			}
		}
		if (!foundServer) {
			return TW_INVALID_PARAM;
		}
	}
	s->state = CLOSED;
	return TW_OK;
}


int twSocket_SetProxyInfo(twSocket * s, char * proxyHost, uint16_t proxyPort, char * proxyUser, twPasswdCallbackFunction proxyPassCallback) {

	struct addrinfo hints, *p;
	int rval;
	char * temp = 0;
	char portStr[10];

	if (!s || !proxyHost || proxyPort == 0) return TW_INVALID_PARAM;
	temp = duplicateString(proxyHost);
	if (!temp) {
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	/* Check the proxy address */
	memset(&hints, 0x00, sizeof(hints));
	hints.ai_family = TW_HINTS;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	snprintf(portStr, 9, "%d", proxyPort);
	if ((rval = getaddrinfo(proxyHost, portStr, &hints, &p)) != 0) {
		return TW_INVALID_PARAM;
	}
	/* Clean up the old address info */
	twSocket_Close(s);
	freeaddrinfo(s->addrInfo);

	/* loop through all the results and connect to the first we can */
	s->addrInfo = p;
	for(; p != NULL; p = p->ai_next) {
		if ((s->sock = twPosixSocket_Create(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			continue;
		} else {
			memcpy(&s->addr, p, sizeof(struct addrinfo));
			break;
		}
	}
	s->proxyHost = temp;
	s->proxyPort = proxyPort;
	if (proxyUser) {
		s->proxyUser = duplicateString(proxyUser);
		if (!s->proxyUser) {
			return TW_ERROR_ALLOCATING_MEMORY;
		}
	}
	if (proxyPassCallback) {
		s->proxyPassCallback = proxyPassCallback;
		if (!s->proxyPassCallback) {
			return TW_ERROR_ALLOCATING_MEMORY;
		}
	}
	return 0;
}

int twSocket_GetLastError() {
	return errno;
}

/* Tasker Functions */
pthread_t tickTimerThread = 0;
char tickSignal = 0;
int32_t tickRate = TICKS_PER_MSEC;
unsigned int thread_id = 0;

extern void tickTimerCallback (void * params); /* Defined in tasker.c */

void * TimerThread(void * params) {
	while (!tickSignal) {
		tickTimerCallback(0);
		twSleepMsec(*(int *)params);
	}
	thread_id = 0;
	return 0;
}

void twTasker_Start() {
	tickSignal = 0;
	pthread_create(&tickTimerThread, NULL, TimerThread, (void *)&tickRate);
}

void twTasker_Stop() {
	void * status;
	tickSignal = 1;
	pthread_join(tickTimerThread, &status);
}

#ifndef OS_IOS
/* getch */
static struct termios old, new;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	new = old; /* make new settings same as old settings */
	new.c_lflag &= ~ICANON; /* disable buffered i/o */
	new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
	tcsetattr(0, TCSANOW, &old);
}

char getch()
{
	char ch;
	initTermios(0);
	ch = getchar();
	resetTermios();
	return ch;
}

int twDirectory_GetFileInfo(char * filename, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly) {
	struct stat64 s ;
	TW_LOG(TW_TRACE, "Getting file info for %s", filename);
	if (!filename || !size || !lastModified || !isDirectory || !isReadOnly) return TW_INVALID_PARAM;
	if (!stat64(filename,&s))  {
		*size = s.st_size;
		*lastModified = ((DATETIME)s.st_mtime) * 1000;
		*isDirectory = S_ISDIR(s.st_mode );
		*isReadOnly = (s.st_mode & S_IWRITE) ? FALSE : TRUE;
		return 0;
	}
	return errno;
}

TW_DIR twDirectory_IterateEntries(char * dirName, TW_DIR dir, char ** name, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly) {
	/* Variable declarations */
	struct dirent * entry = NULL;
	char * tmp = NULL;
	size_t len = 0;
	char * fullpath = NULL;
	int res = 0;
	/* Parameter check */
	if (!dirName || !name || !size || !lastModified || !isDirectory || !isReadOnly) return 0;
	/* Make sure the directory ends with '/' */
	len = strnlen(dirName, PATH_MAX);
	if (len && dirName[len - 1] != '/') {
		tmp = (char *)TW_CALLOC(len + 2, 1);
		if (!tmp) {
			TW_LOG(TW_TRACE, "Unable to allocate memory for directory name.");
			return 0;
		}
		strcpy(tmp,dirName);
		tmp[len + 1] = '/';
		dirName = tmp;
	}
	/* If dir is NULL this is the first pass through and we need to open the directory and get the first file */
	if (!dir) {
		/* Need to open the directory */
		TW_LOG(TW_TRACE, "Opening directory, %s", dirName);
		dir = opendir(dirName);
		if (!dir) {
			TW_LOG(TW_ERROR, "Unable to open directory, %s", dirName);
			if (tmp) TW_FREE(tmp);
			return 0;
		}
	}
	if ((entry = readdir(dir)) == NULL) {
		TW_LOG(TW_TRACE, "No more entries. Closing directory, %s", dirName);
		closedir(dir);
		if (tmp) TW_FREE(tmp);
		return 0;
	}
	/* Fill in the file info details by creating the fullPath getting the file info */
	fullpath = (char *) TW_CALLOC(strnlen(dirName, PATH_MAX) + strnlen(entry->d_name, PATH_MAX) + 2, 1);
	if (!fullpath) {
		TW_LOG(TW_ERROR, "Unable to allocate memory for full directory path. Closing directory, %s", dirName);
		closedir(dir);
		if (tmp) TW_FREE(tmp);
		return 0;
	}
	strcpy(fullpath,dirName);
	strcat(fullpath,"/");
	strcat(fullpath,entry->d_name);
	TW_FREE(tmp);
	res = twDirectory_GetFileInfo(fullpath, size, lastModified, isDirectory, isReadOnly);
	TW_FREE(fullpath);
	if (res) {
		TW_LOG(TW_TRACE, "Could not get file info (errno=%d). Closing directory, %s, ", res, dirName);
		closedir(dir);
		return 0;
	}
	*name = duplicateString(entry->d_name);
	return dir;
}

char twDirectory_FileExists(char * name) {
	struct stat64 s ;
	if (!name) return FALSE;
	if (!stat64(name,&s)) {
		return TRUE;
	}
	return FALSE;
}

int twDirectory_CreateFile(char * name) {
	FILE * res = NULL;
	if (!name) return TW_INVALID_PARAM;
	res = fopen(name, "w+");
	if (res == 0) {
		return errno;
	}
	fclose(res);
	return 0;
}

int twDirectory_CopyFile(char * fromName, char * toName) {
	int res = 0;

	/* create variable necessary to read and write the file */
	FILE* in_file = NULL;
	FILE* out_file = NULL;
	size_t result = 0;
	char buf[TW_COPY_BUF_SIZE];

	if (!fromName || !toName) return TW_INVALID_PARAM;

	/* start by deleting the destination file if necessary */
	twDirectory_DeleteFile(toName);

	/* open the input and output files */
	if ( !(in_file = fopen(fromName, "r+")) ) {
		 res = errno;
	} else if ( !(out_file = fopen(toName, "w+")) ) {
		res = errno;
	} else {
		/*
		read the file into the buffer, write the buffer out to the new file,
		repeat until all bytes are written
		*/
		while (0 < (result = fread(buf, 1, TW_COPY_BUF_SIZE, in_file)) ) {
			if( 0 > (result = fwrite(buf, 1, result, out_file)) ) {
				res = errno;
				break;
			}
		}
	}

	/* cleanup buffer and files */
	if (in_file) fclose(in_file);
	if (out_file) fclose(out_file);
	return res;
}

int twDirectory_MoveFile(char * fromName, char * toName) {
	int res = 0;
	if (!fromName || !toName) return TW_INVALID_PARAM;
	twDirectory_DeleteFile(toName);
	/* try to just rename first, because this is faster than a full copy */
	res = rename(fromName, toName);
	/* .if the rename fails, attempt to copy and delete before reporting an error */
	if (res) {
		res = twDirectory_CopyFile(fromName, toName);
		/* not deleting the file, until we verify that the copy succeeds */
		if (!res) {
			res = twDirectory_DeleteFile(fromName);
		}
	}
	return res ? errno : 0;
}

int twDirectory_DeleteFile(char * name) {
	int res = 0;
	if (!name) return TW_INVALID_PARAM;
	res = remove(name);
	return res ? errno : 0;
}

int twDirectory_CreateDirectory(char * name) {
	char opath[PATH_MAX];
	char *p;
	size_t len;

	/* if name pointer is null return an error */
	if (!name) {
		return TW_INVALID_PARAM;
	}

	/* If the directory already exists, nothing to do */
	if (twDirectory_FileExists(name)) {
		return 0;
	}

	/* make a copy of name, so we can modify the memory later  */
	strncpy(opath, name, sizeof(opath));

	/* check that length is valid and wihtin the bounds of the OS */
	len = strnlen(opath, PATH_MAX);
	if (len == 0 || len >= (PATH_MAX)) {
		return TW_INVALID_PARAM;
	}

	/* Walk through the path and add all directories that do not exist*/
	for (p = opath; *p; p++) {
		if (*p == '/' || *p == '\\') {
			*p = '\0';
			if (*opath != '\0' && !twDirectory_FileExists(opath)) {
				if (mkdir(opath, S_IRWXU | S_IRWXG | S_IRWXO)) {
					if (errno != EEXIST) {
						return errno;
					}
				}
			}
			*p = '/';
		}
	}

	/* if trailing delimiter is not present, then add last directory */
	if (*p == '\0' && (*(p-1) != '\\' && *(p-1) != '/')) {
		if (!twDirectory_FileExists(opath)) {
			if (mkdir(opath, S_IRWXU | S_IRWXG | S_IRWXO)) {
				if (errno != EEXIST) {
					return errno;
				}
			}
		}
	}

	return 0;
}

int twDirectory_Unlink_Callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	int res = 0;
	res = remove(fpath);
	if (res)
		perror(fpath);
	return res;
}

int twDirectory_DeleteDirectory(char * name) {
	int res = 0;
	if (!name) return TW_INVALID_PARAM;
	res = nftw(name, twDirectory_Unlink_Callback, 64, FTW_DEPTH | FTW_PHYS);
	return res ? errno : 0;
}

int twDirectory_GetLastError() {
	return errno;
}

#endif

