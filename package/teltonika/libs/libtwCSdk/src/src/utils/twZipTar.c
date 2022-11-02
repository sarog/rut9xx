#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
#ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #endif
        #ifndef _FILE_OFFSET_BIT
                #define _FILE_OFFSET_BIT 64
        #endif
#endif

#ifdef __APPLE__
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "twExt.h"

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#else
# include <utime.h>
#endif

#include "unzip.h"

#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

enum { TGZ_EXTRACT, TGZ_LIST, TGZ_INVALID };
int tar (gzFile in,int action,int arg,int argc,char **argv);

int makedir (const char* dir);

/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
void change_file_date(const char *filename,uLong dosdate,tm_unz tmu_date){
#ifdef _WIN32
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#else
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#endif
}

int twDoZipExtractCurrentFile(unzFile uf, const int *popt_extract_without_path,
							  const char *password) {
	char filename_inzip[MAXFILENAME];
	char *filename_withoutpath;
	char *p;
	int err = UNZ_OK;
	FILE *fout = NULL;
	void *buf;
	uInt size_buf;

	unz_file_info64 file_info;

	err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

	if (err != UNZ_OK) {
		TW_LOG(TW_ERROR, "error %d with zipfile in unzGetCurrentFileInfo", err);
		return err;
	}

	size_buf = WRITEBUFFERSIZE;
	buf = TW_MALLOC(size_buf);
	if (buf == NULL) {
		TW_LOG(TW_ERROR, "Error allocating memory.");
		return UNZ_INTERNALERROR;
	}

	/* Extract filename from path */
	p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0') {
		if (((*p) == '/') || ((*p) == '\\'))
			filename_withoutpath = p + 1;
		p++;
	}

	/* Are we creating a file or a directory? */
	if ((*filename_withoutpath) == '\0') {
		if ((*popt_extract_without_path) == 0) {
			TW_LOG(TW_DEBUG, "creating directory: %s.", filename_inzip);
			twDirectory_CreateDirectory(filename_inzip);
		}
	} else {
		const char *write_filename;

		if ((*popt_extract_without_path) == 0)
			write_filename = filename_inzip;
		else
			write_filename = filename_withoutpath;

		err = unzOpenCurrentFilePassword(uf, password);
		if (err != UNZ_OK) {
			TW_LOG(TW_ERROR, "error %d with zipfile in unzOpenCurrentFilePassword.", err);
		}

		if (err == UNZ_OK) {
			fout = FOPEN_FUNC(write_filename, "wb");
			/* Some zip files don't contain directory path before file */
			if ((fout == NULL) && ((*popt_extract_without_path) == 0) &&
				(filename_withoutpath != (char *) filename_inzip)) {
				char c = *(filename_withoutpath - 1);
				*(filename_withoutpath - 1) = '\0';
				makedir(write_filename);
				*(filename_withoutpath - 1) = c;
				fout = FOPEN_FUNC(write_filename, "wb");
			}

			if (fout == NULL) {
				TW_LOG(TW_ERROR, "Error opening %s.", write_filename);
			}
		}

		if (fout != NULL) {
			TW_LOG(TW_DEBUG, "Extracting: %s", write_filename);

			do {
				err = unzReadCurrentFile(uf, buf, size_buf);
				if (err < 0) {
					TW_LOG(TW_ERROR, "Error %d with zipfile in unzReadCurrentFile.", err);
					break;
				}
				if (err > 0)
					if (fwrite(buf, err, 1, fout) != 1) {
						TW_LOG(TW_ERROR, "Error in writing extracted file.");
						err = UNZ_ERRNO;
						break;
					}
			} while (err > 0);
			if (fout)
				fclose(fout);

			if (err == 0)
				change_file_date(write_filename, file_info.dosDate,
								 file_info.tmu_date);
		}

		if (err == UNZ_OK) {
			err = unzCloseCurrentFile(uf);
			if (err != UNZ_OK) {
				TW_LOG(TW_ERROR, "error %d with zipfile in unzCloseCurrentFile.", err);
			}
		} else
			unzCloseCurrentFile(uf); /* don't lose the error */
	}

	TW_FREE(buf);
	return err;
}

int twDoZipExtract(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char *password) {
	uLong i;
	unz_global_info64 gi;
	int err;

	err = unzGetGlobalInfo64(uf, &gi);
	if (err != UNZ_OK) {
		TW_LOG(TW_ERROR, "Error %d with zipfile in unzGetGlobalInfo.", err);
		return 1;
	}
	for (i = 0; i < gi.number_entry; i++) {
		if (twDoZipExtractCurrentFile(uf, &opt_extract_without_path,password) != UNZ_OK)
			break;

		err = unzGoToNextFile(uf);
		if (err == UNZ_END_OF_LIST_OF_FILE) {
			TW_LOG(TW_DEBUG, "End of list in unzGoToNextFile.");
			break;
			}
	    else if (err != UNZ_OK) {
			TW_LOG(TW_ERROR, "Error %d with zipfile in unzGoToNextFile.", err);
			break;
		}
	}

	return 0;
}

int twApi_ZipExtractFile(const char *zipFileName){
	int opt_extract_without_path=0;
	int opt_overwrite=1;
	int result=0;
	const char* password = NULL;
	unzFile uf = unzOpen64(zipFileName);
	result = twDoZipExtract(uf, opt_extract_without_path, opt_overwrite, password);
	unzClose(uf);
	return result;
}

int twApi_TgzExtractFile( const char* tgzFileName){
	char* argv[2];
	char * fileName = (char*)tgzFileName;
	gzFile f = gzopen(fileName,"rb");
	if (f == NULL)
	{
		TW_LOG(TW_ERROR,"Couldn't gzopen %s.",tgzFileName);
		return 1;
	}
	argv[0] = fileName;
	argv[1] = fileName;
	/* The tar function closes its own file handles */
	tar(f, TGZ_EXTRACT, 1, 1, argv);
	return TW_OK;
}
