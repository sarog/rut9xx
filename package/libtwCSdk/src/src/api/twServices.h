/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twServices.h
 *
 * \brief Service definitions & metadata functions
*/

#ifndef SERVICES_H
#define SERVICES_H

#include "twOSPort.h"
#include "twLogger.h"
#include "twBaseTypes.h"
#include "twInfoTable.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Service definition structure.
*/
typedef struct twServiceDef {
	char * name;                   /**< The name of the service. **/
	char * description;            /**< A description of the service. **/
	twDataShape * inputs;          /**< A ::twDataShape that describes the service input. **/
	enum BaseType outputType;      /**< the ::BaseType of the service result. **/
	twDataShape * outputDataShape; /**< A :twDataShape that describes the service output if the output is a ::twInfoTable. **/
	cJSON * aspects;	/** Aspects of the property, such as pushType, pushThreshold, etc */
} twServiceDef;

/**
 * \brief Creates a new ::twServiceDef structure.
 *
 * \param[in]     name              The name of the service.
 * \param[in]     description       A description of the service.
 * \param[in]     inputs            A ::twDataShape that describes the service
 *                                  input.
 * \param[in]     outputType        The ::BaseType of the service result.
 * \param[in]     outputDataShape   A ::twDataShape that described the service
 *                                  output if the output is a ::twInfoTable.
 *
 * \return A pointer to the newly allocated ::twServiceDef structure.  Returns
 * NULL on failure.
 *
 * \note The calling function gains ownership of the newly created
 * ::twServiceDef and is responsible for freeing it via twServiceDef_Delete().
*/
twServiceDef * twServiceDef_Create(char * name, char * description, twDataShape * inputs, 
								   enum BaseType outputType, twDataShape * outputDataShape);

/**
 * \brief Frees all memory associated with a ::twServiceDef structure and all
 * its owned substructures.
 *
 * \param[in]     input             A pointer to the ::twServiceDef structure
 *                                  to be deleted.
 *
 * \return Nothing.
*/
void twServiceDef_Delete(void * input);

#ifdef __cplusplus
}
#endif

#endif /* SERVICES_H */
