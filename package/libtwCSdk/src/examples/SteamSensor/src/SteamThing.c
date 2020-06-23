/*
 *  Copyright (C) 2017 ThingWorx Inc.
 */

#include "twExt.h"
#include "twMacros.h"
#include "gps-route.h"

#define DATSHAPE_NAME_SENSOR_READINGS "SteamSensorReadingShape"


#ifdef WIN32
#define LOG_PATH ".\\logs"
#define LOGFILE_TEMPLATE "%s\\%s-log.txt"
char* getFullPathFromRelativePath(const char* relativePath){
	DWORD   nBufferLength = 255;
	char* buffer;
	buffer = (char*)TW_MALLOC(256);
	GetFullPathName(relativePath,nBufferLength,buffer,NULL);
	return buffer;
}
#else
#define LOG_PATH "./logs"
#define LOGFILE_TEMPLATE "%s/%s-log.txt"
char* getFullPathFromRelativePath(const char* relativePath) {
	return realpath(relativePath, NULL);
}
#endif

static int location_step = 0;
char enableLogging = TRUE;

void writeToLogFile(const char* thingName,DATETIME timeNow, double totalFlow,  double pressure,  twLocation location,
                    double temperature) {
    char logLineBuffer[1000];
    char timeBuffer[100];
    time_t timer;
    struct tm *tm_info;
	char* logfile;
	FILE *fh;

    time(&timer);
    tm_info = gmtime(&timer);
    strftime (timeBuffer, 100, "%Y-%m-%d %H:%M:%S.000", tm_info);
    logfile = TW_GET_PROPERTY(thingName,"Logfile").bytes.data;

	if(NULL!=logfile) {

		if (!twDirectory_FileExists(logfile)) {
			twDirectory_CreateFile(logfile);
		}

		fh = fopen(logfile, "a+");
		if (fh) {
			snprintf(logLineBuffer, 1000, "%s,%f,%f,%f,%f,%f,%f\n", timeBuffer, totalFlow, pressure, temperature,
					 location.latitude, location.longitude, location.elevation);
			fwrite(logLineBuffer, strlen(logLineBuffer), 1, fh);
			fclose(fh);
		} else {
			TW_LOG(TW_ERROR, "Failed to write to log file at %s", logfile);
		}
	}

}

void onSteamSensorProcessScanRequest(char *thingName) {
	double temp;
    double tempLim = 0;
    char faultStat;

    TW_LOG(TW_TRACE,"dataCollectionTask: Executing");
    TW_SET_PROPERTY(thingName, "TotalFlow", TW_MAKE_NUMBER(rand() / (RAND_MAX / 10.0)));
    TW_SET_PROPERTY(thingName, "Pressure", TW_MAKE_NUMBER(18 + rand() / (RAND_MAX / 5.0)));
    TW_SET_PROPERTY(thingName, "Location",
        TW_MAKE_LOC(gpsroute[location_step].latitude,gpsroute[location_step].longitude,gpsroute[location_step].elevation)
    );
    TW_SET_PROPERTY(thingName, "Temperature", TW_MAKE_NUMBER(400 + rand() / (RAND_MAX / 40)));
    if(++location_step == gpsroute_steps)
        location_step=0;

    /* Check for a fault.  Only do something if we haven't already */
    temp = TW_GET_PROPERTY(thingName, "Temperature").number;
    tempLim = 0;
    if(TW_NUMBER == TW_GET_PROPERTY_TYPE(thingName, "TemperatureLimit")) {
        tempLim = TW_GET_PROPERTY(thingName, "TemperatureLimit").number;
    }
    faultStat = TW_GET_PROPERTY(thingName,"FaultStatus").boolean;

    if (temp > tempLim){
		char msg[140];
		/* If we have not already signaled a fault, signal a fault, send out an event */
		if(faultStat == FALSE) {
			TW_SET_PROPERTY(thingName, "FaultStatus", TW_MAKE_BOOL(TRUE));
		}

		/* Open the steam release valve to lower temperature */
		TW_SET_PROPERTY(thingName, "InletValve", TW_MAKE_BOOL(TRUE));

		sprintf(msg, "%s Temperature %4.2f exceeds threshold of %4.2f",
				thingName, TW_GET_PROPERTY(thingName, "Temperature").number,
				TW_GET_PROPERTY(thingName, "TemperatureLimit").number);

		/* Fire Event */
		TW_FIRE_EVENT(thingName, "SteamSensorFault",
					  TW_MAKE_IT(TW_MAKE_DATASHAPE(
							  "SteamSensorFault",
							  TW_DS_ENTRY("message", TW_NO_DESCRIPTION, TW_STRING)
								 ), TW_IT_ROW(TW_MAKE_STRING(msg))
					  ));
    } else {
		/* If we have returned to normal operating temperature, close the valve. */
		/* Its up to the operator to reset the fault status */
		TW_SET_PROPERTY(thingName, "InletValve", TW_MAKE_BOOL(FALSE));
	}

    /* Update the properties on the server */
    TW_PUSH_PROPERTIES_FOR(thingName,TW_PUSH_CONNECT_FORCE);

	if (enableLogging) {
		writeToLogFile(thingName, twGetSystemTime(TRUE),
		               TW_GET_PROPERTY(thingName, "TotalFlow").number,
		               TW_GET_PROPERTY(thingName, "Pressure").number,
		               TW_GET_PROPERTY(thingName, "Location").location,
		               TW_GET_PROPERTY(thingName, "Temperature").number
		);
	}
}


/***********************
Service Implementations
************************/

/**
 * Example of handling a single service in a callback, this functions adds two numbers and returns their sum.
 */
enum msgCodeEnum addNumbersService(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
    double a, b, res;

    TW_LOG(TW_TRACE,"addNumbersService - Function called");
    if (!params || !content) {
        TW_LOG(TW_ERROR,"addNumbersService - NULL params or content pointer");
        return TWX_BAD_REQUEST;
    }

    /* Get the parameters */
    twInfoTable_GetNumber(params, "a", 0, &a);
    twInfoTable_GetNumber(params, "b", 0, &b);

    /* perform the calculation */
    res = a + b;

    /* Return Results */
    *content = twInfoTable_CreateFromNumber("result", res);
    if (*content)
        return TWX_SUCCESS;
    else
        return TWX_INTERNAL_SERVER_ERROR;
}

/**
 * Example service to output an InfoTable containing arbitrary SteamSensor data (see #service_cb).
 */
enum msgCodeEnum getSteamSensorReadingsService(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata) {
	twInfoTable* it;

    TW_LOG(TW_TRACE,"getSteamSensorReadingsService - Function called");
    if (!content) {
        TW_LOG(TW_ERROR, "getSteamSensorReadingsService - NULL content pointer");
        return (enum msgCodeEnum)TW_BAD_REQUEST;
    }

    it = TW_MAKE_IT(
            TW_MAKE_DATASHAPE(DATSHAPE_NAME_SENSOR_READINGS,
                    TW_DS_ENTRY("ActivationTime", TW_NO_DESCRIPTION ,TW_DATETIME),
                    TW_DS_ENTRY("SensorName", TW_NO_DESCRIPTION ,TW_NUMBER),
                    TW_DS_ENTRY("Temperature", TW_NO_DESCRIPTION ,TW_NUMBER),
                    TW_DS_ENTRY("Pressure", TW_NO_DESCRIPTION ,TW_NUMBER),
                    TW_DS_ENTRY("FaultStatus", TW_NO_DESCRIPTION ,TW_BOOLEAN),
                    TW_DS_ENTRY("InletValve", TW_NO_DESCRIPTION ,TW_BOOLEAN),
                    TW_DS_ENTRY("TemperatureLimit", TW_NO_DESCRIPTION ,TW_NUMBER),
                    TW_DS_ENTRY("TotalFlow", TW_NO_DESCRIPTION ,TW_INTEGER)
            ),
            TW_IT_ROW(TW_MAKE_DATETIME_NOW,TW_MAKE_STRING("Sensor Alpha"),TW_MAKE_NUMBER(60),TW_MAKE_NUMBER(25),TW_MAKE_BOOL(TRUE),TW_MAKE_BOOL(TRUE),TW_MAKE_NUMBER(150),TW_MAKE_NUMBER(77)),
            TW_IT_ROW(TW_MAKE_DATETIME_NOW,TW_MAKE_STRING("Sensor Beta"),TW_MAKE_EMPTY,TW_MAKE_NUMBER(35),TW_MAKE_BOOL(FALSE),TW_MAKE_BOOL(TRUE),TW_MAKE_EMPTY,TW_MAKE_NUMBER(88)),
            TW_IT_ROW(TW_MAKE_DATETIME_NOW,TW_MAKE_STRING("Sensor Gamma"),TW_MAKE_EMPTY,TW_MAKE_NUMBER(80),TW_MAKE_BOOL(TRUE),TW_MAKE_BOOL(FALSE),TW_MAKE_NUMBER(150),TW_MAKE_NUMBER(99))
    );

    /* output is always returned as an InfoTable type, since our output is an InfoTable type we can just return it */
    *content = it;
    if (*content) return TWX_SUCCESS;
    else return TWX_INTERNAL_SERVER_ERROR;
}

enum msgCodeEnum getBigStringService(const char *entityName, const char *serviceName, twInfoTable *params,
                                                twInfoTable **content, void *userdata) {
    TW_LOG(TW_TRACE,"steamSensorMultiServiceHandler - Function called");
    if (!content) {
        TW_LOG(TW_ERROR,"steamSensorMultiServiceHandler - NULL content pointer");
        return TWX_BAD_REQUEST;
    }
    if (strcmp(serviceName, "GetBigString") == 0) {
        int len = 10000;
        char text[] = "This is a really big string. ";
		int textlen;
        char * bigString = (char *)TW_CALLOC(len,1);
        textlen = strlen(text);
        while (bigString && len > textlen) {
            strncat(bigString, text, len - textlen - 1);
            len = len - textlen;
        }
        *content = twInfoTable_CreateFromString("result", bigString, FALSE);
        if (*content) return TWX_SUCCESS;
        return TWX_ENTITY_TOO_LARGE;
    }
    return TWX_NOT_FOUND;
}

enum msgCodeEnum stopLogging(const char *entityName, const char *serviceName, twInfoTable *params,
                                     twInfoTable **content, void *userdata) {
	enableLogging = FALSE;
	return TWX_SUCCESS;
}

enum msgCodeEnum startLogging(const char *entityName, const char *serviceName, twInfoTable *params,
                             twInfoTable **content, void *userdata) {
	enableLogging = TRUE;
	return TWX_SUCCESS;
}

/**
 * This function defines a thing. Multiple instances of this thing can be declared by changing the
 * thingName parameter.
 */
void createSteamSensorThing(char* thingName){
    char logFileNameBuffer[255];
	char* fullPathToLogs;

    /********************** Declare Properties **********************/
    TW_MAKE_THING(thingName,TW_THING_TEMPLATE_GENERIC);

	TW_PROPERTY("Pressure", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_ADD_BOOLEAN_ASPECT("Pressure", TW_ASPECT_ISREADONLY,TRUE);
		TW_ADD_BOOLEAN_ASPECT("Pressure", TW_ASPECT_ISLOGGED,TRUE);
	TW_PROPERTY("Temperature", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_ADD_BOOLEAN_ASPECT("Temperature", TW_ASPECT_ISREADONLY,TRUE);
		TW_ADD_BOOLEAN_ASPECT("Pressure", TW_ASPECT_ISLOGGED,TRUE);
	TW_PROPERTY("TotalFlow", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_ADD_BOOLEAN_ASPECT("TotalFlow", TW_ASPECT_ISREADONLY,TRUE);
		TW_ADD_BOOLEAN_ASPECT("TotalFlow", TW_ASPECT_ISLOGGED,TRUE);
	TW_PROPERTY("TemperatureLimit", TW_NO_DESCRIPTION, TW_NUMBER);
        TW_ADD_NUMBER_ASPECT("TemperatureLimit", TW_ASPECT_DEFAULT_VALUE,320.0);
	TW_PROPERTY("FaultStatus", TW_NO_DESCRIPTION, TW_BOOLEAN);
	TW_PROPERTY("InletValve", TW_NO_DESCRIPTION, TW_BOOLEAN);
	TW_PROPERTY("Location", TW_NO_DESCRIPTION, TW_LOCATION);
		TW_ADD_BOOLEAN_ASPECT("Location", TW_ASPECT_ISREADONLY,TRUE);
	TW_PROPERTY("Logfile", TW_NO_DESCRIPTION, TW_STRING);
		TW_ADD_BOOLEAN_ASPECT("Logfile", TW_ASPECT_ISREADONLY,TRUE);


    /********************** Declare Events **********************/
    TW_EVENT("SteamSensorFault",
        "Steam sensor event",
             TW_MAKE_DATASHAPE(
                     "SteamSensorFault",
                     TW_DS_ENTRY("message",TW_NO_DESCRIPTION,TW_STRING)
             )
    );

    /********************** Register Services **********************/
    TW_SERVICE("GetSteamSensorReadings",
        "Returns a set of readings as in InfoTable",
        TW_NO_PARAMETERS,
        TW_INFOTABLE,
        TW_MAKE_DATASHAPE(DATSHAPE_NAME_SENSOR_READINGS,
			TW_DS_ENTRY("ActivationTime", TW_NO_DESCRIPTION, TW_DATETIME),
			TW_DS_ENTRY("SensorName", TW_NO_DESCRIPTION, TW_NUMBER),
			TW_DS_ENTRY("Temperature", TW_NO_DESCRIPTION, TW_NUMBER),
			TW_DS_ENTRY("Pressure", TW_NO_DESCRIPTION, TW_NUMBER),
			TW_DS_ENTRY("FaultStatus", TW_NO_DESCRIPTION, TW_BOOLEAN),
			TW_DS_ENTRY("InletValve", TW_NO_DESCRIPTION, TW_BOOLEAN),
			TW_DS_ENTRY("TemperatureLimit", TW_NO_DESCRIPTION, TW_NUMBER),
			TW_DS_ENTRY("TotalFlow", TW_NO_DESCRIPTION, TW_INTEGER)
        ),
        getSteamSensorReadingsService
    );

    TW_SERVICE("AddNumbers",
        "Add two numbers together",
        TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
                TW_DS_ENTRY("a", TW_NO_DESCRIPTION ,TW_NUMBER),
                TW_DS_ENTRY("b", TW_NO_DESCRIPTION ,TW_NUMBER)
        ),
        TW_NUMBER,
        TW_NO_RETURN_DATASHAPE,
        addNumbersService
    );

    TW_DECLARE_SERVICE("GetBigString",
        "Generates a big string.",
        TW_NO_PARAMETERS,
        TW_STRING,
        TW_NO_RETURN_DATASHAPE,
        getBigStringService
    );

	TW_DECLARE_SERVICE("StartLogging",
	                   "Starts edge-side logging.",
	                   TW_NO_PARAMETERS,
	                   TW_NOTHING,
	                   TW_NO_RETURN_DATASHAPE,
	                   startLogging
	);

	TW_DECLARE_SERVICE("StopLogging",
	                   "Stops edge-side logging.",
	                   TW_NO_PARAMETERS,
	                   TW_NOTHING,
	                   TW_NO_RETURN_DATASHAPE,
	                   stopLogging
	);

    /* Share a directory using file transfer containing a log of readings */
    TW_ADD_FILE_TRANSFER_SHAPE();
    twDirectory_CreateDirectory(LOG_PATH);
    fullPathToLogs = getFullPathFromRelativePath(LOG_PATH);
    snprintf(logFileNameBuffer,255,LOGFILE_TEMPLATE,fullPathToLogs,thingName);
    remove(logFileNameBuffer);	
    TW_SHARE_DIRECTORY("logs",fullPathToLogs);
	TW_FREE(fullPathToLogs);

    /* Register a function to be called periodically after this thing has been created */
    twExt_RegisterPolledTemplateFunction(onSteamSensorProcessScanRequest, TW_THING_TEMPLATE_GENERIC);

    /* Set an initial property values */
    TW_SET_PROPERTY(thingName, "TemperatureLimit", TW_MAKE_NUMBER(435));
    TW_SET_PROPERTY(thingName,"Logfile",TW_MAKE_STRING(logFileNameBuffer));

    TW_BIND();

}

void destroySteamSensorThing(char* thingName) {
	twApi_UnbindThing(thingName);
	twExt_RemovePolledFunction(onSteamSensorProcessScanRequest);

}

