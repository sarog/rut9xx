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
#include "twPasswds.h"

#define MAX_PATH_LEN 4096

#ifndef OS_IOS
#include <termios.h>
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

/* Making this global is not optimal but GetLastError has no params */
CFErrorRef iosSocketErrCode = NULL;

void iosReadStreamCallback(CFReadStreamRef stream, CFStreamEventType type, void *clientCallBackInfo)  {
	if (!clientCallBackInfo) return;
	iosStream * s = (iosStream *)clientCallBackInfo;
    switch(type) {
		/***** Not sure we want to handle the data now - it can get a bit complicated *****/
        case kCFStreamEventHasBytesAvailable:
            break;
		case kCFStreamEventCanAcceptBytes:
			break;
        case kCFStreamEventErrorOccurred:
			s->parent->state = CLOSED;
			setLastErrorFromReadStream(stream);
            CFReadStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(),
                                              kCFRunLoopCommonModes);
            CFReadStreamClose(stream);
            CFRelease(stream);
			s->readStream = NULL;
            break;
        case kCFStreamEventEndEncountered:
			s->parent->state = CLOSED;
            CFReadStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(),
                                              kCFRunLoopCommonModes);
            CFReadStreamClose(stream);
			CFRelease(stream);
			s->readStream = NULL;
            break;
    }
}

void iosWriteStreamCallback(CFWriteStreamRef stream, CFStreamEventType type, void *clientCallBackInfo)  {
	if (!clientCallBackInfo) return;
	iosStream * s = (iosStream *)clientCallBackInfo;
	switch(type) {
			/***** Not sure we want to handle the data now - it can get a bit complicated *****/
		case kCFStreamEventHasBytesAvailable:
			break;
		case kCFStreamEventCanAcceptBytes:
			break;
		case kCFStreamEventErrorOccurred:
			s->parent->state = CLOSED;
			setLastErrorFromWriteStream(stream);
			CFWriteStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(),
											  kCFRunLoopCommonModes);
			CFWriteStreamClose(stream);
			CFRelease(stream);
			s->writeStream = NULL;
			break;
		case kCFStreamEventEndEncountered:
			s->parent->state = CLOSED;
			CFWriteStreamUnscheduleFromRunLoop(stream, CFRunLoopGetCurrent(),
											  kCFRunLoopCommonModes);
			CFWriteStreamClose(stream);
			CFRelease(stream);
			s->writeStream = NULL;
			break;
	}
}

int createStreams(twSocket * s) {
	CFStringRef hostRef = NULL;
	if (!s || !s->sock) return -1;
	s->sock->parent = s;
	
	if (s->proxyHost) {
		hostRef = cFStringFromUTF8String(s->proxyHost);
		if (!hostRef) { return -1; }
		CFStreamCreatePairWithSocketToHost(NULL, hostRef, s->proxyPort, &(s->sock->readStream), &(s->sock->writeStream));
		CFRelease(hostRef);
	} else {
		hostRef = cFStringFromUTF8String(s->host);
		if (!hostRef) { return -1; }
		CFStreamCreatePairWithSocketToHost(NULL, hostRef, s->port, &(s->sock->readStream), &(s->sock->writeStream));
		CFRelease(hostRef);	
	}
	if (!s->sock->readStream || !s->sock->writeStream) {
		return -1;
	}
	/* Set the stream type to voip so we can run in the background */
	CFReadStreamSetProperty(s->sock->readStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
	CFWriteStreamSetProperty(s->sock->writeStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);

	/* Create a callback context */
	s->sock->ctx.info = &(s->sock);
	
	/* Register our events */
	CFOptionFlags registeredEvents = (kCFStreamEventHasBytesAvailable | kCFStreamEventEndEncountered | kCFStreamEventErrorOccurred); 
	CFReadStreamSetClient(s->sock->readStream, registeredEvents, iosReadStreamCallback, &(s->sock->ctx)) ;
	
	registeredEvents = (kCFStreamEventEndEncountered | kCFStreamEventErrorOccurred | kCFStreamEventCanAcceptBytes); 
	CFWriteStreamSetClient(s->sock->writeStream, registeredEvents, iosWriteStreamCallback, &(s->sock->ctx)) ;

	/* Schedule with run loop */
	 CFReadStreamScheduleWithRunLoop(s->sock->readStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
	 CFWriteStreamScheduleWithRunLoop(s->sock->writeStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

	return TW_OK;
}


twSocket * twSocket_Create(const char * host, int16_t port, uint32_t options) {

   twSocket * res = NULL;
   if (!host || port == 0) {
	   return NULL;
   }
   /* Allocate our twSocket */
   res = (twSocket *)TW_CALLOC(sizeof(twSocket), 1);
   if (!res) return NULL;

   /* Create the iOS Streams */
   res->sock = (iosStream *)TW_CALLOC(sizeof(iosStream), 1);
   if (!res->sock) {
	   twSocket_Delete(res);
	   return NULL;
   }
	
   res->host = duplicateString(host);
   res->port = port;
   res->state = CLOSED;
	
	if (createStreams(res)) {
		twSocket_Delete(res);
		return NULL;
	}
	
   return res;
}

int twSocket_Connect(twSocket * s) {
	int res = 0;
	if (!s || !s->sock || !s->sock->readStream || !s->sock->writeStream) return -1;
	if (s->state == OPEN) {
		/* We are already open */
		return res;
	}
	if (!CFReadStreamOpen(s->sock->readStream)) {
		setLastErrorFromReadStream(s->sock->readStream);
		return -1;
	}
	if (!CFWriteStreamOpen(s->sock->writeStream)) {
		setLastErrorFromWriteStream(s->sock->writeStream);
		return -1;
	}
	if (s->proxyHost && s->proxyPort > 0) {
		res = connectToProxy(s, NULL);
		if (res) {
			return res;
		}
	}	
	s->state = OPEN;
	return res;
}

int twSocket_Reconnect(twSocket * s) {
	if (!s) return -1;
	twSocket_Close(s);
	if (createStreams(s) == -1) {
		return -1;
	}
	return twSocket_Connect(s);
}

int twSocket_Close(twSocket * s) {
	if (!s || !s->sock) return -1;
    if (s->sock->readStream) {
		CFReadStreamUnscheduleFromRunLoop(s->sock->readStream, CFRunLoopGetCurrent(),
                                              kCFRunLoopCommonModes);
        CFReadStreamClose(s->sock->readStream);
        CFRelease(s->sock->readStream);
		s->sock->readStream = 0;
	}
    if (s->sock->writeStream) {
		CFWriteStreamUnscheduleFromRunLoop(s->sock->writeStream, CFRunLoopGetCurrent(),
                                              kCFRunLoopCommonModes);
        CFWriteStreamClose(s->sock->writeStream);
        CFRelease(s->sock->writeStream);
		s->sock->writeStream = 0;
	}
	s->state = CLOSED;
	return 0;
}

int twSocket_Read(twSocket * s, char * buf, int len, int timeout) {
	int read = 0;
	if (!s || !s->sock || !s->sock->readStream || s->state != OPEN) return -1;
	/* Check for data so we don't block */
    if (!twSocket_WaitFor(s, timeout)) return 0;
	/* Do our read */
	read = CFReadStreamRead(s->sock->readStream, buf, len);
    if( read >= 0 ) {
        return read;
    } else {
        setLastErrorFromReadStream(s->sock->readStream);
        s->state = CLOSED;
		return -1;
    } 
}

int twSocket_WaitFor(twSocket * s, int timeout) {
	DATETIME now;
	if (!s || !s->sock || !s->sock->readStream || s->state == CLOSED) return 0;
	now = twGetSystemMillisecondCount();
	while (twGetSystemMillisecondCount() < now + timeout) {
		if (CFReadStreamHasBytesAvailable(s->sock->readStream)) return 1;
		twSleepMsec(1);
	}
	
	return 0;
}

int twSocket_Write(twSocket * s, char * buf, int len, int timeout) {
	if (!s || !s->sock || !s->sock->writeStream || s->state != OPEN) return -1;
	/*** TW_LOG_HEX(buf, "Sent Packet: ", len);  ***/
	if (CFWriteStreamCanAcceptBytes(s->sock->writeStream)) {
		CFIndex bytesWritten = CFWriteStreamWrite(s->sock->writeStream, buf, len);
        if (bytesWritten < 0) {
            setLastErrorFromWriteStream(s->sock->writeStream);
        } else if (bytesWritten == 0) {
            if (CFWriteStreamGetStatus(s->sock->writeStream) == kCFStreamStatusAtEnd)  {
                twSocket_Close(s);
            }
		} else if (bytesWritten != len) {
			/* Figure out what went wrong with the write stream */
            setLastErrorFromWriteStream(s->sock->writeStream);
		} else {
			return bytesWritten;
		}
	} else {
		/* signal that this would block and to try again */
		errno = EWOULDBLOCK;
	}
	
	return -1;
}

int twSocket_Delete(twSocket * s) {
	if (!s) return -1;
	twSocket_Close(s);
	if (s->sock) TW_FREE(s->sock);
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
	twSocket_Close(s);
	if (s->proxyHost) TW_FREE(s->proxyHost);
	if (s->proxyUser) TW_FREE(s->proxyUser);
	if (s->proxyPass) TW_FREE(s->proxyPass);
	s->proxyHost = NULL;
	s->proxyPort = 0;
	s->proxyUser = NULL;
	s->proxyPass = NULL;

	/* Create the new socket */
	return createStreams(s);
}

int twSocket_SetProxyInfo(twSocket * s, char * proxyHost, uint16_t proxyPort, char * proxyUser, twPasswdCallbackFunction proxyPassCallback) {
	
	if (!s || !proxyHost || proxyPort == 0) return TW_INVALID_PARAM;
	/* Clean up the old address  */
	twSocket_Close(s);

    /* Change the host:port to the proxy's */
	s->proxyHost = duplicateString(proxyHost);
	if (!s->proxyHost) {
		return TW_ERROR_ALLOCATING_MEMORY;
	}
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
	return createStreams(s);
}

int twSocket_GetLastError() {
	CFIndex ret = 0;
	if (iosSocketErrCode) {
		ret = CFErrorGetCode(iosSocketErrCode);
		clearLastStreamError();
	}
	return ret;
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
#endif
int twDirectory_GetFileInfo(char * filename, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly) {
	struct stat s ;
	if (!filename || !size || !lastModified || !isDirectory || !isReadOnly) return TW_INVALID_PARAM;
	if (!stat(filename,&s))  {
		*size = s.st_size;
		*lastModified = ((DATETIME)s.st_mtime) * 1000;
		*isDirectory = S_ISDIR(s.st_mode );
		*isReadOnly = (s.st_mode & S_IWRITE) ? FALSE : TRUE;
		return 0;
	}
	return errno;
}

TW_DIR twDirectory_IterateEntries(char * dirName, TW_DIR dir, char ** name, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly) {
	/* Variable decalrations */
	struct dirent * entry = NULL;
	char * tmp = NULL;
	int len = 0;
	char * fullpath = NULL;
	int res = 0;
	/* Parameter check */
	if (!dirName || !name || !size || !lastModified || !isDirectory || !isReadOnly) return 0;
	/* Make sure the directory ends with '/' */
	len = strnlen(dirName, MAX_PATH_LEN);
	if (len && dirName[len - 1] != '/') {
		tmp = (char *)TW_CALLOC(len + 2, 1);
		if (!tmp) return 0;
		strcpy(tmp,dirName);
		tmp[len + 1] = '/';	
		dirName = tmp;
	}
	/* If dir is NULL this is the first pass through and we need to open the directory and get the first file */
	if (!dir) {
        /* Need to open the directory */
		dir = opendir(dirName);
		if (!dir) {
			if (tmp) TW_FREE(tmp);
			return 0;
		}
	} 
	if ((entry = readdir(dir)) == NULL) {
		closedir(dir);
		if (tmp) TW_FREE(tmp);
		return 0;
	}
	/* Fill in the file info details by creating the fullPath getting the file info */
        fullpath = (char *) TW_CALLOC(strnlen(dirName, MAX_PATH_LEN) + strnlen(entry->d_name, MAX_PATH_LEN) + 2, 1);
	if (!fullpath) {
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
		closedir(dir);
		return 0;
	}
	*name = duplicateString(entry->d_name);
	return dir;
}

char twDirectory_FileExists(char * name) {
	struct stat s ;
	if (!name) return FALSE;
	if (!stat(name,&s)) {
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

int twDirectory_MoveFile(char * fromName, char * toName) {
	int res = 0;
	if (!fromName || !toName) return TW_INVALID_PARAM;
	twDirectory_DeleteFile(toName);
        rename(fromName, toName);
	return res ? errno : 0;
}

int twDirectory_DeleteFile(char * name) {
	int res = 0;
	if (!name) return TW_INVALID_PARAM;
	res = remove(name);
	return res ? errno : 0;
}

int twDirectory_CreateDirectory(char * name) {
    char opath[256];
    char *p;
    size_t len;

    if (!name) return TW_INVALID_PARAM;
	/* If the directory already exists, nothing to do */
	if (twDirectory_FileExists(name)) return 0;

    strncpy(opath, name, sizeof(opath));
    len = strnlen(opath, sizeof(opath));
	if (len == 0) return TW_INVALID_PARAM;
    /* Remove any trainling delimeter*/
    if (opath[len - 1] == '/' || opath[len - 1] == '\\') opath[len - 1] = '\0';
	/* Walk through the path */
    for(p = opath; *p; p++) {
        if(*p == '/' || *p == '\\') {
            *p = '\0';
			if (strnlen(opath, sizeof(opath))) {
				if (!twDirectory_FileExists(opath)) {
					if (mkdir(opath, S_IRWXU | S_IRWXG | S_IRWXO)) if (errno != EEXIST) return errno;
				}
			}
            *p = '/';
        }
	}   
	/* Now we can add the desired directory */  
	if (mkdir(name, S_IRWXU | S_IRWXG | S_IRWXO)) return (errno != EEXIST) ? errno : 0;
	return 0;
}

int twDirectory_DeleteDirectory(char * name) {
	int res = 0;
	if (!name) return TW_INVALID_PARAM;
	res = rmdir(name);
	return res ? errno : 0;
}

int twDirectory_GetLastError() {
	return errno;
}

/********** U T I L S **********/

char * stringCopyInUTF8FromCFString(CFStringRef string) {
	if (string == NULL) {
		return NULL;
	}
	
	CFIndex length = CFStringGetLength(string);
	CFIndex maxSize =
	CFStringGetMaximumSizeForEncoding(length,
									  kCFStringEncodingUTF8);
	char *buffer = (char *)malloc(maxSize);
	if (CFStringGetCString(string, buffer, maxSize,
						   kCFStringEncodingUTF8)) {
		return buffer;
	}
	return NULL;
}

CFStringRef cFStringFromUTF8String(char *string) {
	if (!string) { return NULL; }
	return CFStringCreateWithCString(NULL, string, kCFStringEncodingUTF8);
}

void setLastErrorFromReadStream(CFReadStreamRef stream)
{
	clearLastStreamError();
	iosSocketErrCode = CFReadStreamCopyError(stream);
}

void setLastErrorFromWriteStream(CFWriteStreamRef stream)
{
	clearLastStreamError();
	iosSocketErrCode = CFWriteStreamCopyError(stream);
}

void clearLastStreamError()
{
	if (iosSocketErrCode != NULL) {
		CFRelease(iosSocketErrCode);
		iosSocketErrCode = NULL;
	}
}
