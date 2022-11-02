#include "twApiStubs.h"

#ifdef TW_STUBS
/* do nothing by default */
#else
/*
already globally defined in twApi.c 
twApi_Stubs* twApi_stub=NULL;
*/
#endif

int twStubs_Use() {
    if(twApi_stub == NULL) {
        return -1;
    } else {
        return twApi_CreateStubs();
    }
}

int twStubs_Reset() {
    return twApi_CreateStubs();
}

