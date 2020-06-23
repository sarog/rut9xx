/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx Infotable
 */

#include "twInfoTable.h"
#include "twLogger.h"
#include "stringUtils.h"
#include "twBaseTypes.h"
#include "cJSON.h"

#include <string.h>

#define MAX_ENTITY_NAME_LEN 2048

/* Private API Usage*/
void twList_Unlock(struct twList *list);
void twList_Lock(struct twList *list);

/* Data shape aspect */
twDataShapeAspect * twDataShapeAspect_Create(const char * name, twPrimitive * value) {
	twDataShapeAspect * aspect = NULL;
	if (!name || !value) {
		TW_LOG(TW_ERROR,"twDataShapeAspect_Create: Missing input parameter");
		return NULL;
	}
	aspect = (twDataShapeAspect *)TW_CALLOC(sizeof(twDataShapeAspect), 1);
	if (!aspect) {
		TW_LOG(TW_ERROR,"twDataShapeAspect_Create: Error allocating storage");
		return NULL;
	}
	aspect->name = duplicateString(name);
	aspect->value = value;
	return aspect;
}

twDataShapeAspect * twDataShapeAspect_CreateFromStream(twStream * s) {
	char * name;
	twPrimitive * p = NULL;
	twDataShapeAspect * aspect = NULL;
	if (!s) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: NULL stream pointer");
		return NULL;
	}
	name = streamToString(s);
	p = twPrimitive_CreateFromStream(s);
	if (!p || !name) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: Error getting name or primitve");
		return NULL;
	}
	aspect = twDataShapeAspect_Create(name, p);
	TW_FREE(name);
	if (!aspect) twPrimitive_Delete(p);
	return aspect;
}

void twDataShapeAspect_Delete(void * aspect) {
	twDataShapeAspect * tmp = (twDataShapeAspect *) aspect;
	if (!tmp) {
		TW_LOG(TW_ERROR,"twDataShapeAspect_Delete: Missing input parameter");
		return;
	}
	TW_FREE(tmp->name);
	twPrimitive_Delete(tmp->value);
	TW_FREE(tmp);
}

/* Data shape entry */
twDataShapeEntry * twDataShapeEntry_Create(const char * name, const char * description, enum BaseType type) {
	twDataShapeEntry * entry = NULL;
	if (!name) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_Create: Must supply a name and BaseType");
		return NULL;
	}
	entry = (twDataShapeEntry *)TW_CALLOC(sizeof(twDataShapeEntry),1);
	if (!entry) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_Create: Error allocating storage");
		return NULL;
	}
	entry->name = duplicateString(name);
	if (description) entry->description = duplicateString(description);
	entry->type = type;
	entry->aspects = twList_Create(twDataShapeAspect_Delete);
	if (!entry->aspects) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_Create: Error allocating aspect list");
		twDataShapeEntry_Delete(entry);
		return NULL;
	}
	return entry;
}

twDataShapeEntry * twDataShapeEntry_CreateFromStream(twStream * s) {
	char byte = 0;
	unsigned char aspectCountBytes[2];
	uint16_t i, aspectCount = 0;
	twDataShapeEntry * entry = NULL;
	if (!s) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: NULL stream pointer");
		return NULL;
	}
	/* Look for the marker */
	if (twStream_GetBytes(s, &byte, 1)) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: Error getting marker");
		return NULL;
	}
	if (!byte) {
		/* no marker, must be at the end */
		return NULL;
	}
	/* Allocate our memory */
	entry = (twDataShapeEntry *)TW_CALLOC(sizeof(twDataShapeEntry),1);
	if (!entry) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: Error allocating storage");
		return NULL;
	}
	entry->aspects = twList_Create(twDataShapeAspect_Delete);
	if (!entry->aspects) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: Error allocating aspect list");
		twDataShapeEntry_Delete(entry);
		return NULL;
	}

	/*Get the name */
	if (!(entry->name = streamToString(s))) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: Error getting name");
		twDataShapeEntry_Delete(entry);
		return NULL;
	}
	/* Get the description */
	entry->description = streamToString(s);
	/* Get the base type */
	if (twStream_GetBytes(s, &byte, 1)) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: Error base type");
		twDataShapeEntry_Delete(entry);
		return NULL;
	}
	entry->type = (enum BaseType)byte;
	/* Get the aspect count */
	if (twStream_GetBytes(s, &aspectCountBytes, 2)) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_CreateFromStream: Error aspect count");
		twDataShapeEntry_Delete(entry);
		return NULL;
	}
	aspectCount = aspectCountBytes[0] * 256 + aspectCountBytes[1];
	for (i = 0; i < aspectCount; i++) {
		twList_Add(entry->aspects, twDataShapeAspect_CreateFromStream(s));
	}
	return entry;
}

void twDataShapeEntry_Delete(void * entry) {
	twDataShapeEntry * tmp = NULL;
	tmp = (twDataShapeEntry *)entry;
	if (!entry) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_Delete: Missing input");
		return;
	}
	TW_FREE(tmp->name);
	if (tmp->description) TW_FREE(tmp->description);
	if (tmp->aspects) twList_Delete(tmp->aspects);
	TW_FREE(entry);
}

int twDataShapeEntry_AddAspect(struct twDataShapeEntry * entry, const char * name, twPrimitive * value) {
	if (!entry || !entry->aspects || !name || !value) return TW_INVALID_PARAM;
	return twList_Add(entry->aspects, twDataShapeAspect_Create(name, value));
}

uint32_t twDataShapeEntry_GetLength(struct twDataShapeEntry * entry) {
	uint32_t len = 0;
	uint32_t strLength = 0;
	if (!entry || !entry->name) return 0;
	strLength = strnlen(entry->name, MAX_ENTITY_NAME_LEN);
	len += strLength;
	len += (strLength > 127) ? 4:1;
	if (entry->description) strLength = strnlen(entry->description, MAX_ENTITY_NAME_LEN);
	else strLength = 0;
	len += strLength;
	len += (strLength > 127) ? 4:1;
	len++; /* Basetype */
	len += 2; /* Aspect count */
	if (entry->aspects) {
		ListEntry * le = NULL;
		twDataShapeAspect * a = NULL;
		le = twList_Next(entry->aspects,NULL);
		while (le) {
			a = (twDataShapeAspect *)le->value;
			if (a && a->name && a->value) {
				len += strnlen(a->name, MAX_ENTITY_NAME_LEN) + 1;
				len += a->value->length;
				len++; /* For Base Type */
			}
			le = twList_Next(entry->aspects,le);
		}
	}
	return len;
}

int twDataShapeEntry_ToStream(struct twDataShapeEntry * entry, twStream * s) {
	char marker = 0x01;
	uint16_t aspectCount = 0;
	int res = TW_OK;
	char byte = 0;
	ListEntry * le = NULL;
	if (!s || !entry || !entry->name) {
		TW_LOG(TW_ERROR,"twDataShapeEntry_ToStream: Missing inputs or entry name");
		return TW_INVALID_PARAM;
	}
	byte = (char)entry->type;
	/* Add the marker */
	res = twStream_AddBytes(s, &marker, 1);
	/* Add the name */
	res |= stringToStream(entry->name, s);
	/* Add the description */
	res |= stringToStream(entry->description, s);
	/* Add the base type */
	res |= twStream_AddBytes(s, &byte, 1);
	/* Add the aspects if any or a zero if none */
	if (entry->aspects && entry->aspects->count) {
		aspectCount = (uint16_t)twList_GetCount(entry->aspects);
		/* Correct for endianess */
		byte = aspectCount / 256;
		res |= twStream_AddBytes(s, &byte, 1);
		byte = aspectCount % 256;
		res |= twStream_AddBytes(s, &byte, 1);
		le = twList_Next(entry->aspects, NULL);
		while (le && le->value) {
			twDataShapeAspect * aspect = (twDataShapeAspect *)le->value;
			stringToStream(aspect->name, s);
			twPrimitive_ToStream(aspect->value, s);
			le = twList_Next(entry->aspects, le);
		}
	} else {
		res |= twStream_AddBytes(s, &aspectCount, 2);
	}
	return res;
}

/* Data Shape */
twDataShape * twDataShape_Create(twDataShapeEntry * firstEntry) {
	twDataShape * shape = NULL;
	shape = (twDataShape *)TW_CALLOC(sizeof(twDataShape),1);
	if (!shape) {
		TW_LOG(TW_ERROR,"twDataShape_Create: Error allocating storagefor shape");
		return NULL;
	}
	shape->entries = twList_Create(twDataShapeEntry_Delete);
	if (!shape->entries) {
		TW_LOG(TW_ERROR,"twDataShape_Create: Error allocating storage for entries");
		twDataShape_Delete(shape);
		return NULL;
	}
	if (firstEntry) {
		if (twList_Add(shape->entries, firstEntry) != TW_OK) {
			TW_LOG(TW_ERROR,"twDataShape_Create: Error adding first entry");
			twDataShape_Delete(shape);
		}
		shape->numEntries = 1;
	}
	return shape;
}

twDataShape * twDataShape_CreateFromStream(twStream * s) {
	twDataShape * ds = NULL;
	twDataShapeEntry * entry = NULL;
	if (!s) {
		TW_LOG(TW_ERROR,"twDataShape_CreateFromStream: NULL stream pointer");
		return NULL;
	}
	ds = (twDataShape *)TW_CALLOC(sizeof(twDataShape),1);
	if (!ds) {
		TW_LOG(TW_ERROR,"twDataShape_CreateFromStream: Error allocating storage");
		return NULL;
	}
	ds->entries = twList_Create(twDataShapeEntry_Delete);
	if (!ds->entries) {
		TW_LOG(TW_ERROR,"twDataShape_CreateFromStream: Error allocating entry list");
		return NULL;
	}
	while ((entry = twDataShapeEntry_CreateFromStream(s)) != 0) {
		twList_Add(ds->entries, entry);
		ds->numEntries++;
	}
	/* Ge the end marker */
	/*twStream_GetBytes(s, &byte, 1);*/
	return ds;
}

void twDataShape_Delete(void * ds) {
	twDataShape * tmp = NULL;
	tmp = (twDataShape *)ds;
	if (!ds) {
		TW_LOG(TW_ERROR,"twDataShape_Delete: Missing input");
		return;
	}
	if (tmp->entries) {
		twList_Delete(tmp->entries);
		/* setting pointer to NULL so subsequent
		Null check error conditions are valid */
		tmp->entries = NULL;
	}
	if (tmp->name) {
		TW_FREE(tmp->name);
		/* setting pointer to NULL so subsequent
		Null check error conditions are valid */
		tmp->name = NULL;
	}
	TW_FREE(ds);
}

int twDataShape_SetName(struct twDataShape * ds, char * name) {
	if (!name || !ds ) {
		TW_LOG(TW_ERROR,"twDataShape_SetName: Missing inputs or entry list");
		return TW_INVALID_PARAM;
	}
	ds->name = duplicateString(name);
	return TW_OK;
}

int twDataShape_AddEntry(twDataShape * ds, twDataShapeEntry * entry) {
	if (!entry || !ds || !ds-> entries) {
		TW_LOG(TW_ERROR,"twDataShape_AddEntry: Missing inputs or entry list");
		return TW_INVALID_PARAM;
	}
	if (twList_Add(ds->entries, entry) == TW_OK) {
		ds->numEntries++;
		return TW_OK;
	}
	return TW_ERROR_ADDING_DATASHAPE_ENTRY;
}

uint32_t twDataShape_GetLength(struct twDataShape * ds) {
	uint32_t len = 0;
	ListEntry * le = NULL;
	if (!ds || !ds->entries) return 0;
	le = twList_Next(ds->entries, NULL);
	while (le) {
		twDataShapeEntry * e = (twDataShapeEntry *)le->value;
		if (e) {
			len++; /* marker */
			len += twDataShapeEntry_GetLength(e);
		}
		le = twList_Next(ds->entries, le);
	}
	/* Add the final 0 marker */
	len++;
	return len;
}

int twDataShape_GetEntryIndex(struct twDataShape * ds, const char * name, int * index) {
	ListEntry * le = NULL;
	twDataShapeEntry * entry = NULL;
	int res = 0;
	if (!name || !ds || !ds-> entries || !index) {
		TW_LOG(TW_ERROR,"twDataShape_GetEntryIndex: Missing inputs or entry list");
		if (index) *index = -1;
		return TW_INVALID_PARAM;
	}
	*index = -1;
	/* Add the fields */
	le = twList_Next(ds-> entries, NULL);
	while (le) {
		entry = (twDataShapeEntry *)le->value;
		if (entry && entry->name) {
			if (!strcmp(entry->name, name)) {
				*index = res;
				return TW_OK;
			}
		}
		res++;
		le = twList_Next(ds-> entries, le);
	}
	/* Didn't find a match */
	return TW_INDEX_NOT_FOUND;
}

int twDataShape_ToStream(twDataShape * ds, twStream * s) {
	char marker = 0x00;
	ListEntry * le = NULL;
	if (!s || !ds || !ds-> entries) {
		TW_LOG(TW_ERROR,"twDataShape_ToStream: Missing inputs or entry list");
		return TW_INVALID_PARAM;
	}
	le = twList_Next(ds->entries, NULL);
	while (le) {
		twDataShapeEntry * entry = (twDataShapeEntry *)le->value;
		twDataShapeEntry_ToStream(entry, s);
		le = twList_Next(ds->entries, le);
	}
	/* Add the terminating marker */
	twStream_AddBytes(s, &marker, 1);
	return TW_OK;
}

/* InfoTable Row */
twInfoTableRow * twInfoTableRow_Create(twPrimitive * firstEntry) {
	twInfoTableRow * row = NULL;
	if (!firstEntry) {
		TW_LOG(TW_ERROR,"twInfoTableRow_Create: Must supply an intitial row entry");
		return NULL;
	}
	row = (twInfoTableRow *)TW_CALLOC(sizeof(twInfoTableRow),1);
	if (!row) {
		TW_LOG(TW_ERROR,"twInfoTableRow_Create: Error allocating storage for row");
		return NULL;
	}
	row->fieldEntries = twList_Create(twPrimitive_Delete);
	if (!row->fieldEntries) {
		TW_LOG(TW_ERROR,"twInfoTableRow_Create: Error allocating storage for field entries");
		twDataShape_Delete(row);
		return NULL;
	}
	twList_Add(row->fieldEntries, firstEntry);
	row->numFields = 1;
	return row;
}

twInfoTableRow * twInfoTableRow_CreateFromStream(twStream * s) {
	char byte = 0;
	twPrimitive * p = NULL;
	unsigned char fieldCountBytes[2];
	uint16_t i = 0;
	twInfoTableRow * row = NULL;
	if (!s) {
		TW_LOG(TW_ERROR,"twInfoTableRow_CreateFromStream: NULL stream pointer");
		return NULL;
	}
	/* Look for the marker */
	if (twStream_GetBytes(s, &byte, 1)) {
		TW_LOG(TW_ERROR,"twInfoTableRow_CreateFromStream: Error getting marker");
		return NULL;
	}
	if (!byte) {
		TW_LOG(TW_TRACE,"twInfoTableRow_CreateFromStream: Marker is empty. No more rows in stream");
		return NULL;
	}
	/* Allocate our storage */
	row = (twInfoTableRow *)TW_CALLOC(sizeof(twInfoTableRow),1);
	if (!row) {
		TW_LOG(TW_ERROR,"twInfoTableRow_CreateFromStream: Error allocating storage");
		return NULL;
	}
	row->fieldEntries = twList_Create(twPrimitive_Delete);
	if (!row->fieldEntries) {
		TW_LOG(TW_ERROR,"twInfoTableRow_CreateFromStream: Error allocating storage");
		twInfoTableRow_Delete(row);
		return NULL;
	}
	/* Get the field count */
	if (twStream_GetBytes(s, &fieldCountBytes, 2)) {
		TW_LOG(TW_ERROR,"twInfoTableRow_CreateFromStream: Error getting field count");
		return NULL;
	}
	row->numFields = fieldCountBytes[0] * 256 + fieldCountBytes[1];
	for (i = 0; i < row->numFields; i++) {
		p = twPrimitive_CreateFromStream(s);
		if (!p) {
			TW_LOG(TW_ERROR,"twInfoTableRow_Create: Error getting field entrys");
			twInfoTableRow_Delete(row);
			return NULL;
		}
		twList_Add(row->fieldEntries,p);
	}
	return row;
}

void twInfoTableRow_Delete(void * row) {
	twInfoTableRow * tmp = (twInfoTableRow *)row;
	if (!row) {
		TW_LOG(TW_ERROR,"twInfoTableRow_Delete: Missing input");
		return;
	}
	if (tmp->fieldEntries) twList_Delete(tmp->fieldEntries);
	TW_FREE(row);
}

int twInfoTableRow_GetCount(twInfoTableRow * row) {
	if (!row) {
		TW_LOG(TW_ERROR,"twInfoTableRow_GetCount: Missing input");
		return 0;
	}
	return row->numFields;
}

uint32_t twInfoTableRow_GetLength(twInfoTableRow * row) {
	uint32_t len = 2; /* Field count size */
	ListEntry * le = NULL;
	if (!row || !row->fieldEntries) {
		TW_LOG(TW_ERROR,"twInfoTableRow_GetCount: Missing input");
		return 0;
	}
	le = twList_Next(row->fieldEntries,NULL);
	while (le) {
		twPrimitive * p = (twPrimitive *)le->value;
		len++; /* base type */
		if (p) len += p->length;
		le = twList_Next(row->fieldEntries,le);
	}
	return len;
}

int twInfoTableRow_AddEntry(twInfoTableRow * row, twPrimitive * entry) {
	if (!row || !entry ||!row->fieldEntries) {
		TW_LOG(TW_ERROR,"twInfoTableRow_AddEntry: Missing input or field entry list");
		return TW_INVALID_PARAM;
	}
	twList_Add(row->fieldEntries, entry);
	row->numFields++;
	return TW_OK;
}

twPrimitive * twInfoTableRow_GetEntry(twInfoTableRow * row, int index) {
	ListEntry * le = NULL;
	if (!row || !row->fieldEntries) {
		TW_LOG(TW_ERROR,"twInfoTableRow_AddEntry: Missing row or field entry list");
		return NULL;
	}
	le = twList_GetByIndex(row->fieldEntries, index);
	if (le) return (twPrimitive *)(le->value);
	return NULL;
}
int twInfoTableRow_ToStreamForEach(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	twStream * s = (twStream *)arg;
	twPrimitive_ToStream((twPrimitive *)data, s);
	return TW_FOREACH_CONTINUE;
}

int twInfoTableRow_ToStream(twInfoTableRow * row, twStream * s) {
	char marker = 1;
	char count[2];
	if (!row || !s) {
		TW_LOG(TW_ERROR,"twInfoTable_Create: Missing inputs");
		return TW_INVALID_PARAM;
	}
	/* Add the marker */
	twStream_AddBytes(s, &marker, 1);
	/* Add the number of fields */
	count[0] = (unsigned char)row->numFields/256;
	count[1] = (unsigned char)row->numFields % 256;
	twStream_AddBytes(s, &count, 2);
	/* Add the fields */
	twList_Foreach(row->fieldEntries, twInfoTableRow_ToStreamForEach, (void *) s);
	return TW_OK;
}

/* Infotable */
twInfoTable * twInfoTable_Create(twDataShape * shape) {
	twInfoTable * it = NULL;
	if (!shape) {
		TW_LOG(TW_ERROR,"twInfoTable_Create: DataShape required");
		return NULL;
	}
	it = (twInfoTable *)TW_CALLOC(sizeof(twInfoTable), 1);
	if (!it) {
		TW_LOG(TW_ERROR,"twInfoTable_Create: Error allocating InfoTable structure");
		return NULL;
	}
	it->ds = shape;
	it->length = twDataShape_GetLength(shape) + 1;  /* +1 for Terminating Row marker */
	it->rows = twList_Create(twInfoTableRow_Delete);
	it->mtx = twMutex_Create();
	if (!it->rows || !it->mtx) {
		TW_LOG(TW_ERROR,"twInfoTable_Create: Error allocating InfoTable row list or mutex");
		twInfoTable_Delete(it);
		return NULL;
	}
	return it;
}

twInfoTable * twInfoTable_CreateFromStream(twStream * s) {
	twInfoTableRow * row = NULL;
	uint32_t start = twStream_GetIndex(s);
	twInfoTable * it;
	if (!s) {
		TW_LOG(TW_ERROR,"twInfoTable_CreateFromStream: NULL stream pointer");
		return NULL;
	}
	it = (twInfoTable *)TW_CALLOC(sizeof(twInfoTable),1);
	if (!it) {
		TW_LOG(TW_ERROR,"twInfoTable_CreateFromStream: Error allocating storage");
		return NULL;
	}
	it->rows = twList_Create(twInfoTableRow_Delete);
	if (!it->rows) {
		TW_LOG(TW_ERROR,"twInfoTable_CreateFromStream: Error allocating InfoTable structure");
		twInfoTable_Delete(it);
		return NULL;
	}
	/* Get the datashape */
	it->ds = twDataShape_CreateFromStream(s);
	if (!it->ds) {
		TW_LOG(TW_ERROR,"twInfoTable_CreateFromStream: Error getting datashape");
		twInfoTable_Delete(it);
		return NULL;
	}
	/* Get our rows */
	row = twInfoTableRow_CreateFromStream(s);
	while (row) {
		twList_Add(it->rows, row);
		row = twInfoTableRow_CreateFromStream(s);
	}
	it->length = twStream_GetIndex(s) - start;
	it->mtx = twMutex_Create();
	if (!it->mtx) {
		TW_LOG(TW_ERROR,"twInfoTable_CreateFromStream: Error allocating InfoTable mutex");
		twInfoTable_Delete(it);
		return NULL;
	}
	return it;
}

twInfoTable * twInfoTable_FullCopy(twInfoTable * it) {
	twInfoTable * res = NULL;
	twStream * s = NULL;
	if (!it) return 0;
	s = twStream_Create();
	if (!s) return NULL;
	twInfoTable_ToStream(it,s);
	twStream_Reset(s);
	res = twInfoTable_CreateFromStream(s);
	twStream_Delete(s);
	return res;
}

twInfoTable * twInfoTable_ZeroCopy(twInfoTable * it) {
	twInfoTable * cp = NULL;
	if (!it) return NULL;
	cp = (twInfoTable *)TW_CALLOC(sizeof(twInfoTable), 1);
	if (!cp) return NULL;
	twMutex_Lock(it->mtx);
	cp->ds = it->ds;
	cp->rows = it->rows;
	cp->length = it->length;
	cp->mtx = it->mtx;
	it->ds = NULL;
	it->rows = NULL;
	twMutex_Unlock(it->mtx);
	it->mtx = 0;
	return cp;
}

void twInfoTable_Delete(void * it) {
	TW_MUTEX m = 0;
	twInfoTable * tmp = (twInfoTable *)it;
	if (!it) {
		return;
	}
	m = tmp->mtx;
	twMutex_Lock(m);
	tmp->mtx = NULL;
	if (tmp->ds){
		twDataShape_Delete(tmp->ds);
		/* setting pointer to NULL so subsequent
		Null check error conditions are valid */
		tmp->ds = NULL;
	}
	if (tmp->rows) {
		twList_Delete(tmp->rows);
		/* setting pointer to NULL so subsequent
		Null check error conditions are valid */
		tmp->rows = NULL;
	}
	TW_FREE(tmp);
	twMutex_Unlock(m);
	if (m) twMutex_Delete(m);
}

int twInfoTable_Compare(twInfoTable * p1, twInfoTable * p2) {
	ListEntry * l1 = NULL;
	ListEntry * l2 = NULL;
	if (!p1 || !p2 || !p1->ds || !p2->ds || !p1->ds->entries || !p2->ds->entries) return -1;
	/* Look for identical infotables and obvious inequalities */
	if (p1 == p2) return 0;
	if (p1->ds == p2->ds && p1->rows == p2->rows) return 0;
	if (p1->length != p2->length) return 1;
	/* Check the DataShapes */

		twList_Lock(p1->ds->entries);
		twList_Lock(p2->ds->entries);

		l1 = p1->ds->entries->first;
		l2 = p2->ds->entries->first;

		while (l1 && l2) {
			twDataShapeEntry *dse1 = (twDataShapeEntry *) l1->value;
			twDataShapeEntry *dse2 = (twDataShapeEntry *) l2->value;
			if (!dse1 || !dse2) return -1;
			if (strcmp(dse1->name, dse2->name)) return 1;
			if (dse1->type != dse2->type) return 1;
			l1 = l1->next;
			l2 = l2->next;
		}
		/*
		If we are here, the dataShape names and types are the same.  We just
		check the number of rows to determine if it there has been a change
		*/
		if (p1->rows->count != p2->rows->count) {
			twList_Unlock(p1->ds->entries);
			twList_Unlock(p2->ds->entries);
			return 1;
		} else {
			twList_Unlock(p1->ds->entries);
			twList_Unlock(p2->ds->entries);
			return 0;
		}
}

int twInfoTable_AddRow(twInfoTable * it, twInfoTableRow * row) {
	int res = 0;
	if (!it || !row || !it->rows) {
		TW_LOG(TW_ERROR,"twInfoTable_AddRow: NULL input or row list");
		return TW_INVALID_PARAM;
	}
	twMutex_Lock(it->mtx);
	it->length++; /* Row Marker */
	it->length +=  twInfoTableRow_GetLength(row);
	res = twList_Add(it->rows, row);
	twMutex_Unlock(it->mtx);
	return res;
}

twInfoTableRow * twInfoTable_GetEntry(twInfoTable * it, int index) {
	ListEntry * le = NULL;
	if (!it || !it->rows) {
		TW_LOG(TW_ERROR,"twInfoTable_GetEntry: NULL input or row list");
		return NULL;
	}
	le = twList_GetByIndex(it->rows, index);
	if (le) return (twInfoTableRow *)le->value;
	return NULL;
}

int twInfoTable_ToStreamForEach(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	twStream * s = (twStream *)arg;
	twInfoTableRow *row = (twInfoTableRow *) data;
	twInfoTableRow_ToStream(row, s);
	return TW_FOREACH_CONTINUE;
}

int twInfoTable_ToStream(twInfoTable * it, twStream * s) {
	char marker = 0;
	if (!it || !it->rows || !it->ds || !s) {
		TW_LOG(TW_ERROR,"twInfoTable_GetEntry: NULL input, data shape, stream or row list");
		return TW_INVALID_PARAM;
	}
	twMutex_Lock(it->mtx);
	twDataShape_ToStream(it->ds, s);

	twList_Foreach(it->rows,twInfoTable_ToStreamForEach,(void*)s);

	/* Add the terminating marker */
	twStream_AddBytes(s, &marker, 1);
	twMutex_Unlock(it->mtx);


	return TW_OK;
}

/* Convenience functions - single field, single row infotables */
twInfoTable * twInfoTable_CreateFromPrimitive(const char * name, twPrimitive * value) {
	twDataShapeEntry * ds = NULL;
	twInfoTable * it = NULL;
	if (!value) {
		TW_LOG(TW_ERROR,"twInfoTable_CreateFromPrimitive: NULL pointer to primitive passed in");
		return NULL;
	}
	ds = twDataShapeEntry_Create(name, "", value->type);
	it = twInfoTable_Create(twDataShape_Create(ds));
	if (!it) {
		TW_LOG(TW_ERROR,"twInfoTable_CreateFromPrimitive: Error creating infotable");
		return NULL;
	}
	twInfoTable_AddRow(it, twInfoTableRow_Create(value));
	return it;
}

twInfoTable * twInfoTable_CreateFromString(const char * name, char * value, char duplicate) {
	return twInfoTable_CreateFromPrimitive(name, twPrimitive_CreateFromString(value, duplicate));
}

twInfoTable * twInfoTable_CreateFromNumber(const char * name, double value)  {
	return twInfoTable_CreateFromPrimitive(name, twPrimitive_CreateFromNumber(value));
}

twInfoTable * twInfoTable_CreateFromInteger(const char * name, int32_t value)  {
	return twInfoTable_CreateFromPrimitive(name, twPrimitive_CreateFromInteger(value));
}

twInfoTable * twInfoTable_CreateFromLocation(const char * name, twLocation * value)  {
	return twInfoTable_CreateFromPrimitive(name, twPrimitive_CreateFromLocation(value));
}

twInfoTable * twInfoTable_CreateFromBlob(const char * name, char * value, int32_t length, char isImage, char duplicate)  {
	return twInfoTable_CreateFromPrimitive(name, twPrimitive_CreateFromBlob(value, length, isImage, duplicate));
}

twInfoTable * twInfoTable_CreateFromDatetime(const char * name, DATETIME value)  {
	return twInfoTable_CreateFromPrimitive(name, twPrimitive_CreateFromDatetime(value));
}

twInfoTable * twInfoTable_CreateFromBoolean(const char * name, char value)  {
	return twInfoTable_CreateFromPrimitive(name, twPrimitive_CreateFromBoolean(value));
}

twPrimitive * checkAndGetRow(twInfoTable * it, const char * name, int32_t row, void * value) {
	twInfoTableRow * rowData = NULL;
	twPrimitive * p = NULL;
	int index = -1;
	if (!it || !it->ds || !it->rows  || !it->mtx || !name || !value) return 0;
	/* Get the index of the name in the datashape */
	twMutex_Lock(it->mtx);
	twDataShape_GetEntryIndex(it->ds, name, &index);
	if (index == -1) {
		TW_LOG(TW_WARN,"InfoTable precheck: Name \"%s\" not found in datashape \"%s\"", name, it->ds->name);
		twMutex_Unlock(it->mtx);
		return NULL;
	}
	/* Get the row data */
	rowData = twInfoTable_GetEntry(it, row);
	if (!rowData) {
		TW_LOG(TW_WARN,"InfoTable precheck: Row %d not found in infotable", row);
		twMutex_Unlock(it->mtx);
		return NULL;
	}
	/* Get the value of the index in the row data */
	p = twInfoTableRow_GetEntry(rowData, index);
	twMutex_Unlock(it->mtx);
	return p;
}

int twInfoTable_GetPrimitive(twInfoTable * it, const char * name, int32_t row, twPrimitive ** value) {
	if (!value) return TW_INVALID_PARAM;
	*value = checkAndGetRow(it, name, row, value);
	if (!*value) return TW_ERROR_GETTING_PRIMITIVE;
	return 0;
}

int twInfoTable_GetString(twInfoTable * it, const char * name, int32_t row, char ** value) {
	twPrimitive * p = checkAndGetRow(it, name, row, value);
	if (!p) return TW_INVALID_PARAM;
	*value = duplicateString(p->val.bytes.data);
	return TW_OK;
}

int twInfoTable_GetNumber(twInfoTable * it, const char * name, int32_t row, double * value)  {
	twPrimitive * p = checkAndGetRow(it, name, row, value);
	if (!p) return TW_INVALID_PARAM;
	*value = p->val.number;
	return TW_OK;
}

int twInfoTable_GetInteger(twInfoTable * it, const char * name, int32_t row, int32_t * value) {
	twPrimitive * p = checkAndGetRow(it, name, row, value);
	if (!p) return TW_INVALID_PARAM;
	*value = p->val.integer;
	return TW_OK;
}

int twInfoTable_GetLocation(twInfoTable * it, const char * name, int32_t row, twLocation * value) {
	twPrimitive * p = checkAndGetRow(it, name, row, value);
	if (!p) return TW_INVALID_PARAM;
	value->elevation = p->val.location.elevation;
	value->latitude = p->val.location.latitude;
	value->longitude = p->val.location.longitude;
	return TW_OK;
}

int twInfoTable_GetBlob(twInfoTable * it, const char * name, int32_t row, char ** value, int32_t * length) {
	twPrimitive * p = checkAndGetRow(it, name, row, value);
	if (!p) return TW_INVALID_PARAM;
	*value = p->val.bytes.data;
	* length = p->val.bytes.len;
	return TW_OK;
}

int twInfoTable_GetDatetime(twInfoTable * it, const char * name,  int32_t row, DATETIME * value) {
	twPrimitive * p = checkAndGetRow(it, name, row, value);
	if (!p) return TW_INVALID_PARAM;
	*value = p->val.datetime;
	return TW_OK;
}
int twInfoTable_GetBoolean(twInfoTable * it, const char * name, int32_t row, char * value) {
	twPrimitive * p = checkAndGetRow(it, name, row, value);
	if (!p) return TW_INVALID_PARAM;
	*value = p->val.boolean;
	return TW_OK;
}


/**********************************/
/*     JSON Conversion Utils      */
/**********************************/
int twDataShapeEntry_ToJsonForEach(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	twDataShapeAspect *asp = (twDataShapeAspect *) data;
	cJSON * aspects = (cJSON *)arg;
	twPrimitive_ToJson(asp->name, asp->value, aspects);
	return TW_FOREACH_CONTINUE;
}

cJSON * twDataShapeEntry_ToJson(twDataShapeEntry * de, char * name, cJSON * parent) {
	cJSON * j = NULL;
	if (!de || !name) {
		TW_LOG(TW_ERROR,"twDataShapeEntryToJson: NULL Pointer passed in");
		return NULL;
	}
	/* Do we want to add to an existing object? */
	if (parent) j = parent;
	else j = cJSON_CreateObject();
	if (!j) {
		TW_LOG(TW_ERROR,"twDataShapeEntryToJson: Error allocating cJSON object");
		return NULL;
	}
	cJSON_AddStringToObject(j, "name", de->name);
	cJSON_AddStringToObject(j, "description", de->description ? de->description : "");
	cJSON_AddStringToObject(j, "baseType", baseTypeToString(de->type));
	if (de->aspects) {
		cJSON * aspects = cJSON_CreateObject();

		if (!aspects) { 
			TW_LOG(TW_ERROR,"twDataShapeEntryToJson: Error allocating cJSON object for aspects"); 
			if (!parent) cJSON_Delete(j);
			return NULL;
		}
		twList_Foreach(de->aspects,twDataShapeEntry_ToJsonForEach,(void*)aspects);
		cJSON_AddItemToObject(j, "aspects", aspects);
	}
	return j;
}

int twDataShape_ToJsonForEach(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	cJSON *field = NULL;
	cJSON * fieldDefinitions = (cJSON *) arg;
	twDataShapeEntry *de = (twDataShapeEntry *) data;
	field = twDataShapeEntry_ToJson(de, de->name, NULL);
	cJSON_AddItemToObject(fieldDefinitions, de->name, field);
	return TW_FOREACH_CONTINUE;
}

cJSON * twDataShape_ToJson(twDataShape * ds, cJSON * parent) {
	cJSON * j = parent;
	cJSON * fieldDefinitions = NULL;
	if (!ds || !ds->entries) {
		TW_LOG(TW_ERROR,"twDataShapeToJson: NULL Pointer passed in");
		return NULL;
	}
	if (!j) j = cJSON_CreateObject();
	fieldDefinitions = cJSON_CreateObject();
	if (!j || !fieldDefinitions) {
		TW_LOG(TW_ERROR,"twDataShapeToJson: Error allocating cJSON object");
		if (fieldDefinitions) cJSON_Delete(fieldDefinitions);
		if (j && !parent) cJSON_Delete(j);
		return NULL;
	}
	twList_Foreach(ds->entries,twDataShape_ToJsonForEach,fieldDefinitions);
	cJSON_AddItemToObject(j, "fieldDefinitions", fieldDefinitions);
	if (ds->name) cJSON_AddStringToObject(j, "name", ds->name);
	return j;
}

typedef struct twInfoTableRow_ToJsonParams {
	int index;
	cJSON * json;
	twDataShape * ds;
	char success;
} twInfoTableRow_ToJsonParams;

int twInfoTableRow_ToJsonForEach(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	twInfoTableRow_ToJsonParams* params = (twInfoTableRow_ToJsonParams*)arg;
	ListEntry *dse = twList_GetByIndex(params->ds->entries, params->index);
	if (!dse || !dse->value) {
		TW_LOG(TW_ERROR, "twInfoTableRow_ToJson: Invalid data shape entry found");
		cJSON_Delete(params->json);
		params->success = FALSE;
		return TW_FOREACH_EXIT;
	}
	if (!twPrimitive_ToJson(((twDataShapeEntry *) dse->value)->name, (twPrimitive *) data, params->json)) {
		TW_LOG(TW_ERROR, "twInfoTableRow_ToJson: Error adding primitive to JSON object");
		cJSON_Delete(params->json);
		params->success = FALSE;
		return TW_FOREACH_EXIT;
	}
	params->index++;
	return TW_FOREACH_CONTINUE;
}

cJSON * twInfoTableRow_ToJson(twInfoTableRow * row, twDataShape * ds) {
	cJSON * j = NULL;
	twInfoTableRow_ToJsonParams* forEachParams = NULL;

	if (!ds || !ds->entries || !row || !row->fieldEntries) {
		TW_LOG(TW_ERROR,"twInfoTableRow_ToJson: NULL Pointer passed in");
		return NULL;
	}
	if (row->numFields != ds->numEntries) {
		TW_LOG(TW_ERROR,"twInfoTableRow_ToJson: Number of data shape fields %d not equal to number of row entries %d",
			ds->numEntries, row->numFields);
		return NULL;
	}
	j = cJSON_CreateObject();
	if (!j) {
		TW_LOG(TW_ERROR,"twInfoTableRow_ToJson: Error allocating cJSON object");
		return NULL;
	}

	forEachParams = (twInfoTableRow_ToJsonParams*)TW_MALLOC(sizeof(twInfoTableRow_ToJsonParams));
	forEachParams->ds = ds;
	forEachParams->index = 0;
	forEachParams->json = j;
	forEachParams->success = TRUE;
	twList_Foreach(row->fieldEntries,twInfoTableRow_ToJsonForEach,forEachParams);
	if(forEachParams->success) {
		TW_FREE(forEachParams);
		return j;
	} else {
		TW_FREE(forEachParams);
		return NULL;
	}
}
typedef struct wInfoTable_CreateFromJsonForEachParams {
	twInfoTableRow *row;
	cJSON *r;
	char success;
} wInfoTable_CreateFromJsonForEachParams;


int twInfoTable_CreateFromJsonForEach(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	twDataShapeEntry *dse = (twDataShapeEntry *) data;
	wInfoTable_CreateFromJsonForEachParams* params = (wInfoTable_CreateFromJsonForEachParams*)arg;

	twPrimitive *p = twPrimitive_CreateFromJson(params->r, dse->name, dse->type);
	if (!p) {
		if (params->row) twInfoTableRow_Delete(params->row);
		TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating infotable row");
		return TW_FOREACH_EXIT;
	}
	if (!params->row)
		params->row = twInfoTableRow_Create(p);
	else
		twInfoTableRow_AddEntry(params->row, p);

	return TW_FOREACH_CONTINUE;

}

twInfoTable *twInfoTable_CreateFromJson(cJSON *json, char *singleEntryName) {
	cJSON *dsJSON = NULL;
	cJSON *fd = NULL;
	cJSON *rowJSON = NULL;
	twDataShape *ds = NULL;
	twInfoTable *it = NULL;
	twInfoTableRow *row = NULL;
	wInfoTable_CreateFromJsonForEachParams* params = NULL;

	int i = 0;

	if (!json) {
		TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: NULL JSON object passed in");
		return NULL;
	}
	dsJSON = cJSON_GetObjectItem(json, "dataShape");
	fd = dsJSON ? cJSON_GetObjectItem(dsJSON, "fieldDefinitions") : NULL;
	rowJSON = cJSON_GetObjectItem(json, "rows");
	if (!dsJSON || !fd || !rowJSON || rowJSON->type != cJSON_Array) {
		/* Wrap this as a JSON baseType */
		/* set the datashape entry name to _content_ */
		twDataShapeEntry *dse = NULL;
		twDataShape *ds = NULL;
		twPrimitive *p = NULL;
		twInfoTableRow *row = NULL;
		TW_LOG(TW_DEBUG, "twInfoTable_CreateFromJson: Non-infotable format passed in.  Using single entry '%s'",
			   singleEntryName ? singleEntryName : "_content_");
		if (!singleEntryName) dse = twDataShapeEntry_Create("_content_", NULL, TW_JSON);
		else dse = twDataShapeEntry_Create(singleEntryName, NULL, TW_JSON);
		if (!dse) {
			TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating dataShapeEntry");
			return NULL;
		}
		ds = twDataShape_Create(dse);
		if (!ds) {
			TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating dataShape");
			twDataShapeEntry_Delete(dse);
			return NULL;
		}
		it = twInfoTable_Create(ds);
		if (!it) {
			TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating dataShape");
			twDataShape_Delete(ds);
			return NULL;
		}
		p = twPrimitive_CreateFromJson(json, NULL, TW_JSON);
		row = twInfoTableRow_Create(p);
		if (!row) {
			TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating dataShape");
			twInfoTable_Delete(it);
			if (p) twPrimitive_Delete(p);
			return NULL;
		}
		twInfoTable_AddRow(it, row);
		return it;
	}

	/* Handle the Datashape first */
	{
		cJSON *tmp = cJSON_GetObjectItem(dsJSON, "name");
		char *dsName = tmp ? tmp->valuestring : NULL;
		/* Get each entry from the field description */
		tmp = fd->child;
		while (tmp) {
			cJSON *elem = NULL;
			char *entryName = NULL;
			char *description = NULL;
			enum BaseType type = TW_NOTHING;
			twDataShapeEntry *dse = NULL;
			cJSON *aspects = NULL;

			elem = cJSON_GetObjectItem(tmp, "name");
			entryName = elem ? elem->valuestring : NULL;
			elem = cJSON_GetObjectItem(tmp, "description");
			description = elem ? elem->valuestring : NULL;
			elem = cJSON_GetObjectItem(tmp, "baseType");
			type = baseTypeFromString(elem ? elem->valuestring : NULL);
			/* Create the entry */
			TW_LOG(TW_TRACE, "twInfoTable_CreateFromJson: Processing DataShape Field: %s", entryName);
			dse = twDataShapeEntry_Create(entryName, description, type);
			if (!dse) {
				TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating data shape entry");
				return NULL;
			}
			/* Add the aspects */
			aspects = cJSON_GetObjectItem(tmp, "aspects");
			if (aspects) {
				cJSON *aspect = aspects->child;
				while (aspect && aspect->string) {
					twPrimitive *p = NULL;
					switch (aspect->type) {
						case cJSON_True:
						case cJSON_False:
							p = twPrimitive_CreateFromBoolean(aspect->type);
							break;
						case cJSON_Number:
							if (aspect->valueint) p = twPrimitive_CreateFromInteger(aspect->valueint);
							else p = twPrimitive_CreateFromNumber(aspect->valuedouble);
							break;
						case cJSON_String:
							if (aspect->valuestring) p = twPrimitive_CreateFromString(aspect->valuestring, TRUE);
							break;
						default:
							p = twPrimitive_Create();
					}
					if (!p) {
						TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating data shape entry");
						twDataShapeEntry_Delete(dse);
						if (ds) twDataShape_Delete(ds);
						return NULL;
					} else twDataShapeEntry_AddAspect(dse, aspect->string, p);
					aspect = aspect->next;
				}
			}
			/* Add this entry to the datashape, creating the datshape if needed */
			if (!ds) ds = twDataShape_Create(dse);
			else twDataShape_AddEntry(ds, dse);
			/* Go to the next entry */
			tmp = tmp->next;
		}
		TW_LOG(TW_TRACE, "twInfoTable_CreateFromJson: Datashape Complete");
		/* Sanity check - do we have a datashape? */
		if (!ds) {
			TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating data shape");
			return NULL;
		}
		/* Set the datshape name if there is one */
		if (dsName) twDataShape_SetName(ds, dsName);
	}
	/* Now we can create the infotable */
	it = twInfoTable_Create(ds);
	if (!it) {
		TW_LOG(TW_ERROR, "twInfoTable_CreateFromJson: Error creating infotable");
		twDataShape_Delete(ds);
		return NULL;
	}
	/* Start processing the rows */
	TW_LOG(TW_TRACE, "twInfoTable_CreateFromJson: Beginning row processing");
	for (i = 0; i < cJSON_GetArraySize(rowJSON); i++) {
		/* Need to do this in  precise order the fields appear in the datashape */
		cJSON *r = cJSON_GetArrayItem(rowJSON, i);

		TW_LOG(TW_TRACE, "twInfoTable_CreateFromJson: Processing Row %d", i);
		params = (wInfoTable_CreateFromJsonForEachParams*)TW_MALLOC(sizeof(wInfoTable_CreateFromJsonForEachParams));
		params->r = r;
		params->row = row;
		params->success = TRUE;
		twList_Foreach(ds->entries, twInfoTable_CreateFromJsonForEach, params);
		if(!params->success){
			TW_FREE(params);
			twInfoTable_Delete(it);
			return NULL;
		}
		row = params->row;
		TW_FREE(params);

		twInfoTable_AddRow(it, row);
		row = NULL;
	}
	TW_LOG(TW_TRACE, "twInfoTable_CreateFromJson: InfoTable Complete");
	return it;
}

typedef struct twInfoTable_ToJsonForEachParams {
	cJSON * rows;
	twDataShape * ds;
} twInfoTable_ToJsonForEachParams;

int twInfoTable_ToJsonForEach(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	cJSON *row = NULL;
	twInfoTable_ToJsonForEachParams* params = (twInfoTable_ToJsonForEachParams*)arg;
	row = twInfoTableRow_ToJson((twInfoTableRow *) data, params->ds);
	if (row) cJSON_AddItemToArray(params->rows, row);
	return TW_FOREACH_CONTINUE;
}

cJSON * twInfoTable_ToJson(twInfoTable * it) {
	cJSON * j = NULL;
	cJSON * rows = NULL;
	cJSON * ds = NULL;
	twInfoTable_ToJsonForEachParams* params = NULL;

	if (!it || !it->ds || !it->rows) {
		TW_LOG(TW_ERROR,"twInfoTable_ToJson: NULL of bad infotable");
		return NULL;
	}
	j = cJSON_CreateObject();
	rows = cJSON_CreateArray();
	if (!j || !rows) {
		TW_LOG(TW_ERROR,"twInfoTable_ToJson: Error allocating cJSON object");
		if (rows) cJSON_Delete(rows);
		if (j) cJSON_Delete(j);
		return NULL;
	}
	cJSON_AddItemToObject(j, "rows", rows);
	/* Convert the data shape */
	ds = cJSON_CreateObject();
	if (!ds || !twDataShape_ToJson(it->ds, ds)) {
		TW_LOG(TW_ERROR,"twInfoTable_ToJson: Error converting datashape to JSON");
		if (ds) cJSON_Delete(ds);
		cJSON_Delete(j);
		return NULL;
	}
	cJSON_AddItemToObject(j, "datashape", ds);

	params = (twInfoTable_ToJsonForEachParams*)TW_MALLOC(sizeof(twInfoTable_ToJsonForEachParams));
	params->ds=it->ds;
	params->rows=rows;
	twList_Foreach(it->rows,twInfoTable_ToJsonForEach,params);
	TW_FREE(params);

	return j;
}
