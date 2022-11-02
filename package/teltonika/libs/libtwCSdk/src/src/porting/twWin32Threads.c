/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Wrappers for OS Specific Thread functionality
 */

#include "twThreads.h"

#include <process.h>

/********************************/
/* OS ThreadFunction Definition */
/********************************/
unsigned __stdcall OsThreadFunction( void * param ) {
	twThread * t = (twThread *)param;
	if (!t || !t->func) return -1;
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
	_endthreadex(0);
	return 0;
}

/*******************************/
/*    twThread API Funtions    */
/*******************************/
twThread * twThread_Create(twTaskFunction func, uint32_t rate, void * opaquePtr, char autoStart) {
	/* Internal variables */
	twThread * t = NULL;
	/* Check required params */
	if (!func) return NULL;
	/* Allocate our structure */
	t = (twThread *)TW_CALLOC(sizeof(twThread), 1);
	if (!t) return NULL;
	/* Set up the twThread. Don't start running yet unless autorun is set */
	t->func = func;
	t->rate = rate;
	t->opaquePtr = opaquePtr;
	t->id = (HANDLE)_beginthreadex(NULL, 0, OsThreadFunction, t, 0, NULL);  
	if (t->id == NULL) {
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
	CloseHandle(tmp->id);
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
		TerminateThread(t->id, -1);
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
