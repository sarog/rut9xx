/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twTasker.h
 * \brief Simple ThingWorx Tasker
*/

#ifndef TASKER_H
#define TASKER_H

#include "twDefaultSettings.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Function signature of a task called in round robin fashion.
 *
 * \param[in]     sys_msecs     The current non-rollover 64-bit msec counter
 *                              value. On a platform with millisecond datetime
 *                              capabilities, this will be the current
 *                              date/time.
 * \param[in]     params        A pointer to the parameters of the function.
 *
 * \return Nothing.
 *
 * \note Task functions <b>must</b> return in a cooperative fashion.
*/
typedef void (*twTaskFunction) (uint64_t sys_msecs, void * params);

/**
 * \brief Task structure definition.
*/
typedef struct twTask {
   uint32_t runTimeIntervalMsec; /**< The period (in msec) at which to call this task. **/
   uint64_t nextRunTick;         /**< The next run tick for this task. **/
   twTaskFunction func           /**< A pointer to the function to call when executing the task. **/;
} twTask;

twTask* twTaskerGetTask(int i);

/**
 * \brief Initializes the tasker.
 *
 * \return Nothing.
 *
 * \note Task functions are called in round robin fashion at a rate defined
 * when the tasks are added.
*/
void twTasker_Initialize();

/**
 * \brief Adds a new task to the tasker.
 *
 * \param[in]     runTimeIntervalMsec   The period (in msec) at which to call
 *                                      this task.
 * \param[in]     func                  A pointer to the function to call when
 *                                      executing the task.
 *
 * \return The id of the resulting task.
*/
int twTasker_CreateTask(uint32_t runTimeIntervalMsec, twTaskFunction func);

/**
 * \brief Removes a task from the tasker.
 *
 * \param[in]     id        The id of the task to be removed from the tasker.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTasker_RemoveTask(int id);

/**
 * \brief Removes all tasks from the tasker.
 *
*/
void twTaskerRemoveAllTasks();

#ifdef __cplusplus
}
#endif

#endif
