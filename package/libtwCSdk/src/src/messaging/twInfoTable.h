/*************************************
 * Copyright 2017, PTC, Inc.
 *************************************/

/**
 * \file twInfoTable.h
 *
 * \brief ThingWorx ::twInfoTable and ::twDataShape definitions and functions.
*/

#include "twOSPort.h"
#include "twBaseTypes.h"
#include "twList.h"

#ifndef TW_INFOTABLE_H
#define TW_INFOTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

struct cJSON;

/**
 * \name DataShape Aspects
*/

/**
 * \brief Data shape aspect structure definition.
*/
typedef struct twDataShapeAspect {
	char * name;         /**< The name of the DataShape **/
	twPrimitive * value; /**< The value of the DataShape **/
} twDataShapeAspect;

/**
 * \brief Creates a new ::twDataShapeAspect structure.
 *
 * \param[in]     name          Name of the aspect.
 * \param[in]     value         A pointer to a primitive containing the
 *                              ::BaseType and value of the aspect.
 *
 * \return A pointer to the newly allocated ::twDataShapeAspect.  Returns NULL
 * on failure.
 *
 * \note The calling function gains ownership of the returned
 * ::twDataShapeAspect and is responsible for freeing it via
 * twDataShapeAspect_Delete().
*/
twDataShapeAspect * twDataShapeAspect_Create(const char * name, twPrimitive * value);

/**
 * \brief Creates a new ::twDataShapeAspect structure from data in a
 * ::twStream.
 *
 * \param[in]     stream        A pointer to the ::twStream to parse.
 *
 * \return A pointer to the newly allocated ::twDataShapeAspect.  Returns NULL
 * on failure.
 *
 * \note The calling function gains ownership of the returned
 * ::twDataShapeAspect and is responsible for freeing it via
 * twDataShapeAspect_Delete().
*/
twDataShapeAspect * twDataShapeAspect_CreateFromStream(twStream * s);

/**
 * \brief Frees all memory associated with a ::twDataShapeAspect and all its
 * owned substructures.
 *
 * \param[in]     aspect        A pointer to the ::twDataShapeAspect to delete.
 *
 * \return Nothing.
*/
void twDataShapeAspect_Delete(void * aspect);

/**
 * \name Data Shape Entries
*/

/**
 * \brief DataShape entry structure definition.
*/
typedef struct twDataShapeEntry {
	char * name;        /**< The name of the entry. **/
	char * description; /**< A description of the entry. **/
	enum BaseType type; /**< The ::BaseType of the entry. **/
	twList * aspects;   /**< A ::twList of the aspects associated with the entry. **/
} twDataShapeEntry;

/**
 * \brief Creates a new ::twDataShapeEntry structure.
 *
 * \param[in]     name          Name of the field.
 * \param[in]     value         A description of the field.
 * \param[in]     type          The ::BaseType associated with the field.
 *
 * \return A pointer to the newly allocated ::twDataShapeEntry.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the returned ::twDataShapeEntry
 * and is responsible for freeing it via twDataShapeEntry_Delete().
*/
twDataShapeEntry * twDataShapeEntry_Create(const char * name, const char * description, enum BaseType type);

/**
 * \brief Creates a new ::twDataShapeEntry structure from data in a ::twStream.
 *
 * \param[in]     s             A pointer to the ::twStream to parse.
 *
 * \return A pointer to the newly allocated ::twDataShapeEntry.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the returned
 * ::twDataShapeEntry and is responsible for freeing it via
 * twDataShapeEntry_Delete().
*/
twDataShapeEntry * twDataShapeEntry_CreateFromStream(struct twStream * s);

/**
 * \brief Frees all memory associated with a ::twDataShapeEntry structure and
 * all its owned substructures.
 *
 * \param[in]     entry         A pointer to the ::twDataShapeEntry structure
 *                              to delete.
 *
 * \return Nothing.
*/
void twDataShapeEntry_Delete(void * entry);

/**
 * \brief Creates a new ::twDataShapeAspect with ::twDataShapeAspect#value \p
 * value and adds it to the specified ::twDataShapeEntry \p entry.
 *
 * \param[in]     entry         A pointer to the ::twDataShapeEntry to add the
 *                              ::twDataShapeAspect to.
 * \param[in]     name          Name of the aspect.
 * \param[in]     value         A pointer to a primitive containing the
 *                              ::BaseType and value of the aspect.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The ::twDataShapeEntry \p entry will gain ownership of the newly
 * allocated ::twDataShapeAspect.
*/
int twDataShapeEntry_AddAspect(struct twDataShapeEntry * entry, const char * name, twPrimitive * value);

/**
 * \brief Retrieves the length of a ::twDataShapeEntry.
 *
 * \param[in]     entry         A pointer to the ::twDataShapeEntry to retrieve
 *                              the length of.
 *
 * \return The length of \p entry.
 *
 * \note The length returned is calculated as if it were serialized to a
 * ::twStream.
*/
uint32_t twDataShapeEntry_GetLength(struct twDataShapeEntry * entry);

/**
 * \brief Serializes a ::twDataShapeEntry to a ::twStream.
 *
 * \param[in]     entry         A pointer to the ::twDataShapeEntry to
 *                              serialize.
 * \param[in,out] s             A pointer to the ::twStream to serialize to.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The calling function retains ownership of the \p s pointer and is
 * responsible for freeing it via twStream_Delete().
*/
int twDataShapeEntry_ToStream(struct twDataShapeEntry * entry, twStream * s);

/**
 * \name Data Shapes
*/

/**
 * \brief Data shape base structure definition.
*/
typedef struct twDataShape {
	int numEntries;   /**< The number of entries associated with the ::twDataShape **/
	twList * entries; /**< A list of the entries associated with the ::twDataShape **/
	char * name;      /**< The name of the ::twDataShape **/
} twDataShape;

/**
 * \brief Creates a new ::twDataShape structure.
 *
 * \param[in]     firstEntry    A pointer to a ::twDataShapeEntry to seed the
 *                              ::twDataShape with. If NULL and empty datashape 
 *                              is created.
 *
 * \return A pointer to the newly allocated ::twDataShape.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the ::twDataShape and is
 * responsible for freeing it via twDataShape_Delete().
*/
twDataShape * twDataShape_Create(twDataShapeEntry * firstEntry);

/**
 * \brief Creates a new ::twDataShape structure from data in a ::twStream.
 *
 * \param[in]     s             A pointer to a ::twStream to parse.
 *
 * \return A pointer to the newly allocated ::twDataShape.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the ::twDataShape and is
 * responsible for freeing it via twDataShape_Delete().
*/
twDataShape * twDataShape_CreateFromStream(struct twStream * s);

/**
 * \brief Frees all memory associated with a ::twDataShape structure and all
 * its owned substructures.
 *
 * \param[in]     entry         A pointer to the ::twDataShape structure to
 *                              delete.
 *
 * \return Nothing.
*/
void twDataShape_Delete(void * ds);

/**
 * \brief Retrieves the length of a ::twDataShape.
 *
 * \param[in]     entry         A pointer to the ::twDataShape to retrieve the
 *                              length of.
 *
 * \return The length of \p entry.
 *
 * \note The length returned is calculated as if it were serialized to a
 * ::twStream.
*/
uint32_t twDataShape_GetLength(struct twDataShape * ds);

/**
 * \brief Serializes a ::twDataShape to a ::twStream.
 *
 * \param[in]     entry         A pointer to the ::twDataShape to serialize.
 * \param[in,out] s             A pointer to the ::twStream to serialize to.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twDataShape_ToStream(struct twDataShape * ds, twStream * s);

/**
 * \brief Sets the name of a ::twDataShape.
 *
 * \param[in]     ds            A pointer to the ::twDataShape to set the name
 *                              of.
 * \param[in]     s             The name to set the ::twDataShape to.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The \p ds ::twDataShape does not take ownership of the \p s string.
*/
int twDataShape_SetName(struct twDataShape * ds, char * name);

/**
 * \brief Adds a new ::twDataShapeEntry to a ::twDataShape.
 *
 * \param[in]     ds            A pointer to the ::twDataShape to add the entry
 *                              to.
 * \param[in]     entry         A pointer to the ::twDataShapeEntry to add.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The \p ds ::twDataShape takes ownership of the \p entry
 * ::twDataShapeEntry.
*/
int twDataShape_AddEntry(struct twDataShape * ds, struct twDataShapeEntry * entry);

/**
 * \brief Gets the index of the ::twDataShapeEntry with the specified name.
 *
 * \param[in]     ds            A pointer to the ::twDataShape to get the index
 *                              from.
 * \param[in]     name          The name of the field to get the index of.
 * \param[out]    index         The field index (0 based) of \p name if successful.
 *
 * \return #TW_OK is successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twDataShape_GetEntryIndex(struct twDataShape * ds, const char * name, int * index);

/**
 * \name Info Table Rows
*/

/**
 * \brief Info table row structure.
*/
typedef struct twInfoTableRow {
	uint16_t numFields;    /**< The number of fields associated with the ::twInfoTableRow. **/
	twList * fieldEntries; /**< A ::twList of the entries associated with the ::twInfoTableRow. **/
} twInfoTableRow;

/**
 * \brief Creates a new ::twInfoTableRow.
 *
 * \param[in]     firstEntry    A pointer to a ::twPrimitive to seed the
 *                              ::twInfoTableRow with.
 *
 * \return A pointer to the newly allocated ::twInfoTableRow.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the returned ::twInfoTableRow
 * and is responsible for freeing it via twInfoTableRow_Delete().
*/
twInfoTableRow * twInfoTableRow_Create(twPrimitive * firstEntry);

/**
 * \brief Creates a new ::twInfoTableRow from data in a ::twStream.
 *
 * \param[in]     s             A pointer to the ::twStream to parse.
 *
 * \return A pointer to the newly allocated ::twInfoTableRow.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the returned ::twInfoTableRow
 * and is responsible for freeing it via twInfoTableRow_Delete().
*/
twInfoTableRow * twInfoTableRow_CreateFromStream(twStream * s);

/**
 * \brief Deletes a ::twInfoTableRow structure and frees all memory associated
 * with it.
 *
 * \param[in]     entry         A pointer to the ::twInfoTableRow structure to
 *                              delete.
 *
 * \return Nothing.
*/
void twInfoTableRow_Delete(void * row);

/**
 * \brief Gets the number of fields in a ::twInfoTableRow.
 *
 * \param[in]     row           A pointer to the ::twInfoTableRow get the
 *                              number of fields of.
 *
 * \return The number of fields in \p row if successful, positive integral on
 * error code (see twErrors.h) if an error was encountered.
*/
int twInfoTableRow_GetCount(twInfoTableRow * row);

/**
 * \brief Gets the length of a ::twInfoTableRow.
 *
 * \param[in]     row           A pointer to the ::twInfoTableRow get the
 *                              length of.
 *
 * \return The length of \p row.
 *
 * \note The length returned is calculated as if it were serialized to a
 * ::twStream.
 *
*/
uint32_t twInfoTableRow_GetLength(twInfoTableRow * row);

/**
 * \brief Adds a new ::twPrimitive to a ::twInfoTableRow
 *
 * \param[in]     row           A pointer to the ::twInfoTableRow to add the
 *                              entry to.
 * \param[in]     entry         A pointer to the ::twPrimitive to add.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The \p ds ::twInfoTableRow takes ownership of the \p entry ::twPrimitive.
*/
int twInfoTableRow_AddEntry(twInfoTableRow * row, twPrimitive * entry);

/**
 * \brief Gets the ::twPrimitive value of an entry in a ::twInfoTableRow.
 *
 * \param[in]     row           A pointer to the ::twInfoTableRow to get the
 *                              value from.
 * \param[in]     index         The (zero based) index of the field to
 *                              retrieve.
 *
 * \return A pointer to the ::twPrimitive at index \p index of \p row.  Returns
 * NULL if an error occurred.
*/
twPrimitive * twInfoTableRow_GetEntry(twInfoTableRow * row, int index);

/**
 * \brief Serializes a ::twInfoTableRow to a ::twStream.
 *
 * \param[in]     row           A pointer to the ::twInfoTableRow to serialize.
 * \param[in,out] s             A pointer to the ::twStream to serialize to.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The calling function retains ownership of \p s and is responsible for
 * freeing it via twStream_Delete().
*/
int twInfoTableRow_ToStream(twInfoTableRow * row, twStream * s);

/**
 * \name Info Tables
*/

/**
 * \brief Info table base structure.
*/
typedef struct twInfoTable {
	twDataShape * ds; /**< The data shape associated with the ::twInfoTable. **/
	twList * rows;    /**< A ::twList of the ::twInfoTableRow entries associated with the ::twInfoTable. **/
	uint32_t length;  /**< The length of the ::twInfoTable. **/
	TW_MUTEX mtx;     /**< The #TW_MUTEX associated with the ::twInfoTable. **/
} twInfoTable;

/**
 * \brief Creates a new ::twInfoTable.
 *
 * \param[in]     shape         A pointer to the ::twDataShape that the
 *                              ::twInfoTable will use.
 *
 * \return A pointer to the newly allocated ::twInfoTable.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_Create(twDataShape * shape);

/**
 * \brief Creates a new ::twInfoTable from data in a ::twStream.
 *
 * \param[in]     s             A pointer to the ::twStream to parse.
 *
 * \return A pointer to the newly allocated ::twInfoTable.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_CreateFromStream(twStream * s);

/**
 * \brief Frees all memory associated with a ::twInfoTable structure and all
 * its owned substructures.
 *
 * \param[in]     entry         A pointer to the ::twInfoTable structure to
 *                              delete.
 *
 * \return Nothing.
*/
void twInfoTable_Delete(void * it);

/**
 * \brief Creates a new ::twInfoTable structure and copies all data from an
 * existing ::twInfoTable structure \p to the newly created ::twInfoTable
 * structure.
 *
 * \param[in]     it   A pointer to the ::twInfoTable to be copied from.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The original ::twInfoTable structure \p it will not be modified.  The
 * calling function will therefore retain ownership of \p it and is responsible
 * for freeing it via twInfoTable_Delete().
 * \note The calling function will gain ownership of the returned structure and
 * will be responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_FullCopy(twInfoTable * it);

/**
 * \brief Creates a new ::twInfoTable structure which inherits all pointers of
 * an existing ::twInfoTable structure \p it.  The pointers of \p it will be
 * zeroed so that it may be safely deleted.
 *
 * \param[in]     it    A pointer to the ::twInfoTable to be copied from and
 *                      zeroed.
 *
 * \return A pointer to the newly allocated ::twInfoTable.  Returns NULL on
 * failure.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
 * \note twInfoTable_ZeroCopy() does <b>not</b> free /p it.  It only zeros the
 * pointers of \p it so that the base structure \p it may be deleted without
 * modifying the newly created ::twInfoTable.  The calling function must still
 * free \p it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_ZeroCopy(twInfoTable * it);

/**
 * \brief Compares two ::twInfoTable structures for equivalence.
 *
 * \param[in]     p1            A pointer to the ::twInfoTable to compare
 *                              against.
 * \param[in]     p2            A pointer to the ::twInfoTable to compare.
 *
 * \return 0 if the two ::twInfoTable structures are identical, 1 if they are
 * not, -1 if an error was encountered.
*/
int twInfoTable_Compare(twInfoTable * p1, twInfoTable * p2);

/**
 * \brief Adds a ::twInfoTableRow to a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to add the
 *                              ::twInfoTableRow to.
 * \param[in]     row           A pointer to the ::twInfoTableRow to add.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The \p it ::twInfoTable will gain ownership of the \p row
 * ::twInfoTableRow.
*/
int twInfoTable_AddRow(twInfoTable * it, twInfoTableRow * row);

/**
 * \brief Retrieves the ::twInfoTableRow from a ::twInfoTable at the specified
 * (zero based) index.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to retrieve the
 *                              ::twInfoTableRow from.
 * \param[in]     index         The zero based index of the ::twInfoTableRow to
 *                              retrieve.
 *
 * \return A pointer to the ::twInfoTableRow retrieved.  Returns NULL if an
 * error was encountered.
 *
 * \note The ::twInfoTable will retain ownership of the ::twInfoTableRow.  The
 * calling function should <b>not</not> delete/free it.
*/
twInfoTableRow * twInfoTable_GetEntry(twInfoTable * it, int index);

/**
 * \brief Serializes a ::twInfoTable to a ::twStream.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to serialize.
 * \param[in,out] s             A pointer to the ::twStream to serialize to.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twInfoTable_ToStream(twInfoTable * it, twStream * s);

/**
 * \name Convenience Functions for Creating InfoTables
*/

/**
 * \brief Helper function to create a ::twInfoTable with a single field and
 * single row containing a type and value specified by \p value.
 *
 * \param[in]     name          The name to assign to the field.
 * \param[in]     value         A pointer to the ::twPrimitive structure to
 *                              create the ::twInfoTable from.
 *
 * \return A pointer to the resulting ::twInfoTable.  Returns NULL if an error
 * was encountered.
 *
 * \note The returned ::twInfoTable will <b>not</b> take ownership of the
 * ::twPrimitive \value.  The ownership of the ::twPrimitive is retained by the
 * calling function and must be freed via twPrimitive_Delete().
*/
twInfoTable * twInfoTable_CreateFromPrimitive(const char * name, twPrimitive * value);

/**
 * \brief Helper function to create a ::twInfoTable with a single field and
 * single row containing a #TW_STRING.
 *
 * \param[in]     name          The name to assign to the field.
 * \param[in]     value         A pointer to the string to create the ::twInfoTable from.
 * \param[in]     duplicate     Flag to indicate duplication preference.  If set
 *                              to TRUE, \p value will be copied.  If set to
 *                              FALSE, the ::twInfoTable will take ownership of
 *                              /p value.
 *
 * \return A pointer to the resulting ::twInfoTable.  Returns NULL if an error
 * was encountered.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_CreateFromString(const char * name, char * value, char duplicate);

/**
 * \brief Helper function to create a ::twInfoTable with a single field and
 * single row containing a #TW_NUMBER.
 *
 * \param[in]     name          The name to assign to the field.
 * \param[in]     value         The value to create the ::twInfoTable from.
 *
 * \return A pointer to the resulting ::twInfoTable.  Returns NULL if an error
 * was encountered.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_CreateFromNumber(const char * name, double value);

/**
 * \brief Helper function to create a ::twInfoTable with a single field and
 * single row containing a #TW_INTEGER.
 *
 * \param[in]     name          The name to assign to the field.
 * \param[in]     value         The value to create the ::twInfoTable from.
 *
 * \return A pointer to the resulting ::twInfoTable.  Returns NULL if an error
 * was encountered.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_CreateFromInteger(const char * name, int32_t value);

/**
 * \brief Helper function to create a ::twInfoTable with a single field and
 * single row containing a #TW_LOCATION.
 *
 * \param[in]     name          The name to assign to the field.
 * \param[in]     value         A pointer to the ::twLocation structure to
 *                              create the ::twInfoTable from.
 *
 * \return A pointer to the resulting ::twInfoTable.  Returns NULL if an error
 * was encountered.
 *
 * \note The created ::twInfoTable does <b>not</b> take ownership of the
 * ::twLocation pointer.
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_CreateFromLocation(const char * name, twLocation * value);

/**
 * \brief Helper function to create a ::twInfoTable with a single field and
 * single row containing a #TW_BLOB or #TW_IMAGE.
 *
 * \param[in]     name          The name to assign to the field.
 * \param[in]     value         A pointer to the char array containing the blob
 *                              or image.
 * \param[in]     length        The length of the array.
 * \param[in]     isImage       Flag indicating whether \p value is an
 *                              #TW_IMAGE (#TRUE) or a #TW_BLOB (#FALSE).
 * \param[in]     duplicate     Flag to indicate duplication preference.  If set
 *                              to TRUE, \p value will be copied.  If set to
 *                              FALSE, the ::twInfoTable will take ownership of
 *                              /p value.
 *
 * \return A pointer to the resulting ::twInfoTable.  Returns NULL if an error
 * was encountered.
 *
 * \note The created ::twInfoTable does <b>not</b> take ownership of the ::twLocation pointer.
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_CreateFromBlob(const char * name, char * value, int32_t length, char isImage, char duplicate);

/**
 * \brief Helper function to create a ::twInfoTable with a single field and
 * single row containing a #TW_DATETIME.
 *
 * \param[in]     name          The name to assign to the field.
 * \param[in]     value         The #DATETIME value to create the ::twInfoTable
 *                              from.
 *
 * \return A pointer to the resulting ::twInfoTable.  Returns NULL if an error
 * was encountered.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_CreateFromDatetime(const char * name, DATETIME value);

/**
 * \brief Helper function to create a ::twInfoTable with a single field and
 * single row containing a #TW_BOOLEAN.
 *
 * \param[in]     name          The name to assign to the field.
 * \param[in]     value         The boolean value to create the ::twInfoTable
 *                              from.
 *
 * \return A pointer to the resulting ::twInfoTable.  Returns NULL if an error
 * was encountered.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
*/
twInfoTable * twInfoTable_CreateFromBoolean(const char * name, char value);

/**
 * \name Convenience Functions for Accessing InfoTable Values
*/

/**
 * \brief Helper function to retrieve a TW_STRING from a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to get the value
 *                              from.
 * \param[in]     name          The name of the field to retrieve.
 * \param[in]     row           The (zero based) index of the row from which to
 *                              retrieve the value.
 * \param[in]     value         A pointer to store the retrieved data in.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twInfoTable_GetString(twInfoTable * it, const char * name, int32_t row, char ** value); 

/**
 * \brief Helper function to retrieve a TW_NUMBER from a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to get the value
 *                              from.
 * \param[in]     name          The name of the field to retrieve.
 * \param[in]     row           The (zero based) index of the row from which to
 *                              retrieve the value.
 * \param[in]     value         A pointer to store the retrieved data in.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/

int twInfoTable_GetNumber(twInfoTable * it, const char * name, int32_t row, double * value);
/**
 * \brief Helper function to retrieve a #TW_INTEGER from a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to get the value
 *                              from.
 * \param[in]     name          The name of the field to retrieve.
 * \param[in]     row           The (zero based) index of the row from which to
 *                              retrieve the value.
 * \param[in]     value         A pointer to store the retrieved data in.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twInfoTable_GetInteger(twInfoTable * it, const char * name, int32_t row, int32_t * value);

/**
 * \brief Helper function to retrieve a #TW_LOCATION from a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to get the value
 *                              from.
 * \param[in]     name          The name of the field to retrieve.
 * \param[in]     row           The (zero based) index of the row from which to
 *                              retrieve the value.
 * \param[in]     value         A pointer to store the retrieved data in.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twInfoTable_GetLocation(twInfoTable * it, const char * name, int32_t row, twLocation * value);

/**
 * \brief Helper function to retrieve a #TW_BLOB from a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to get the value
 *                              from.
 * \param[in]     name          The name of the field to retrieve.
 * \param[in]     row           The (zero based) index of the row from which to
 *                              retrieve the value.
 * \param[in]     value         A pointer to store the retrieved data in.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twInfoTable_GetBlob(twInfoTable * it, const char * name, int32_t row, char ** value, int32_t * length);

/**
 * \brief Helper function to retrieve a #TW_DATETIME from a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to get the value
 *                              from.
 * \param[in]     name          The name of the field to retrieve.
 * \param[in]     row           The (zero based) index of the row from which to
 *                              retrieve the value.
 * \param[in]     value         A pointer to store the retrieved data in.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twInfoTable_GetDatetime(twInfoTable * it, const char * name, int32_t row, DATETIME * value);

/**
 * \brief Helper function to retrieve a #TW_BOOLEAN from a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to get the value
 *                              from.
 * \param[in]     name          The name of the field to retrieve.
 * \param[in]     row           The (zero based) index of the row from which to
 *                              retrieve the value.
 * \param[in]     value         A pointer to store the retrieved data in.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twInfoTable_GetBoolean(twInfoTable * it, const char * name, int32_t row, char * value);

/**
 * \brief Helper function to retrieve a ::twPrimitive from a ::twInfoTable.
 *
 * \param[in]     it            A pointer to the ::twInfoTable to get the value
 *                              from.
 * \param[in]     name          The name of the field to retrieve.
 * \param[in]     row           The (zero based) index of the row from which to
 *                              retrieve the value.
 * \param[in]     value         A pointer to store the retrieved data in.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twInfoTable_GetPrimitive(twInfoTable * it, const char * name, int32_t row, twPrimitive ** value);

/**
 * \name JSON Conversion Utils
 *
 * \note Using any of these functions will cause the cJSON library to be linked
 * to your application, increasing the size of the application by ~22KB.
*/

/**
 * \brief Helper function that creates a ::twInfoTable from a cJSON object.  If
 * the JSON object does not represent a valid ::twInfoTable, a ::twInfoTable
 * with a single field, single row, of type JSON will be created.
 *
 * \param[in]     json            The cJSON object to convert to a
 *                                ::twInfoTable.
 * \param[in]     singleEntryName The name of the field to use for a single
 *                                entry table (may be NULL, defaults to
 *                                "_content_").
 *
 * \return A pointer to the newly allocated ::twInfoTable.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function gains ownership of the returned ::twInfoTable and
 * is responsible for freeing it via twInfoTable_Delete().
 *
 * \warning Using this function will cause the cJSON library to be linked to
 * your application, increasing the size of the application by ~22KB.
*/
twInfoTable * twInfoTable_CreateFromJson(struct cJSON * json, char * singleEntryName);

/**
 * \brief Helper function that outputs a ::twDataShape to a cJSON object.
 *
 * \param[in]     ds            The ::twDataShape to convert.
 * \param[in]     parent        The optional parent ::twDataShape to add the result to.
 *
 * \return A pointer to the newly allocated cJSON object.  Returns NULL if an
 * error was encountered.
*/
struct cJSON * twDataShape_ToJson(twDataShape * ds, struct cJSON * parent);

/**
 * \brief Helper function that outputs a ::twInfoTable to a cJSON object.
 *
 * \param[in]     it            The ::twInfoTable to convert.
 *
 * \return A pointer to the newly allocated cJSON object.  Returns NULL if an
 * error was encountered.
 *
 * \note The caller will gain ownership of the cJSON object and is responsible
 * for deleting it via cJSON_Delete().
 *
 * \warning Using this function will cause the cJSON library to be linked to
 * your application, increasing the size of the application by ~22KB.
*/
struct cJSON * twInfoTable_ToJson(twInfoTable * it);

#ifdef __cplusplus
}
#endif

#endif
