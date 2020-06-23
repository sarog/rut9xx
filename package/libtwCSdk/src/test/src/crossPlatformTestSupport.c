#include <stdio.h>
#include "twExt.h"
/*
 Created by William Reichardt on 10/24/17.
*/


int xPlat_executeReturnOutput(const char * shellCommand,char* outputBuffer,int sizeOfBuffer){
	FILE *fp;
	char lineBuffer[256];
	char* currentPosition = outputBuffer;
	/* Open the command for reading. */
#ifdef WIN32
	fp = _popen(shellCommand, "r");
#else
	fp = popen(shellCommand, "r");
#endif
	if (fp == NULL) {
		return -666;
	}

	/* Read the output a line at a time - output it. */
	while (fgets(lineBuffer, 255, fp) != NULL) {
		strcpy(currentPosition,lineBuffer);
		currentPosition+=strlen(lineBuffer);
		printf("%s\n", lineBuffer);
	}

	/* close */
#ifdef WIN32
	_pclose(fp);
#else
	pclose(fp);
#endif
	return TW_OK;
}

char xPlat_NodeInstalled(){
	char output[256];
	if(TW_OK!=xPlat_executeReturnOutput("node -v",output,256))
		return FALSE;
	else {
		if(output[0]=='v'){
			return TRUE;
		} else {
			return FALSE;
		}
	}
}

char xPlat_DockerInstalled(){
	char output[256];
	if(TW_OK!=xPlat_executeReturnOutput("docker -v",output,256))
		return FALSE;
	else {
		if(0 == strncmp("Docker",output,6)){
			return TRUE;
		} else {
			return FALSE;
		}
	}
}




