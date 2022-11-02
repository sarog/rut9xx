/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twBaseTypes.h
 * \brief ThingWorx ::BaseType definitions and functions.
*/

#ifndef BASETYPES_H
#define BASETYPES_H

#include "twOSPort.h"
#include "twDefinitions.h"
#include "twDefaultSettings.h"


#ifdef __cplusplus
extern "C" {
#endif

struct twInfoTable;
struct cJSON;

/**
 * \name ThingWorx Streams
*/

/**
 * \brief Dynamically allocated byte array.  Automatically expands its length as
 * needed.
 *
 * \note Usually not used by applications directly.
*/
typedef struct twStream {
	char * data;        /**< Pointer to array data. **/
	char * ptr;         /**< A pointer to the ::twStream's current position in ::twStream->data. **/
	uint32_t length;    /**< Length of the stream. **/
	uint32_t maxlength; /**< Maximum length of the stream. **/
	char ownsData;      /**< Data ownership flag. **/
	TW_FILE_HANDLE file /**< File handle if stream is used for file I/O. **/;
} twStream;

/**
 * \brief Creates a new ::twStream structure.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The calling function will gain ownership of the returned structure and
 * will be responsible for freeing it via twStream_Delete().
*/
twStream * twStream_Create();

/**
 * \brief Creates a new ::twStream structure and copies the data from a char
 * array to the new structure's ::twStream#data.
 *
 * \param[in]     data     A pointer to the char array to copy into
 *                         ::twStream#data.
 * \param[in]     length   The length of the char array.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The calling function will gain ownership of the returned structure and
 * will be responsible for freeing it via twStream_Delete().
 * \note The calling function will retain ownership of \p data and will is
 * responsible for freeing it.
*/
twStream * twStream_CreateFromCharArray(const char * data, uint32_t length);

/**
 * \brief Allocates a new ::twStream structure and points ::twStream#data to an
 * existing char array.
 *
 * \param[in]     data     A pointer to the char array to point ::twStream#data
 *                         to.
 * \param[in]     length   The length of the char array.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The newly allocated ::twStream will gain ownership of \p data.  The
 * calling function should <b>not</b> free it directly.
 * \note The calling function will gain ownership of the returned structure and
 * will be responsible for freeing it via twStream_Delete().
*/
twStream * twStream_CreateFromCharArrayZeroCopy(const char * data, uint32_t length);

/**
 * \brief Frees all memory associated with a ::twStream and all of its owned
 * substructures.
 *
 * \param[in]     s     A pointer to the ::twStream to be deleted.
 *
 * \return Nothing.
*/
void twStream_Delete(void* s);

/**
 * \brief Gets a pointer to the ::twStream#data of a ::twStream.
 *
 * \param[in]     s     A pointer to the ::twStream to get the data of.
 *
 * \return The ::twStream#data pointer of \p s.
*/
char * twStream_GetData(struct twStream * s);

/**
 * \brief Gets the ::twStream#index of a ::twStream.
 *
 * \param[in]     s     A pointer to the ::twStream to get the index of.
 *
 * \return The ::twStream#index of \p s.
*/
int32_t twStream_GetIndex(struct twStream * s);

/**
 * \brief Gets the ::twStream#length of a ::twStream.
 *
 * \param[in]     s     A pointer to the ::twStream to get the length of.
 *
 * \return The ::twStream#length of \p s.
*/
int32_t twStream_GetLength(struct twStream * s);

/**
 * \brief Adds data bytes to a ::twStream.
 *
 * \param[in]     s     A pointer to the ::twStream to add bytes to.
 * \param[in]     b     A pointer to the bytes to add.
 * \param[in]     count The number of bytes to be added.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twStream_AddBytes(struct twStream * s, void * b, uint32_t count);

/**
 * \brief Copies \p count bytes of data from the ::twStream#data of a
 * ::twStream into \p buf.
 *
 * \param[in]     s     A pointer to the ::twStream to get the bytes from.
 * \param[out]    buf   A pointer to a buffer to hold the copied data.
 * \param[in]     count The number of bytes to copy.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twStream_GetBytes(struct twStream * s, void * b, uint32_t count);

/**
 * \brief Resets the position pointer of a ::twStream (::twStream#ptr) to the
 * beginning of that ::twStream's data (::twStream#data).
 *
 * \param[in]     s     A pointer to the ::twStream to reset.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twStream_Reset(struct twStream * s);

/**
 * \brief Creates a stream from a file.
 *
 * \param[in]     fname     The name of the file to use for stream I/O.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
*/
twStream * twStream_CreateFromFile(const char * fname);

/**
 * \name ThingWorx Locations
*/

/**
 * \brief Location primitive structure.
*/
typedef struct twLocation {
	double latitude;  /**< Location latitude. **/
	double longitude; /**< Location longitude. **/
	double elevation; /**< Location elevation. **/
} twLocation;

/***************************************/
/*     Helper functions that are       */
/*    typically not directly used      */
/*     by application developers       */
/***************************************/
void swap2bytes(char * bytes);
void swap4bytes(char * bytes);
void swap8bytes(char * bytes);
int stringToStream(char * string, twStream * s);
char * streamToString(twStream * s);
enum BaseType baseTypeFromString(const char * s);
const char * baseTypeToString(enum BaseType b);

/**
 * \name ThingWorx Primitives
*/

/**
 * \brief ::BaseType primitive structure.
 * \note Used as a "variant" type throughout the entire API.
*/
typedef struct twPrimitive {
	enum BaseType type;       /**< The ::BaseType of the primitive. **/
	enum BaseType typeFamily; /**< The ::BaseType family of the primitive. **/
	uint32_t length;          /**< The length of the primitive. **/
	union {
		int32_t integer;     /**< TW_INTEGER value. **/
		double number;       /**< TW_NUMBER value. **/
		DATETIME datetime;   /**< TW_DATETIME value. **/
		twLocation location; /**< TW_LOCATION value. **/
		char boolean;        /**< TW_BOOLEAN value. **/
		struct {
			char * data; /**< Value data pointer. **/
			int32_t len; /**< Value data length. **/
		} bytes;
		struct twInfoTable * infotable; /**< ::twInfoTable value structure. **/
		struct twPrimitive * variant;   /**< Variant ::twPrimitive value. **/
	} val; /**< The value of the primitive. **/
} twPrimitive;


/**
 * \brief Creates a new ::twPrimitive structure.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The newly allocated ::twPrimitive structure defaults to a type of
 * #TW_NOTHING.
 * \note The calling function will gain ownership of the returned structure and
 * will be responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_Create();

/**
 * \brief Creates a new ::twPrimitive structure and populates it with data from
 * a ::twStream.
 *
 * \param[in]     s     A pointer to the ::twStream containing the data to
 *                      populate the ::twPrimitive with.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The newly allocated ::twPrimitive structure defaults to a type of
 * #TW_NOTHING.
 * \note The calling function will gain ownership of the returned structure and
 * will be responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromStream( twStream * s);

/**
 * \brief Allocates a new ::twPrimitive structure of a specified type and
 * populates it with data from a ::twStream.
 *
 * \param[in]     s     A pointer to the ::twStream containing the data to
 *                      populate the ::twPrimitive with.
 * \param[in]     type  The ::BaseType to associate with the ::twPrimitive.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The calling function will gain ownership of the returned structure and
 * will be responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromStreamTyped(twStream * s, enum BaseType type);

/**
 * \brief Creates a new ::twPrimitive structure which inherits all pointers of
 * an existing ::twPrimitive structure \p p.  The pointers of \p p will be
 * zeroed so that it may be safely deleted.
 *
 * \param[in]     p     A pointer to the ::twPrimitive to be copied from and
 *                      zeroed.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
 * \note twPrimitive_ZeroCopy() does <b>not</b> free /p p.It only zeros the
 * pointers of \p p so that the base structure \p p may be deleted without
 * modifying the newly created ::twPrimitive.  The calling function must still
 * free \p p via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_ZeroCopy(twPrimitive * p);

/**
 * \brief Creates a new ::twPrimitive structure and copies all data from an
 * existing ::twPrimitive structure \p to the newly created ::twPrimitive
 * structure.
 *
 * \param[in]     p     A pointer to the ::twPrimitive to be copied from.
 *
 * \return A pointer to the newly allocated structure.  Returns NULL on error.
 *
 * \note The original ::twPrimitive structure \p p will not be modified.  The
 * calling function will therefore retain ownership of \p p and is responsible
 * for freeing it via twPrimitive_Delete().
 * \note The calling function will gain ownership of the returned structure and
 * will be responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_FullCopy(twPrimitive * p);

/**
 * \brief Frees all memory associated with a ::twPrimitive and all of its owned
 * substructures.
 *
 * \param[in]     p     A pointer to the ::twPrimitive to be deleted.
 *
 * \return Nothing.
*/
void twPrimitive_Delete(void * p);

/**
 * \brief Copies the data from a ::twPrimitive structure to a ::twStream
 * structure.
 *
 * \param[in]     p     A pointer to the ::twPrimitive to write to \p s.
 * \param[in,out] s     A pointer to the ::twStream to write \p p to.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note This function is typically not used directly by applications.
*/
int twPrimitive_ToStream(twPrimitive * p, twStream * s);

/**
 * \brief Helper function to return the value of a #TW_STRING family type,
 * remove it from the ::twPrimitive \p p, and then delete \p, leaving the
 * caller with ownership of the returned #TW_STRING #TW_STRING.
 *
 * \param[in]     p     A pointer to the ::twPrimitive (must have
 *                      ::twPrimitive#typeFamily of type #TW_STRING).
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The calling function will gain ownership of the returned string and is
 * responsible for freeing it.
 * \note The returned string is guaranteed to not be NULL.
*/
char * twPrimitive_DecoupleStringAndDelete(twPrimitive * p);

/**
 * \brief Compares two ::twPrimitive structures for equivalence.
 *
 * \param[in]     p1    A pointer to the ::twPrimitive to be compared against.
 * \param[in]     p2    A pointer to the ::twPrimitive to compare with.
 *
 * \return 0 if the ::twPrimitive structures are identical, 1 if they are not
 * identical, -1 if an error was encountered.
*/
int twPrimitive_Compare(twPrimitive * p1, twPrimitive * p2);

/**
 * \brief Compares a ::twPrimitive to "true". For booleans true is if the value is "true",
 *        for numeric and dattime values true means non-zero,
 *        for string, blob & image types true means non-zero length,
 *        for locations true means one of longitude, latitude or altitude is non-zero,
 *        for infotables true means the table has at least one row
 *
 * \param[in]     p1    A pointer to the ::twPrimitive to be compared.
 *
 * \return 0 if the ::twPrimitive structures is "true", 1 if they are not
 * identical, -1 if an error was encountered.
*/
char twPrimitive_IsTrue(twPrimitive * p1) ;

/**
 * \name ::twPrimitive Convenience Functions
*/

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_LOCATION from a
 * location structure.
 *
 * \param[in]     value A pointer to the location structure.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function retains ownership of \p value and is responsible
 * for freeing it.
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromLocation(const twLocation * value);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_LOCATION from a
 * location structure.  Deletes the #TW_LOCATION after creating the primitive.
 *
 * \param[in]     value A pointer to the location structure.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function retains ownership of \p value and is responsible
 * for freeing it.
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromLocationAndDelete(const twLocation * value);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_NUMBER from a
 * double.
 *
 * \param[in]     value A numeric value to assign to the ::twPrimitive.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromNumber(const double value);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_INTEGER from an
 * integral type (int, short, char).
 *
 * \param[in]     value An integral value to assign to the ::twPrimitive.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromInteger(const int32_t value);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_DATETIME from a
 * #DATETIME type.
 *
 * \param[in]     value A #DATETIME to assign to the ::twPrimitive.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromDatetime(const DATETIME value);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_DATETIME from
 * the current time.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromCurrentTime();

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_BOOLEAN from a
 * char type.
 *
 * \param[in]     value A char value to assign to the ::twPrimitive.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromBoolean(const char value);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_INFOTABLE from
 * a ::twInfoTable structure.
 *
 * \param[in]     value A pointer to the ::twInfoTable structure.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function retains ownership of \p value and is responsible
 * for freeing it via twInfoTable_Delete().
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateFromInfoTable(struct twInfoTable * it);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_VARIANT from a
 * ::twPrimitive structure.
 *
 * \param[in]     value A pointer to the ::twPrimitive structure.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function retains ownership of \p value and is responsible
 * for freeing it via twPrimitive_Delete().
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete().
*/
twPrimitive * twPrimitive_CreateVariant(twPrimitive * input);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_STRING from a
 * null-terminated char array.
 *
 * \param[in]     value     A pointer to the null-terminated char array to
 *                          assign to the ::twPrimitive.
 * \param[in]     duplicate If #TRUE, a copy of the string is made.  If #FALSE,
 *                          the primitive takes ownership of the string.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note If \p duplicate is #TRUE, the calling function retains ownership of \p
 * value and is responsible for freeing it.
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete();
*/
twPrimitive * twPrimitive_CreateFromString(const char * value, char duplicate);

/**
 * \brief Helper function to create a ::twPrimitive of type #TW_BLOB or
 * #TW_IMAGE from a char array.
 *
 * \param[in]     value     A pointer to the char array to assign to the
 *                          ::twPrimitive.
 * \param[in]     length    The length of the char array.
 * \param[in]     isImage   If #TRUE, the returned ::twPrimitive#type is set to
 *                          #TW_IMAGE.  If FALSE, it is set to #TW_BLOB.
 * \param[in]     duplicate If #TRUE, a copy of the array is made.  If #FALSE,
 *                          the primitive takes ownership of the array.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note If \p duplicate is #TRUE, the calling function retains ownership of \p
 * value and is responsible for freeing it.
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete();
*/
twPrimitive * twPrimitive_CreateFromBlob(const char * value, int32_t length, char isImage, char duplicate);

/**
 * \brief Helper function to create a ::twPrimitive of any type from a void *.
 *
 * \param[in]     value              A pointer to the char array to assign to
 *                                   the ::twPrimitive.
 * \param[in]     type               The base type to assign to the new
 *                                   ::twPrimitive.
 * \param[in]     duplicateCharArray If #TRUE, a copy of the array is made.  If
 *                                   #FALSE, the primitive takes ownership of
 *                                   the array.
 * \param[in]     blobLength         The length of the char array if the type
 *                                   is #TW_BLOB or #TW_IMAGE.
 *
 * \return A pointer to the newly allocated ::twPrimitive.  Returns NULL if an
 * error was encountered.
 *
 * \note If \p duplicate is #TRUE, the calling function retains ownership of \p
 * value and is responsible for freeing it.
 * \note The calling function gains ownership of the returned ::twPrimitive and
 * is responsible for freeing it via twPrimitive_Delete();
*/
twPrimitive * twPrimitive_CreateFromVariable(const void * value, enum BaseType type, char duplicateCharArray, uint32_t blobLength);

/**
 * \name JSON Conversion Utils
 *
 * \note Using any of these functions will cause the cJSON library to be linked
 * to your application, increasing the size of the application by ~22KB.
*/

/**
 * \brief Helper function to convert a ::twPrimitive to a CJSON object.
 *
 * \param[in]     name    The name of the newly created JSON object.
 * \param[in]     p       A pointer to the ::twPrimitive to convert.
 * \param[in]     parent  (Optional) parent to add the JSON object to (may be
 *                        NULL).
 *
 * \return A pointer to the newly allocated cJSON object.  Returns NULL if an
 * error was encountered.
 *
 * \note The calling function will retain ownership of the \p parent object and
 * is responsible for freeing it if necessary.
 * \note The calling function will gain ownership of the newly allocated cJSON
 * object and is responsible for freeing it.
 *
 * \warning Using this function will cause the cJSON library to be linked to
 * your application, increasing the size of the application by ~22KB.
*/
struct cJSON * twPrimitive_ToJson(const char * name, twPrimitive * p, struct cJSON * parent);

/**
 * \brief Helper function to convert a cJSON object to a ::twPrimitive.
 *
 * \param[in]     j       A pointer to the cJSON object to convert.
 * \param[in]     name    The name of the element that is to be converted.
 * \param[in]     type    The ::BaseType of the ::twPrimitive to be created.
 *
 * \return A pointer to the newly allocated ::twPrimitive object.  Returns NULL
 * if an error was encountered.
 *
 * \note The calling function will gain ownership of the newly allocated cJSON
 * object and is responsible for freeing it.
 *
 * \warning Using this function will cause the cJSON library to be linked to
 * your application, increasing the size of the application by ~22KB.
*/
twPrimitive * twPrimitive_CreateFromJson(struct cJSON * j, char * name, enum BaseType type);

#ifdef __cplusplus
}
#endif

#endif
