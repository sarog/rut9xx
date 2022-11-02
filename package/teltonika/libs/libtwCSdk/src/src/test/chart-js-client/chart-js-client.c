//
// Created by William Reichardt on 12/9/16.
//

#include <stringUtils.h>
#include <twList.h>
#include "chart-js-client.h"
#include "twOSPort.h"
#include "twMap.h"
#include "cfuhash.h"

#define METRIC_BUFFER_SIZE 100000

static twMap* charjsMetricList=NULL;
static twList* chartjsGraphList=NULL;

typedef struct chartjs_GraphListValue {
	char * graphName;
	char * metricNameForX;
	char * metricUnitForX;
	char * metricNameForY;
	char * metricUnitForY;
} chartjs_GraphListValue;

twMap* chartjs_getMasterMetricList() {
	if(charjsMetricList==NULL){
		charjsMetricList  = twMap_Create(NULL,NULL);
	}
	return charjsMetricList;
}

void chartjs_DeleteGraphList(void * graphListEntry){
	chartjs_GraphListValue* item = (chartjs_GraphListValue*)graphListEntry;
	TW_FREE(item->graphName);
	TW_FREE(item->metricNameForX);
	TW_FREE(item->metricUnitForX);
	TW_FREE(item->metricNameForY);
	TW_FREE(item->metricUnitForY);
}

twMap* chartjs_getGraphList() {
	if(chartjsGraphList==NULL){
		chartjsGraphList  = twList_Create(chartjs_DeleteGraphList);
	}
	return chartjsGraphList;
}


char* chartjs_replace_char(char* str, char find, char replace){
	char *current_pos = strchr(str,find);
	while (current_pos){
		*current_pos = replace;
		current_pos = strchr(current_pos,find);
	}
	return str;
}


void chartjs_send_plain( const char* path, float value, unsigned long timestamp ){
	char* jsName;
	twList * currentMetricList;
	char metricRenderBuff[100];
	twMap* masterMetricList = chartjs_getMasterMetricList();
	jsName = duplicateString(path);

	chartjs_replace_char(jsName,'.','_');
	snprintf(metricRenderBuff,100,"%f",value);
	if(TW_OK == twMap_get(masterMetricList,jsName,&currentMetricList)){
		twList_Add(currentMetricList,duplicateString(metricRenderBuff));
	} else {
		// Add first item in metric list
		twList* metricList = twList_Create(NULL);
		twList_Add(metricList,duplicateString(metricRenderBuff));
		twMap_put(masterMetricList,jsName,metricList);
	}


}

typedef struct chartjs_write_fileParams {
	FILE* outFile;
	char* metricName;
	char* metricTitle;
} chartjs_write_fileParams;

int chartjs_write_fileMetricForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	char metricBuffer[255];
	char* metricValue;
	chartjs_write_fileParams *params = (chartjs_write_fileParams *) arg;
	metricValue = (char*)data;
	snprintf(metricBuffer,255,"%s,",metricValue);
	TW_FWRITE(metricBuffer, 1, strlen(metricBuffer), params->outFile);
	return TW_FOREACH_CONTINUE;
}

int chartjs_write_fileForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg){
	char metricBuffer[255];
	twList* currentMetricList;
	char* terminator = "0];\n";
	char* jsname = (char*)key;
	chartjs_write_fileParams* params = (chartjs_write_fileParams*)arg;
	currentMetricList=(twList*)data;

	snprintf(metricBuffer,255,"var %s =[",key);
	TW_FWRITE(metricBuffer, 1, strlen(metricBuffer), params->outFile);

	twList_Foreach(currentMetricList,chartjs_write_fileMetricForEachHandler,arg);
	twList_Clear(currentMetricList);
	snprintf(metricBuffer,255,"var %s =[",jsname);
	TW_FWRITE(terminator, 1, sizeof(terminator), params->outFile);

	return TW_FOREACH_CONTINUE;
}

/**
 * Responsible for writing all stored metrics to file. Should be called atexit() and on sigint
 */
void chartjs_write_file(){
	char* localFile="perfdata.js";
	twMap* masterMetricList = chartjs_getMasterMetricList();

	FILE* fout = TW_FOPEN(localFile,"w");

	chartjs_write_fileParams* params = TW_MALLOC(sizeof(chartjs_write_fileParams));
	params->outFile = fout;
	twMap_Foreach(masterMetricList,chartjs_write_fileForEachHandler,params);
	twMap_free(masterMetricList);
	TW_FREE(params);

	TW_FCLOSE(fout);
}

void chartjs_write_graph_file(){
	char* localFile="graphdata.js";
	twMap* masterMetricList = chartjs_getMasterMetricList();

	FILE* fout = TW_FOPEN(localFile,"w");

	//TODO Write out object designing all graphs twGraphs = [{},{}];
//	chartjs_write_fileParams* params = TW_MALLOC(sizeof(chartjs_write_fileParams));
//	params->outFile = fout;
//	twMap_Foreach(masterMetricList,chartjs_write_fileForEachHandler,params);
//	twMap_free(masterMetricList);
//	TW_FREE(params);

	TW_FCLOSE(fout);
}

void chartjs_write_graph(const char* name,const char* x,const char* xunits,const char* y,const char* yunits){
	twList* masterMetricList = chartjs_getGraphList();
	chartjs_GraphListValue* graphData = (chartjs_GraphListValue*)TW_MALLOC(sizeof(chartjs_GraphListValue));
	graphData->graphName=name;
	graphData->metricNameForX=x;
	graphData->metricUnitForX=xunits;
	graphData->metricNameForY=y;
	graphData->metricUnitForY=yunits;
	twList_Add(masterMetricList,graphData);
}
