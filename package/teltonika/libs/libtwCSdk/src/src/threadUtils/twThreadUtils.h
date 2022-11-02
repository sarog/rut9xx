/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/
#ifndef TW_C_SDK_TWTHREADUTILS_H
#define TW_C_SDK_TWTHREADUTILS_H

/**
 * \file twThreadUtils.h
 * \brief Provides simple thread management for ThingWorx C SDK functionality
 * \author bill.reichardt@thingworx.com
 *
 * This module introduces two thread management functions, twExt_Idle() and twExt_Start(). Both functions can be called to
 * establish the minimum number of support threads and services to manage an Always-On connection to ThingWorx. The
 * main difference is that twExt_Idle() will not return until your application is terminated. Any thread used to call this
 * function will not exit. twExt_Idle() is useful in situations where your application only wants to start up services
 * and then idle until it is exited.
 *
 * twExt_Start() assumes that you are starting up Always-On services as part of your application's normal start up process
 * and need the calling thread to return once this is done to continue with operations you may need to perform as part
 * of you startup process.
 *
 * Both functions set up periodic calls to any polled functions you declare which will be bound to specific Thing Shapes
 * or Thing Templates. Periodic polled functions, often referred to as "Process Scan Request" functions can be declared
 * against any Edge Thing Shape or Template using the twExt_RegisterPolledTemplateFunction() and
 * twExt_RegisterPolledShapeFunction() functions can can be used to generate simulated data or to poll hardware for new data
 * in your Thing or Shape.
*/

#include "twOSPort.h"
#include "twMap.h"

enum twThreadingModel {TW_THREADING_SINGLE=0,TW_THREADING_TASKER=1,TW_THREADING_MULTI=2};

typedef void (*twOnPolled)(char* entityName);
typedef struct twPolledFunctionRecord {
    twOnPolled onPolledFunction;
    char * shapeName;
    char * templateName;
} twPolledFunctionListRecord;

/**
 * Register a callback function that takes a single char* that will be called
 * once for each Thing that uses the template "templateName". Use this function to add polled or
 * processScanRequest style functions that should be called on any thing based on this template.
 * @param polledFunction A function using the style defined by twOnPolled. Ex.
 *		myPolledFunction(char* entityName);
 * @param templateName The name of the template this function should be attached to.
 */
void twExt_RegisterPolledTemplateFunction(twOnPolled polledFunction, char *templateName);

/**
 * Register a callback function that takes a single char* that will be called
 * once for each Thing that uses the thing shape "shapeName". Use this function to add polled or
 * processScanRequest style functions that should be called on any thing based on this thing shape.
 * @param polledFunction A function using the style defined by twOnPolled. Ex.
 *		myPolledFunction(char* entityName);
 * @param shapeName The name of the shape this function should be attached to.
 */
void twExt_RegisterPolledShapeFunction(twOnPolled polledFunction, char *shapeName);

/**
 * Remove the polled function from the list of active polled functions.
 *
 * @param polledFunction A pointer to the polled function to be removed.
 */
void twExt_RemovePolledFunction(twOnPolled polledFunction);

/**
 * A tasker style handler function which can be called directly or by the tasker
 * to execute all registered polled functions for any Template or Shape.
 *
 * @param now Unused.
 * @param params Unused.
 */
void twExt_PerformPolledFunctions(DATETIME now, void *params);

/**
 * Call this function if you want this thread to take control of polling any registered polled functions.
 * This function will not exit until the application is terminated.
 * Choice of threading models is:
 *     TW_THREADING_SINGLE - Use the thread this function is called on to service registered polled functions
 *     TW_THREADING_TASKER - Use the built in tasker functionally of the C SDK to call all polled functions.
 * @param intervalMsec Polling period in milliseconds.
 * @param threadingModel Threading model to use.
 * @param messageHandlerThreadCount Number of message handling threads to spawn.
 */
void twExt_Idle(uint32_t intervalMsec, enum twThreadingModel threadingModel, uint32_t messageHandlerThreadCount);

/**
 * Starts a thread to monitor all things with registered polled functions.
 * Use this function if you want control of the calling thread to perform other
 * work inside your application. This function relies on the tasker to call
 * polled functions on a thread it creates.
 * @param intervalMsec Polling period in milliseconds.
 * @param threadingModel Threading model to use.
 * @param messageHandlerThreadCount Number of message handling threads to spawn.
 */
void twExt_Start(uint32_t dataCollectionRate, enum twThreadingModel threadingModel,
				 uint32_t messageHandlerThreadCount);

/**
 * Shut down all threads associated with your current threading model. You should call twExt_Stop() before twApi_Disconnect()
 * when shutting down your active threading model.
  * @return TW_OK is succeeds, otherwise TW_THREADING_MODEL_FAILED_SHUTDOWN
*/
int twExt_Stop();

/**
 * Block until it can confirm that you have received at least one synchronization message from the server.
 * It can be used to wait until your application is cleared to start posting property changes.
 * @param timeoutMills Maximum wait time in milliseconds.
 * @return TW_OK is succeeds, otherwise TW_SUBSCRIBED_PROPERTY_SYNCHRONIZATION_TIMEOUT
 */
int twExt_WaitUntilFirstSynchronization(uint32_t timeoutMills);

#endif //TW_C_SDK_TWTHREADUTILS_H
