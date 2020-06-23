/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Wrappers for OS Specific functionality
 */

#include "twOSPort.h"
#include "twDefaultSettings.h"
#include "twHttpProxy.h"
#include "stringUtils.h"

#include "process.h"
#include "ws2tcpip.h"
#include "time.h"
#include "sys/timeb.h"
#include "errno.h"
#include "Shlwapi.h"
#include "Strsafe.h"

#define MAX_BUFFER_SIZE 65536
#define MAX_PATH_LEN 260

/* Logging Function */
void LOGGING_FUNCTION( enum LogLevel level, const char * timestamp, const char * message ) {
	printf("[%-5s] %s: %s\n", levelString(level), timestamp, message);
}

/* Time Functions */
/* FILETIME of Jan 1 1970 00:00:00. */

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
	struct _timeb timebuffer;
	_ftime_s( &timebuffer ); /* C4996 */
	return (DATETIME)(timebuffer.time * 1000 + timebuffer.millitm);
}

uint64_t twGetSystemMillisecondCount() {
	return (uint64_t)twGetSystemTime(TRUE);
}

void twGetTimeString(DATETIME time, char * s, const char * format, int length, char msec, char utc) {
	struct tm timeinfo;
	time_t seconds;
	uint32_t mseconds;
	char millisec[8];
	TIME_ZONE_INFORMATION tz;
	mseconds = time % 1000;
	seconds = time / 1000;
	/* Convert this to a tm struct */
	if (utc) _gmtime64_s(&timeinfo, &seconds);
	else _localtime64_s(&timeinfo, &seconds);
	
	if (msec) {
		strftime (s,length - 4,format, &timeinfo);
		/* append the milliseconds */
		memset(millisec, 0, 8);
		_itoa_s(mseconds, millisec, 4, 10);
		strcat_s(s, length, ",");
		strcat_s(s, length, millisec);
	} else strftime (s,length,format,&timeinfo);
}

void twGetSystemTimeString(char * s, const char * format, int length, char msec, char utc) {
	DATETIME t;
	t = twGetSystemTime(utc);
	twGetTimeString(t, s, format, length, msec, utc);
}

void twSleepMsec(int milliseconds) {
	  Sleep(milliseconds);
}

/* Mutex Functions */
TW_MUTEX twMutex_Create() {
	LPHANDLE hMutex = (LPHANDLE)malloc(sizeof(LPHANDLE));
	if (hMutex) *hMutex = CreateMutex( NULL, FALSE, NULL);  
	return hMutex;
}

void twMutex_Delete(TW_MUTEX m) {
	if (m) CloseHandle(*m); 
	TW_FREE(m);
}

void twMutex_Lock(TW_MUTEX m) {
	if (m) WaitForSingleObject(*m, INFINITE); 
}

void twMutex_Unlock(TW_MUTEX m) {
	if (m) ReleaseMutex(*m);
}

/* Socket Functions */
uint32_t openSockets = 0; 
WSADATA wsaData;

twSocket * twSocket_Create(const char * host, int16_t port, uint32_t options) {

   struct addrinfo hints, *p;
   int rval;
   char foundServer = 0;
   twSocket * res = NULL;
   char portStr[10];
   foundServer =1;
   /* Initialize winsock if we need to */
   if (openSockets++ == 0) {
	   if (WSAStartup(MAKEWORD(2, 0), &wsaData)) return NULL;
   }
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
	   hints.ai_family = PF_UNSPEC;
	   hints.ai_socktype = SOCK_STREAM;
	   hints.ai_protocol = IPPROTO_TCP;

		_itoa_s(port, portStr, 9, 10);
		if (rval = getaddrinfo(host, portStr, &hints, &p) != 0) {
			twSocket_Delete(res);
			return NULL;
		}

		/*loop through all the results and connect to the first we can*/
		  res->addrInfo = p;
		for(; p != NULL; p = p->ai_next) {
			if ((res->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET ) {
				continue;
			} else {
				foundServer = 1;
				/*res->addr = p;*/
				memcpy(&res->addr, p, sizeof(struct addrinfo));
				break;
			}
		}
		if (!foundServer) {
			twSocket_Delete(res);
			return NULL;
		}
   }
	res->state = CLOSED;
	TW_LOG(TW_TRACE, "twSocket_Create: creating socket: %d, socket info - host: %s, port: %d, proxyHost: %s, proxyPort: %d, state: %d ", res->sock, res->host ? res->host : "NULL", res->port, res->proxyHost ? res->proxyHost : "NULL", res->proxyPort, res->state);

	return res;
}

int twSocket_Connect(twSocket * s) {
	int res;
	char i = 1;
	if (!s) return -1;
	TW_LOG(TW_TRACE, "twSocket_Connect: connecting socket: %d, socket info - host: %s, port: %d, proxyHost: %s, proxyPort: %d, state: %d ", s->sock, s->host ? s->host : "NULL", s->port, s->proxyHost ? s->proxyHost : "NULL", s->proxyPort, s->state);
	/*setsockopt(s->sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));*/
	if (res = connect(s->sock, s->addr.ai_addr, s->addr.ai_addrlen) == -1) {
		int err = 0;
		closesocket(s->sock);
		s->sock = INVALID_SOCKET ;
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
	if ((s->sock = socket(s->addr.ai_family, s->addr.ai_socktype, s->addr.ai_protocol)) == INVALID_SOCKET ) {
		return -1;
	}
	return twSocket_Connect(s);
}

int twSocket_Close(twSocket * s) {
	if (!s) return -1;
	TW_LOG(TW_TRACE, "twSocket_Close: closing socket: %d, socket info - host: %s, port: %d, proxyHost: %s, proxyPort: %d, state: %d ", s->sock, s->host ? s->host : "NULL", s->port, s->proxyHost ? s->proxyHost : "NULL", s->proxyPort, s->state);
	closesocket(s->sock);
	s->sock = INVALID_SOCKET;
	s->state = CLOSED;
	return 0;
}

int twSocket_Read(twSocket * s, char * buf, int len, int timeout) {
	int read = 0;
	int res = 0;
	/*int iter = 0;*/
    
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
		/**** printf("\n\n#################################### Error selecting on socket %d.  Error: %d\n\n", s->sock, twSocket_GetLastError()); ***/
		return res;
	}
    if (res == 0) {
		return 0;
	}
	/* Do our read */
	read = recv(s->sock, buf, len, 0);
	/*printf("###DEBUG###\tRcvd Length:\t%d\n###DEBUG###\tData:\t", read);
	for(iter = 0; iter < read; iter++){
		printf("%X ", (unsigned char) buf[iter]);
	}
	printf("\n");*/
	return read;
}

int twSocket_WaitFor(twSocket * s, int timeout) {
    fd_set readfds;
	struct timeval t;
	if (!s) return -1;
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
	return send(s->sock, buf, len, 0);
}

int twSocket_Delete(twSocket * s) {
	if (!s) return -1;
	twSocket_Close(s);
	freeaddrinfo(s->addrInfo);
	if (s->host) TW_FREE(s->host);
	if (s->proxyHost) TW_FREE(s->proxyHost);
	if (s->proxyUser) TW_FREE(s->proxyUser);
	TW_FREE(s);
	if (--openSockets == 0) {
	   WSACleanup();
	}
	return  0;
}

int twSocket_GetLastError() {
	return WSAGetLastError();
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
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		_itoa_s(s->port, portStr, 9, 10);
		if (rval = getaddrinfo(s->host, portStr, &hints, &p) != 0) {
			return TW_INVALID_PARAM;
		}

		/* loop through all the results and connect to the first we can */
		s->addrInfo = p;
		for(; p != NULL; p = p->ai_next) {
			if ((s->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET ) {
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
   char foundServer = 0;
   char portStr[10];
   foundServer = 1;

	if (!s || !proxyHost || proxyPort == 0) return TW_INVALID_PARAM;
	temp = duplicateString(proxyHost);
	if (!temp) {
		return TW_ERROR_ALLOCATING_MEMORY;
	}
   /* Check the proxy address */
   memset(&hints, 0x00, sizeof(hints));
   hints.ai_family = PF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;

	_itoa_s(proxyPort, portStr, 9, 10);
	if (rval = getaddrinfo(proxyHost, portStr, &hints, &p) != 0) {
		return TW_INVALID_PARAM;
	}
	/* Clean up the old address info */
	twSocket_Close(s);
	freeaddrinfo(s->addrInfo);

    /* loop through all the results and connect to the first we can */
    s->addrInfo = p;
    for(; p != NULL; p = p->ai_next) {
        if ((s->sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        } else {
			foundServer = 1;
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

/* Tasker Functions */
HANDLE tickTimeHandle = 0;
HANDLE tickSignal = 0;
int32_t tickRate = TICKS_PER_MSEC;
unsigned int thread_id = 0;

extern void tickTimerCallback (void * params); /* Defined in tasker.c */

unsigned int _stdcall  WindowsTimerThread(PVOID lpParameter) {	
	while (WaitForSingleObject(tickSignal, 0 )) {
		tickTimerCallback(0);
		twSleepMsec(*(int *)lpParameter);
	}
	thread_id = 0;
	TW_LOG(TW_ERROR, "twWindows.c:WindowsTimerThread is exiting!!!!");
	return 0;
}

void twTasker_Start() {
	
	void * event_name = NULL;
	event_name = TW_MALLOC(MAX_PATH);
	if(event_name==NULL){
		TW_LOG(TW_ERROR, "twWindows.c:twTaskerStart error allocating memory");
		return;
	}
	twGetSystemTimeString((char*)event_name, "%Y-%m-%d %H:%M:%S", MAX_PATH, TRUE, TRUE);
	tickSignal = CreateEvent( 
			NULL,               /* default security attributes */
			TRUE,               /* manual-reset event */
			FALSE,              /* initial state is nonsignaled */
			(LPCWSTR)event_name				/* object name */
        ); 
	TW_FREE(event_name);
	tickTimeHandle = (HANDLE)_beginthreadex(0, 0, &WindowsTimerThread, &tickRate, 0, &thread_id);
}

void twTasker_Stop() {
	SetEvent(tickSignal);
	while (thread_id) {
		twSleepMsec(tickRate);
	}
	CloseHandle(tickSignal);
	tickSignal = 0;
}

/* File Transfer */
#include <sys/stat.h>
#include "stringUtils.h"

wchar_t * convertToWide( const char * in) {
	wchar_t * tmp = NULL;
	size_t outlen = 0;
	if (!in) return 0;
	/* Get the required number of characters in the buffer */
	outlen = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
	if (!outlen) return NULL;
	tmp = (wchar_t *)TW_CALLOC(outlen + 1, 2);
	if (!tmp) return NULL;
	outlen = MultiByteToWideChar(CP_UTF8, 0, in, -1, tmp, outlen);
    if (outlen >=0) {
		return tmp;
	}
	TW_FREE(tmp);
	return NULL;
}

char * convertToMb(const wchar_t * in) {
	char * tmp = NULL;
	size_t outlen = 0;
	size_t bufferSize = 0;
	
	if (!in) return NULL;
	/* Get the required number of characters in the buffer */
	StringCbLengthW(in, MAX_BUFFER_SIZE, &bufferSize);
	if(!bufferSize) return NULL;
	
	outlen = WideCharToMultiByte(CP_UTF8, 0, in, bufferSize, NULL, 0, NULL, NULL);
	if (!outlen) return NULL;
	tmp = (char *)TW_CALLOC(outlen + 1, 1);
	if (!tmp) return NULL;
	outlen = WideCharToMultiByte(CP_UTF8, 0, in, bufferSize, tmp, outlen, NULL, NULL);
	if (outlen >= 0) {
		return tmp;
	}
	TW_FREE(tmp);
	return NULL;
}

TW_FILE_HANDLE win_fopen(const char* name, const char* mode){
	TW_FILE_HANDLE result = NULL;
	wchar_t * w_name = NULL;
	wchar_t * w_mode = NULL;
	w_name = convertToWide(name);
	if (!w_name) return result;
	w_mode = convertToWide(mode);
	if (!w_mode){
		TW_FREE(w_name);
		return result;
	}
	result = _wfopen(w_name, w_mode);
	TW_FREE(w_name);
	TW_FREE(w_mode);
	return result;
}

TW_DIR twDirectory_IterateEntries(char * dirName, TW_DIR dir, char ** name, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly) {
	/* Variable declarations */
    /* Note: We have to use the wide-form of WIN32_FIND_DATA, FindFirstFile() and FindNextFile() for this to work
     * correctly. Otherwise heap corruption can happen and the SDK quickly crashes thereafter. See CSDK-1004. */
	WIN32_FIND_DATAW ffd;
	uint64_t filetime = 0;
	/* Parameter check */
	if ((!dir && !dirName) || !name || !size || !lastModified || !isDirectory || !isReadOnly) return 0;
	/* If dir is NULL this is the first pass through and we need to get the first file */
	if (!dir) {
		size_t len = 0;
		char * tmp = NULL;
		wchar_t * n = NULL;
        HANDLE hFind = 0;

		/* Make sure the directory ends with '\*' */
		StringCbLength(dirName, MAX_PATH_LEN, &len);	
		if(!len) return NULL;
        if (1 < len && '*' != dirName[len - 1]) {
            if ('\\' == dirName[len - 1]) {
                /* Concatenate dirName with '*' in tmp */
                tmp = (char *)TW_CALLOC(len + 2, 1);
                if (!tmp) return 0;
                strncpy(tmp, dirName, len+2);
                tmp[len] = '*';
            }
            else {
                /* Concatenate dirName with '\*' in tmp */
                tmp = (char *)TW_CALLOC(len + 3, 1);
                if (!tmp) return 0;
                strcpy(tmp, dirName);
                tmp[len] = '\\';
                tmp[len + 1] = '*';
            }
        }

		/* Find the first file in the directory. */
		n = convertToWide(tmp);
		hFind = FindFirstFileW(n, &ffd);
		TW_FREE(n);

		if (tmp) TW_FREE(tmp);
		if (INVALID_HANDLE_VALUE == hFind)  {
			return 0;
		} 
		dir = hFind;
	} else {
		if (!FindNextFileW(dir, &ffd)) {
			FindClose(dir);
			return 0;
		}
	}
	/* Fill in the file info details */
	filetime = ((uint64_t)ffd.ftLastWriteTime.dwHighDateTime << 32) + ffd.ftLastWriteTime.dwLowDateTime;
	*lastModified = (DATETIME)(filetime / 10000 - 11644473600000);
	/* Need to convert back to c string */
	*name = convertToMb(ffd.cFileName);
	*isReadOnly = (ffd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? TRUE : FALSE;
	*size = ((uint64_t)ffd.nFileSizeHigh << 32) + ffd.nFileSizeLow;
	*isDirectory = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
	return dir;
}

int twDirectory_GetFileInfo(char * filename, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly) {
	struct _stat64 s;
	wchar_t * n = NULL;
	n = convertToWide(filename);
	
	if (!n || !size || !lastModified || !isDirectory || !isReadOnly) return TW_INVALID_PARAM;
	if (!_wstat64(n,&s))  {
		*size = s.st_size;
		*lastModified = s.st_mtime * 1000;
		*isDirectory = (s.st_mode & _S_IFDIR) ? TRUE : FALSE ;
		*isReadOnly = (s.st_mode & S_IWRITE) ? FALSE : TRUE;
		TW_FREE(n);
		return 0;
	}
	TW_FREE(n);
	return GetLastError();
}

char twDirectory_FileExists(char * name) {
	struct _stat64 s ;
	wchar_t * n = NULL;
	n = convertToWide(name);
	
	if (!n) return FALSE;
	if (!_wstat64(n,&s)) {
		TW_FREE(n);
		return TRUE;
	}
	TW_FREE(n);
	return FALSE;
}

int twDirectory_CreateFile(char * name) {
	HANDLE res = 0;
	wchar_t * n = NULL;
	n = convertToWide(name);
	if (!n) return TW_INVALID_PARAM;
    res = CreateFileW(n, GENERIC_WRITE | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  
	TW_FREE(n);
    if (res == INVALID_HANDLE_VALUE) { 
        return GetLastError();
    } 
	CloseHandle(res);
	return 0;
}

int twDirectory_MoveFile(char * fromName, char * toName) {
	int res = 0;
	wchar_t * from = NULL; 
	wchar_t * to = NULL;

	from = convertToWide(fromName);
	if (!from) return TW_INVALID_PARAM;
	to = convertToWide(toName);
	if (!to){
		TW_FREE(from);
		return TW_INVALID_PARAM;
	}
	res = MoveFileExW(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	TW_FREE(from);
	TW_FREE(to);
	return res ? 0 : GetLastError();
}

int twDirectory_CopyFile(char * fromName, char * toName) {
	int res = 0;
	wchar_t * from = NULL; 
	wchar_t * to = NULL;

	from = convertToWide(fromName);
	if (!from) return TW_INVALID_PARAM;
	to = convertToWide(toName);
	if (!to){
		TW_FREE(from);
		return TW_INVALID_PARAM;
	}
	res = CopyFileExW(from, to, NULL,NULL,FALSE,0);
	TW_FREE(from);
	TW_FREE(to);
	return res ? 0 : GetLastError();
}

int twDirectory_DeleteFile(char * name) {
	int res = 0;
	wchar_t * n = convertToWide(name);
	if (!n) return TW_INVALID_PARAM;
	res = DeleteFileW(n);
	TW_FREE(n);
	return res ? 0 : GetLastError();
}

int twDirectory_CreateDirectory(char * name) {
	int res = 0;
	wchar_t * n = convertToWide(name);
	wchar_t opath[MAX_PATH]; 
	wchar_t *p = NULL; 
	size_t len; 
	if (!n) return TW_INVALID_PARAM;

	/* If the directory already exists, nothing to do */
	if (twDirectory_FileExists(name)) {
		TW_FREE(n);
		return 0;
	}
	memset(opath, 0, MAX_PATH);
    wcsncpy(opath, n, MAX_PATH-1);
	TW_FREE(n);
    len = wcslen(opath); 
	if (len == 0) return TW_INVALID_PARAM;
    /* Remove any trailing delimeter*/
    if (opath[len - 1] == L'/' || opath[len - 1] == L'\\') opath[len - 1] = L'\0'; 
	/* Walk through the path */
    for(p = opath; *p; p++) {
        if(*p == L'/' || *p == L'\\') {
           *p = L'\0';
				if (wcslen(opath) && GetFileAttributesW(opath) == -1) {
					if (!CreateDirectoryW(opath, 0)) {
						if (GetLastError() != ENOENT) {
							return GetLastError();
						}
					}
				}
           *p = L'\\';
        }
	}   
	/* Now we can add the desired directory */  
	if (!CreateDirectoryW(opath, 0)){
		res = GetLastError();
		return (res == ERROR_ALREADY_EXISTS) ? 0 : res;
	}
	return 0;
}

int twDirectory_DeleteDirectory(char * name) {
	int res = 0;
	char isEmpty = FALSE;
	char foundFile = FALSE;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = NULL;
	DWORD dwError;
	size_t name_length;
	size_t fixed_name_length;
	char * fixed_name = NULL;
	char * wildcard_name = NULL;
	char * found_item = NULL;

	wchar_t * n = NULL;

	/* wide char string */
	n = convertToWide(name);
	if (!n) return TW_INVALID_PARAM;

	/* get string length for later use */
	StringCbLength(name, MAX_PATH_LEN, &name_length);	
	if(!name_length) return TW_INVALID_PARAM;

	/* duplicate string and append \ if needed */
	if (name_length > 2 && name[name_length-1] == '\\') {
		/* string already has correct end, just duplicate */
		fixed_name = duplicateString(name);
	} else {
		/* append \ to string */
		fixed_name = (char*)TW_CALLOC(name_length + 2, 1); /* 2 extra bytes for NULL terminator and appended '\' */
		sprintf(fixed_name, "%s\\", name);
	}

	/* get fixed name length for later use */
	StringCbLength(fixed_name, MAX_PATH_LEN, &fixed_name_length);	
	if(!fixed_name_length) return TW_INVALID_PARAM;
	
	/* duplicate string and add three bytes for \\* so we can filter all entries in the directory */
	wildcard_name = (char*)TW_CALLOC(fixed_name_length + 2, 1);  /* 2 extra bytes for NULL terminator and appended '*' */
	sprintf(wildcard_name, "%s*", fixed_name);

	/* Find the first file in the directory. */
	hFind = FindFirstFile(wildcard_name, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		res = GetLastError();
	} 
	else
	{
		/* delete the file */
		do {
			if (FindFileData.cFileName[0] != '.') {
				/* create new string with new file or dir path */
				size_t cFileNameLen = 0;
				StringCbLength(FindFileData.cFileName, MAX_PATH_LEN, &cFileNameLen);	
				if(!cFileNameLen) {
					TW_FREE(wildcard_name);
					return TW_INVALID_PARAM;
				}
				
				found_item = (char*)TW_CALLOC(fixed_name_length + cFileNameLen + 1, 1); /* +1 for NULL terminator */
				sprintf(found_item, "%s%s", fixed_name, FindFileData.cFileName);

				if ((FindFileData.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
					/* recursivly delete the directory */
					twDirectory_DeleteDirectory(found_item);
				} else {
					/* delete the file */
					twDirectory_DeleteFile(found_item);
				}

				/* cleanup and NULL memory for next run */
				TW_FREE(found_item);
				found_item = NULL;
			}
		}
		while (FindNextFile(hFind, &FindFileData));

		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			res = GetLastError();
		}
	}


	/* recursively delete and directories in any subdirectories */

	/* delete the directory */
	res = RemoveDirectoryW(n);
	TW_FREE(n);
	TW_FREE(wildcard_name);
	TW_FREE(fixed_name);
	return res ? 0: GetLastError();
}

int twDirectory_GetLastError() {
	return GetLastError();
}
