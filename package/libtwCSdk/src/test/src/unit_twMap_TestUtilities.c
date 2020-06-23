#include "twApi.h"
#include "TestUtilities.h"

const char *twMap_parseBadString(void *item) {
	return NULL;
}

const char *twMap_parseString(void *item) {
	char *string;
	if(twMap_parseStringForceFail)
		return NULL;
	else {
		string = (char *) item;
		twMapCreateTestParseHandlerCalled = TRUE;
		return duplicateString(string);
	}
}

void twMap_deleteString(void *item) {
	char *string = (char *)item;
	twMapCreateTestDeleteHandlerCalled = TRUE;
	TW_FREE(string);
}