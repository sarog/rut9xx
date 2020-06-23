/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twThreads.h
 * \brief Wrappers for OS-specific threading functionality
*/

#ifndef TW_THREADS_H
#define TW_THREADS_H

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "twDefinitions.h"
#include "twOSPort.h"
#include "twTasker.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief ::twThread structure definition.
 *
 * \note This is a very simple threading facility designed to
 * allow the use to execute twTaskFunctions to be run in
 * separate threads rather than using the built-in tasker
*/
typedef struct twThread {
	TW_THREAD_ID id;        /**< ID of the thread. **/
	twTaskFunction func;    /**< Thread task function. **/
	uint32_t rate;          /**< TBD **/
	char isRunning;         /**< #TRUE if the thread is running.  #FALSE otherwise. **/
	char isPaused;          /**< #TRUE if the thread is paused.  #FALSE otherwise. **/
	char shutdownRequested; /**< #TRUE if a shut down is requested.  #FALSE otherwise. **/
	char hasStopped;        /**< #TRUE if the thread has stopped.  #FALSE otherwise. **/
	void * opaquePtr;       /**< An opaque pointer passed into the thread for any user purpose. **/
} twThread;

/**
 * \brief Creates a new ::twThread.
 *
 * \param[in]     func          The ::twTaskFunction of the thread.
 * \param[in]     rate          TBD
 * \param[in]     opaquePtr     An opaque pointer passed into the thread for
 *                              any user purpose.
 * \param[in]     autoStart     #TRUE has new thread start automatically.
 *
 * \return A pointer to the newly allocated ::twThread.
 *
 * \note The calling function gains ownership of the returned ::twThread and is
 * responsible for freeing it via twThread_Delete().
*/
twThread * twThread_Create(twTaskFunction func, uint32_t rate, void * opaquePtr, char autoStart);

/**
 * \brief Frees all memory associated with a ::twThread and all of its owned
 * substructures.
 *
 * \param[in]     t     A pointer to the ::twThread to be deleted.
 *
 * \return Nothing.
 *
 * \note if ::twThread#hasStopped is #FALSE, the thread will be
 * twThread_Stop()'d before it is deleted.
*/
void twThread_Delete(void * t);

/**
 * \brief Starts a ::twThread.
 *
 * \pram[in]     t      A pointer to the ::twThread to start.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twThread_Start(twThread * t);

/**
 * \brief Stops a ::twThread.
 *
 * \pram[in]     t          A pointer to the ::twThread to stop.
 * \param[in]    waitTime   How long to wait for the ::twThread to stop after a
 *                          shutdown request is set.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note If the ::twThread has not stopped after \p waitTime, the thread is
 * canceled.
*/
int twThread_Stop(twThread * t, int32_t waitTime);

/**
 * \brief Pauses a ::twThread.
 *
 * \pram[in]     t      A pointer to the ::twThread to pause.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twThread_Pause(twThread * t);

/**
 * \brief Resumes a ::twThread.
 *
 * \pram[in]     t      A pointer to the ::twThread to resume.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twThread_Resume(twThread * t);

/**
 * \brief Checks if a ::twThread is running via ::twThread#isRunning of \p t.
 *
 * \param[in]     t     The ::twThread to check.
 *
 * \return #TRUE if the thread is running, #FALSE if it is not.
*/
char twThread_IsRunning(twThread * t);

/**
 * \brief Checks if a ::twThread is paused via ::twThread#isPaused of \p t.
 *
 * \param[in]     t     The ::twThread to check.
 *
 * \return #TRUE if the thread is paused, #FALSE if it is not.
*/
char twThread_IsPaused(twThread * t);

/**
 * \brief Checks if a ::twThread has stopped via ::twThread#hasStopped of \p t.
 *
 * \param[in]     t     The ::twThread to check.
 *
 * \return #TRUE if the thread has stopped, #FALSE if it has not.
*/
char twThread_IsStopped(twThread * t);

/**
 * \brief Gets the id of a thread via ::twThread#id of \p t.
 *
 * \param[in]     t     The ::twThread to get the id of.
 *
 * \return The ::twThread#id of \p t.  0 if \p t was NULL.
*/
TW_THREAD_ID twThread_GetThreadId(twThread * t);

#ifdef __cplusplus
}
#endif

#endif
