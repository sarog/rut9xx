/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Wrappers for OS Specific Thread functionality
 */

#include "twThreads.h"

#include <pthread.h>

/********************************/
/* OS ThreadFunction Definition */
/********************************/
void * OsThreadFunction( void * param ) {
	twThread * t = (twThread *)param;
	if (!t || !t->func) return NULL;
	while (!t->shutdownRequested) {
		if (t->isRunning && !t->isPaused) {
			/* Call our thread function */
			t->func(twGetSystemTime(TRUE), t->opaquePtr);
		}
		if (t->rate) {
			twSleepMsec(t->rate);
		} else {
			/* Run once and done thread has a rate of 0 */
			t->shutdownRequested = TRUE;
		}
	}
	t->hasStopped = TRUE;
	t->isPaused = FALSE;
	t->isRunning = FALSE;
	pthread_exit(0);
	return NULL;
}

/*******************************/
/*    twThread API Funtions    */
/*******************************/
twThread * twThread_Create(twTaskFunction func, uint32_t rate, void * opaquePtr, char autoStart) {
	/* Internal variables */
	twThread * t = NULL;
	int res = 0;
	pthread_attr_t pthread_attr = {0};
	/* Check required params */
	if (!func) return NULL;
	/* Allocate our structure */
	t = (twThread *)TW_CALLOC(sizeof(twThread), 1);
	if (!t) return NULL;
	/* Set up the twThread. Don't start running yet unless autorun is set */
	t->func = func;
	t->rate = rate;
	t->opaquePtr = opaquePtr;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
	res = pthread_create(&t->id, &pthread_attr, OsThreadFunction, t);
	pthread_attr_destroy(&pthread_attr);
	if (res != 0) {
		twThread_Delete(t);
		return NULL;
	}
	t->isRunning = autoStart;
	return t;
}

void twThread_Delete(void * t) {
	twThread * tmp = (twThread *)t;
	if (!t) return;
	if (!tmp->hasStopped) twThread_Stop(tmp, 10000);
	TW_FREE(t);
}

int twThread_Start(twThread * t) {
	if (!t) return TW_INVALID_PARAM;
	t->isRunning = TRUE;
	return TW_OK;
}

int twThread_Stop(twThread * t, int32_t waitTime) {
	uint64_t shutdownTime = twGetSystemMillisecondCount() + waitTime;
	if (!t) return TW_INVALID_PARAM;
	t->shutdownRequested = TRUE;
	while (!t->hasStopped && twGetSystemMillisecondCount() < shutdownTime) {
		twSleepMsec(5);
	}

	if (!t->hasStopped) {
		pthread_cancel(t->id);
		t->hasStopped = TRUE;
	}

	return TW_OK;
}

int twThread_Pause(twThread * t) {
	if (!t) return TW_INVALID_PARAM;
	t->isPaused = TRUE;
	return TW_OK;
}

int twThread_Resume(twThread * t) {
	if (!t) return TW_INVALID_PARAM;
	t->isPaused = FALSE;
	return TW_OK;
}

char twThread_IsRunning(twThread * t) {
	return t ? t->isRunning : FALSE;
}

char twThread_IsPaused(twThread * t) {
	return t ? t->isPaused : FALSE;
}

char twThread_IsStopped(twThread * t) {
	return t ? t->hasStopped : FALSE;
}

TW_THREAD_ID twThread_GetThreadId(twThread * t) {
	if (!t) return 0;
	return t->id;
}
