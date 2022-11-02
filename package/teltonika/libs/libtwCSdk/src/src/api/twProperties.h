/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twProperties.h
 *
 * \brief Property definitions & metadata functions
*/

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "twOSPort.h"
#include "twLogger.h"
#include "twBaseTypes.h"
#include "twInfoTable.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Property definition structure.
*/
typedef struct twPropertyDef {
	char * name;          /**< The name of the property. **/
	char * description;   /**< A description of the property. **/
	enum BaseType type;   /**< The ::BaseType of the property. **/
	cJSON * aspects;	/** Aspects of the property, such as pushType, pushThreshold, etc */
} twPropertyDef;

/**
 * \brief Creates a new ::twPropertyDef structure.
 *
 * \param[in]     name              The name of the property.
 * \param[in]     type              The ::BaseType of the property.
 * \param[in]     description       A description of the property.
 * \param[in]     pushType          The push type of the property.  Can be set
 *                                  to #NEVER, #ALWAYS, or #VALUE (on change).
 * \param[in]     pushThreshold     The amount the property has to change (if
 *                                  the type is #TW_NUMBER or #TW_INTEGER)
 *                                  before pushing the new value.
 *
 * \return A pointer to the structure that is created.  Returns NULL on failure.
 *
 * \note The calling function will gain ownership of the newly allocated
 * ::twPropertyDef structure and is responsible for freeing it via
 * twPropertyDef_Delete().
*/
twPropertyDef * twPropertyDef_Create(char * name, enum BaseType type, char * description, char * pushType, double pushThreshold);

/**
 * \brief Frees all memory associated with a ::twPropertyDef structure and all
 * its owned substructures.
 *
 * \param[in]     input             A pointer to the ::twPropertyDef structure
 *                                  to be deleted.
 *
 * \return Nothing.
*/
void twPropertyDef_Delete(void * input);

/**
 * \brief Property base structure.
*/
typedef struct twProperty {
	char * name;         /** The name of the property. **/
	twPrimitive * value; /** A pointer to the primitive containing the property value. **/
	DATETIME timestamp;  /** Timestamp of the property (defaults to current time). **/
	char * quality; /** Quality of the property value (defaults to GOOD). **/
} twProperty;

/**
 * \brief Creates a new ::twProperty structure.
 *
 * \param[in]     name              The name of the property.
 * \param[in]     value             A pointer to a ::twPrimitive containing the
 *                                  value of the property.
 * \param[in]     timestamp         Timestamp of the property (defaults to
 *                                  current time).
 *
 * \return A pointer to the structure that is created.  Returns NULL on failure.
 *
 * \note The newly created structure will gain ownership of the \p value
 * pointer and is responsible for freeing it.
  * \note The default quality value is "GOOD".
 * \note The calling function retains ownership of the newly allocated
 * ::twProperty structure and is responsible for freeing it via
 * twProperty_Delete().
*/
twProperty * twProperty_Create(char * name, twPrimitive * value, DATETIME timestamp);

/**
 * \brief Creates a new ::twProperty structure with an OPC style Quality value.
 *
 * \param[in]     name              The name of the property.
 * \param[in]     value             A pointer to a ::twPrimitive containing the
 *                                  value of the property.
 * \param[in]     timestamp         Timestamp of the property (defaults to
 *                                  current time).
 * \param[in]     qualtiy           The OPC style quality of the value(defaults to
 *                                  "GOOD").
 *
 *
 * \return A pointer to the structure that is created.  Returns NULL on failure.
 *
 * \note The newly created structure will gain ownership of the \p value
 * pointer and is responsible for freeing it.
 * \note The calling function retains ownership of the newly allocated
 * ::twProperty structure and is responsible for freeing it via
 * twProperty_Delete().
*/
twProperty * twPropertyVTQ_Create(char * name, twPrimitive * value, DATETIME timestamp, char * quality);


/**
 * \brief Creates a new ::twProperty structure from a :twStream.
 *
 * \param[in]     s                 Pointer to the stream to read from
 *
 * \return A pointer to the structure that is created.  Returns NULL on failure.
*/
twProperty * twProperty_CreateFromStream(twStream * s);

/**
 * \brief Frees all memory associated with a ::twProperty structure and all its
 * owned substructures.
 *
 * \param[in]     input             A pointer to the ::twProperty structure to
 *                                  be deleted.
 *
 * \return Nothing.
*/
void twProperty_Delete(void * input);

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIES_H */
