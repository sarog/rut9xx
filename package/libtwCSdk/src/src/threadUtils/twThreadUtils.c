/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

#include "twTls.h"
#include "twThreadUtils.h"
#include "twThreads.h"
#include "twShapes.h"
#include "twTunnelManager.h"
#include "twConstants.h"
#define TW_POLL_TIME_DEFAULT_MS 5
#define TW_STOP_THREAD_NOW 0
extern twApi * tw_api;

static twList* twPolledFunctionsList = NULL;
static twList* twMessageWorkerThreads = NULL;
static char twThreadingModelStarted = FALSE;
static twThread* twApiThread;
static twThread* twDataCollectionThread;
static twThread* twTunnelManagerThread;
static enum twThreadingModel twCurrentThreadingModel;


void twPolledFunctionsList_Delete(void *item) {
    twPolledFunctionListRecord * tmp = (twPolledFunctionListRecord *) item;
    if (!item) {return;}
    if (tmp->shapeName) TW_FREE(tmp->shapeName);
    if (tmp->templateName) TW_FREE(tmp->templateName);
    TW_FREE(tmp);
}

twList* twGetPolledFunctionsList(){
    if(NULL == twPolledFunctionsList){
        twPolledFunctionsList = twList_Create(twPolledFunctionsList_Delete);
    }
    return twPolledFunctionsList;
}


int twPerformPolledFunctionsShapeForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg){
    twPolledFunctionListRecord * currentFunctionRecord = (twPolledFunctionListRecord *)arg;
    bindListEntry * currentBindListEntry = (bindListEntry *)data;
    if(twExt_DoesThingImplementShape(currentBindListEntry->name, currentFunctionRecord->shapeName)) {
        /* Call this poll function with the Thing name */
        currentFunctionRecord->onPolledFunction(currentBindListEntry->name);
    }
    return TW_FOREACH_CONTINUE;
}

int twPerformPolledFunctionsTemplateForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg){
    twPolledFunctionListRecord * currentFunctionRecord = (twPolledFunctionListRecord *)arg;
    bindListEntry * currentBindListEntry = (bindListEntry *)data;
    if(twExt_DoesThingImplementTemplate(currentBindListEntry->name, currentFunctionRecord->templateName)) {
        /* Call this poll function with the Thing name */
        currentFunctionRecord->onPolledFunction(currentBindListEntry->name);
    }
    return TW_FOREACH_CONTINUE;
}

void twExt_RegisterPolledTemplateFunction(twOnPolled polledFunction, char *templateName){
    twList* polledFunctionList = twGetPolledFunctionsList();

    twPolledFunctionListRecord * polledFunctionRecord = NULL;
    if (!templateName) {
        TW_LOG(TW_ERROR,"twRegisterPolledFunction: NULL entity pointer passed in.");
        return;
    }
    polledFunctionRecord = (twPolledFunctionListRecord *)TW_CALLOC(sizeof(twPolledFunctionListRecord), 1);
    if (!polledFunctionRecord) {
        TW_LOG(TW_ERROR,"twRegisterPolledFunction: Error allocating memory");
        return;
    }

    /* populate it */
    polledFunctionRecord->onPolledFunction = polledFunction;
    polledFunctionRecord->templateName = duplicateString(templateName);
    polledFunctionRecord->shapeName = NULL;

    twList_Add(polledFunctionList, polledFunctionRecord);

}

void twExt_RegisterPolledShapeFunction(twOnPolled polledFunction, char *shapeName){
    twList* polledFunctionList = twGetPolledFunctionsList();

    twPolledFunctionListRecord * polledFunctionRecord = NULL;
    if (!shapeName) {
        TW_LOG(TW_ERROR,"twRegisterPolledFunction: NULL entity pointer passed in.");
        return;
    }
    polledFunctionRecord = (twPolledFunctionListRecord *)TW_CALLOC(sizeof(twPolledFunctionListRecord), 1);
    if (!polledFunctionRecord) {
        TW_LOG(TW_ERROR,"twRegisterPolledFunction: Error allocating memory");
        return;
    }

    /* populate it */
    polledFunctionRecord->onPolledFunction = polledFunction;
    polledFunctionRecord->shapeName = duplicateString(shapeName);
    polledFunctionRecord->templateName = NULL;

    twList_Add(polledFunctionList, polledFunctionRecord);

}

void twExt_RemovePolledFunction(twOnPolled polledFunction){
    twList* polledFunctionList = twGetPolledFunctionsList();
    struct ListEntry * node = NULL;
    twPolledFunctionListRecord * val = NULL;
    node = polledFunctionList->first;
    while (node) {
        val = node->value;
        if(val->onPolledFunction == polledFunction){
            twList_Remove(polledFunctionList, node,TRUE);
            break;
        }
        node = node->next;
    }
}


void twExt_PerformPolledFunctions(DATETIME now, void *params){
    twList* polledFunctionList;
    struct ListEntry * le = NULL;

    /* Don't perform any polled function until we are ready to send subscribed property values and the API has been
     * initialized.
     */
    if(NULL == tw_api || FALSE == tw_api->firstSynchronizationComplete) {
        TW_LOG(TW_TRACE,"All Polled functions will be deferred until your API is initialized and property subscriptions are received from the server.");
        return;
    }

    /* Don't poll if there is no threading model established. */
    if(FALSE == twThreadingModelStarted){
        return;
    }

    /* iterate the list of registered polling handlers */
    polledFunctionList = twGetPolledFunctionsList();

    TW_LOG(TW_TRACE,"twExt_PerformPolledFunctions called.");

    le = twList_Next(polledFunctionList, 0);
    /* Use an asterisk to check if this entity has registered any properties */
    while (le) {
        /* Iterate all polled functions */
        if (le->value) {
            twPolledFunctionListRecord * currentFunctionRecord = (twPolledFunctionListRecord *)(le->value);
            /* Determine if they are for a template or a shape */
            if(currentFunctionRecord->templateName == NULL){
                /* Iterate through the list of Things */
				twDict_Foreach(tw_api->boundList,twPerformPolledFunctionsShapeForEachHandler,currentFunctionRecord);
            } else {
                /* Iterate through the list of Things */
				twDict_Foreach(tw_api->boundList,twPerformPolledFunctionsTemplateForEachHandler,currentFunctionRecord);
            }
        }

        le = twList_Next(polledFunctionList, le);
    }
}

void twExt_Start(uint32_t dataCollectionRate, enum twThreadingModel threadingModel,
                 uint32_t messageHandlerThreadCount) {

    if(twThreadingModelStarted){
        return;
    }

    twCurrentThreadingModel = threadingModel;

    if(threadingModel==TW_THREADING_SINGLE){
        TW_LOG(TW_WARN,"twExt_Start() cannot create any new threads in the TW_THREADING_SINGLE model. Try calling twExt_Idle().");
        return;
    }

    twThreadingModelStarted = TRUE;

    /* Register our "Data collection Task" with the Tasker */
    if((threadingModel) == TW_THREADING_TASKER) {
        twTasker_Initialize();
        twApi_CreateTask(dataCollectionRate, twExt_PerformPolledFunctions);
        twApi_CreateTask(TW_POLL_TIME_DEFAULT_MS,twApi_TaskerFunction);
        twApi_CreateTask(TW_POLL_TIME_DEFAULT_MS,twMessageHandler_msgHandlerTask);
        twApi_CreateTask(TW_POLL_TIME_DEFAULT_MS,twTunnelManager_TaskerFunction);
    }

    if((threadingModel) == TW_THREADING_MULTI) {
        /* Create and start our worker Threads */
        twMessageWorkerThreads = twList_Create(twThread_Delete);
        if (twMessageWorkerThreads) {
            int i = 0;
            for (i = 0; i < messageHandlerThreadCount; i++) {
                twThread * tmp = twThread_Create(twMessageHandler_msgHandlerTask, TW_POLL_TIME_DEFAULT_MS, TW_NO_USER_DATA, TW_THREAD_AUTOSTART);
                if (!tmp) {
                    TW_LOG(TW_ERROR,"main: Error creating worker thread.");
                    break;
                }
                twList_Add(twMessageWorkerThreads, tmp);
            }
        } else {
            TW_LOG(TW_ERROR,"main: Error creating worker thread list.");
        }
        /* Create and start a thread for the Api Tasker function */
        twApiThread = twThread_Create(twApi_TaskerFunction, TW_POLL_TIME_DEFAULT_MS, TW_NO_USER_DATA, TW_THREAD_AUTOSTART);

        /* Create and start a thread for the data collection function */
        twDataCollectionThread = twThread_Create(twExt_PerformPolledFunctions, dataCollectionRate, TW_NO_USER_DATA, TW_THREAD_AUTOSTART);

        /* Create and start a thread for the tunnel manager function */
        twTunnelManagerThread = twThread_Create(twTunnelManager_TaskerFunction, TW_POLL_TIME_DEFAULT_MS, TW_NO_USER_DATA, TW_THREAD_AUTOSTART);

        /* Check for any errors */
        if (!twApiThread || !twDataCollectionThread || !twTunnelManagerThread) {
            TW_LOG(TW_ERROR,"main: Error creating a required thread.");
        }

        /* Show that our threads are running */
        TW_LOG(TW_DEBUG,"main: API Thread isRunning: %s", twThread_IsRunning(twApiThread) ? "TRUE" : "FALSE");
        TW_LOG(TW_DEBUG,"main: Data Collection Thread isRunning: %s", twThread_IsRunning(twDataCollectionThread) ? "TRUE" : "FALSE");
        TW_LOG(TW_DEBUG,"main: Tunnel Manager Thread isRunning: %s", twThread_IsRunning(twTunnelManagerThread) ? "TRUE" : "FALSE");

        {
            ListEntry * le = twList_Next(twMessageWorkerThreads, NULL);
            int cnt = 0;
            while (le && le->value) {
                TW_LOG(TW_DEBUG,"main: Worker Thread [%d] isRunning: %s", cnt++, twThread_IsRunning((twThread *)le->value) ? "TRUE" : "FALSE");
                le = twList_Next(twMessageWorkerThreads, le);
            }
        }
    }
}

void twExt_Idle(uint32_t dataCollectionRate, enum twThreadingModel threadingModel, uint32_t messageHandlerThreadCount){

    char running = TRUE;
    DATETIME nextDataCollectionTime = 0;
    twCurrentThreadingModel = threadingModel;
    if(TW_THREADING_SINGLE != threadingModel) {
        twExt_Start(dataCollectionRate, threadingModel, messageHandlerThreadCount);
    } else {
        twThreadingModelStarted = TRUE;
    }

    while(running) {
        if(threadingModel==TW_THREADING_SINGLE) {
            DATETIME now = twGetSystemTime(TRUE);
            twApi_TaskerFunction(now, NULL);
            twMessageHandler_msgHandlerTask(now, NULL);
            twTunnelManager_TaskerFunction(now, NULL);
            if (twTimeGreaterThan(now, nextDataCollectionTime)) {
                twExt_PerformPolledFunctions(now, NULL);
                nextDataCollectionTime = twAddMilliseconds(now, dataCollectionRate);
            }
        } else if(threadingModel==TW_THREADING_TASKER||threadingModel==TW_THREADING_MULTI) {
            char in = 0;
            in = getch();
            if (in == 'q') break;
            else printf("\n");
        }
        twSleepMsec(TW_POLL_TIME_DEFAULT_MS);
    }
}

int twExt_Stop(){
    TW_LOG(TW_DEBUG,"twExt_Stop() begins.");

    if(NULL == tw_api){
        TW_LOG(TW_ERROR,"twExt_Stop() cannot shutdown threads because your API has not been initialized.");
        return TW_THREADING_MODEL_FAILED_SHUTDOWN;
    }

	if(twCurrentThreadingModel == TW_THREADING_SINGLE){
		TW_LOG(TW_WARN,"twExt_Stop() cannot shutdown threads because TW_THREADING_SINGLE only has one execution thread.");
		return TW_THREADING_MODEL_FAILED_SHUTDOWN;
	}

    if(twCurrentThreadingModel == TW_THREADING_MULTI) {
        twThreadingModelStarted = FALSE;

            /* Stop all new API calls */
            twMutex_Lock(tw_api->mtx);

            twList_Delete(twMessageWorkerThreads);
            TW_LOG(TW_DEBUG,"Message  handlers Stopped.\n");
            twThread_Delete(twTunnelManagerThread);
            TW_LOG(TW_DEBUG,"Tunnel  handler Stopped.\n");

            twThread_Delete(twDataCollectionThread);
            TW_LOG(TW_DEBUG,"Data collection  handler Stopped.\n");

            twThread_Delete(twApiThread);
            TW_LOG(TW_DEBUG,"API handler Stopped.\n");

            /* These are not lock mismatches. The threads possibly killed above may have
             * left these mutexes locked. */
            twMutex_Unlock(tw_api->mh->ws->connection->mtx);
            twMutex_Unlock(tw_api->mh->ws->sendMessageMutex);
            twMutex_Unlock(tw_api->mh->ws->sendFrameMutex);
            twMutex_Unlock(tw_api->mh->ws->recvMutex);

            twMutex_Unlock(tw_api->mtx);
    }

	if(twCurrentThreadingModel == TW_THREADING_TASKER) {
        twThreadingModelStarted = FALSE;
        twTaskerRemoveAllTasks();
    }
    TW_LOG(TW_DEBUG,"twExt_Stop() Completed.");
	return TW_OK;
}

int twExt_WaitUntilFirstSynchronization(uint32_t timeoutMills){
    /* Check for sync every 0.1s until sync is complete or timeout is reached */
    uint32_t count = 0;
    for(count = 0; count < timeoutMills/100; count++){
        if(tw_api->firstSynchronizationComplete)
            break;
        twSleepMsec(100);
    }
    if(tw_api->firstSynchronizationComplete)
        return TW_OK;
    else
        return TW_SUBSCRIBED_PROPERTY_SYNCHRONIZATION_TIMEOUT;
}
