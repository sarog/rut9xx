#ifndef TW_ZIPTAR_H
#define TW_ZIPTAR_H
/**
 * Extracts the contents of a Zip file into the same directory the Zip file is located in.
 * @param zipFileName a Path to the zip file.
 * @return
 */
int twApi_ZipExtractFile(const char *zipFileName);
int twApi_TgzExtractFile(const char* tgzFileName);

#endif