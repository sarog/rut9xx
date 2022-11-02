/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Logging facility
 */

#include "twLogger.h"
#include "twOSPort.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

twLogger * logger_singleton = NULL;

char * levelString(enum LogLevel level) {
	switch(level) {
	case TW_TRACE: 
		return "TRACE";
	case TW_DEBUG: 
		return "DEBUG";
	case TW_INFO: 
		return "INFO";
	case TW_WARN: 
		return "WARN";
	case TW_ERROR: 
		return "ERROR";
	case TW_FORCE: 
		return "FORCE";
	case TW_AUDIT: 
		return "AUDIT";
	default: 
		return "UNKNOWN";
	}
}

twLogger * twLogger_Instance() {
	twLogger * temp = NULL;
	/* Check to see if it was created already */
	if (logger_singleton) return logger_singleton;
	temp = (twLogger *)TW_CALLOC(sizeof(twLogger),1);
	temp->level = TW_WARN;
	temp->f = LOGGING_FUNCTION;
	temp->isVerbose = 0;
	temp->buffer = (char *)TW_MALLOC(TW_LOGGER_BUF_SIZE);
	temp->mtx = twMutex_Create();
	logger_singleton = temp;
	return logger_singleton;
}

int twLogger_Delete() {
	if (logger_singleton) {
		twLogger * temp = logger_singleton;
		logger_singleton = NULL;
		TW_FREE(temp->buffer);
		twMutex_Delete(temp->mtx);
		TW_FREE(temp);
	}
	return TW_OK;
}

int twLogger_SetLevel(enum LogLevel level) {
	twLogger * l = twLogger_Instance();
	if (l) {
		l->level = level;
		return TW_OK;
	} else return TW_NULL_OR_INVALID_LOGGER_SINGLETON;
}

int twLogger_SetFunction(log_function f) {
	twLogger * l = twLogger_Instance();
	if (l) {
		l->f = f;
		return TW_OK;
	} else return TW_NULL_OR_INVALID_LOGGER_SINGLETON;
}

int twLogger_SetIsVerbose(char val) {
	twLogger * l = twLogger_Instance();
	if (l) {
		l->isVerbose = val;
		return TW_OK;
	} else return TW_NULL_OR_INVALID_LOGGER_SINGLETON;
}

void twLog(enum LogLevel level, const char * format, ... ) {
	char timeStr[80];
	twLogger * l = twLogger_Instance();
	va_list va;

	if (!l || level < l->level) return;
	/* prepare the message */
	if (!l->buffer || !l->mtx) return;
	twMutex_Lock(l->mtx);
    va_start(va, format);
    vsnprintf(l->buffer, TW_LOGGER_BUF_SIZE - 1, format, va);
	va_end(va);
	/* get the timestamp */
	twGetSystemTimeString(timeStr, "%Y-%m-%d %H:%M:%S", 80, 1, 1);
	l->f(level, timeStr, l->buffer);
	twMutex_Unlock(l->mtx);
}

/**************************************/
/*        For Verbose debugging       */
/**************************************/
#include "twMessages.h"

char * twCodeToString(enum msgCodeEnum m) {
	switch(m) {
	case	TWX_GET:
		return "GET";
	case	TWX_PUT:
		return "PUT";
	case	TWX_POST:
		return "POST";
	case	TWX_DEL:
		return "DEL";
	case	TWX_BIND:
		return "BIND";
	case	TWX_UNBIND:
		return "UNBIND";
	case	TWX_AUTH:
		return "AUTH";
	case	TWX_KEEP_ALIVE:
		return "KEEP_ALIVE";
	case	TWX_SUCCESS:
		return "SUCCESS";
	case	TWX_BAD_REQUEST:
		return "BAD_REQUEST";
	case	TWX_UNAUTHORIZED:
		return "UNAUTHORIZED";
	case	TWX_BAD_OPTION:
		return "BAD_OPTION";
	case	TWX_FORBIDDEN:
		return "FORBIDDEN";
	case	TWX_NOT_FOUND:
		return "NOT_FOUND";
	case	TWX_METHOD_NOT_ALLOWED:
		return "METHOD_NOT_ALLOWED";
	case	TWX_NOT_ACCEPTABLE:
		return "NOT_ACCEPTABLE";
	case	TWX_PRECONDITION_FAILED:
		return "PRECONDITION_FAILED";
	case	TWX_ENTITY_TOO_LARGE:
		return "ENTITY_TOO_LARGE";
	case	TWX_UNSUPPORTED_CONTENT_FORMAT:
		return "UNSUPPORTED_CONTENT_FORMAT";
	case	TWX_INTERNAL_SERVER_ERROR:
		return "INTERNAL_SERVER_ERROR";
	case	TWX_NOT_IMPLEMENTED:
		return "NOT_IMPLEMENTED";
	case	TWX_BAD_GATEWAY:
		return "BAD_GATEWAY";
	case	TWX_SERVICE_UNAVAILABLE:
		return "SERVICE_UNAVAILABLE";
	case	TWX_GATEWAY_TIMEOUT:
		return "GATEWAY_TIMEOUT";
	default:
		return "UNKNOWN";
	};
}

char * twEntityToString(enum entityTypeEnum m) {
	switch(m) {
	case	TW_THING:
		return "THING";
	case	TW_RESOURCE:
		return "RESOURCE";
	case	TW_SUBSYSTEM:
		return "SUBSYSTEM";
	default:
		return "UNKNOWN";
	};
}

char * twCharacteristicToString(enum characteristicEnum m) {
	switch(m) {
	case	TW_PROPERTIES:
		return "PROPERTIES";
	case	TW_SERVICES:
		return "SERVICES";
	case	TW_EVENTS:
		return "EVENTS";
	default:
		return "UNKNOWN";
	};
}

void logAuthBody(twAuthBody * b, char * buf, size_t maxLength) {
	if (b && maxLength > 1)
		snprintf(buf, maxLength, "Claim Count: 1\nName: %s\n", b->name);
}

void logBindBody( twBindBody * b, char * buf, size_t maxLength) {
	size_t slen;
	if (b && maxLength > 1) {
		ListEntry * le = NULL;
		snprintf(buf, maxLength, "Count: %d\n", b->count);
		slen = strnlen(buf, maxLength);
		maxLength -= slen;
		if (maxLength <= 1) return;
		buf += slen;
		if (b->gatewayName && b->gatewayType) {
			snprintf(buf, maxLength, "GatewayName: %s\nGatewayType:%s\n", b->gatewayName, b->gatewayType);
			slen = strnlen(buf, maxLength);
			maxLength -= slen;
			if (maxLength <= 1) return;
			buf = buf + slen;
		}
		le = twList_Next(b->names, NULL);
		while (le && le->value && maxLength > 0) {
			snprintf(buf, maxLength, "Name: %s\n", (char *)le->value);
			slen = strnlen(buf, maxLength);
			maxLength -= slen;
			if (maxLength <= 1) return;
			buf = buf + slen;
			le = twList_Next(b->names, le);
		}
	}
}

void logInfoTable(twInfoTable * it, char * buf, size_t maxLength);
void logPrimitive(twPrimitive * p, char * buf, size_t maxLength) {
	char * tmp = NULL;
	size_t slen;
	if (!p || !buf) return;
	snprintf(buf, maxLength, "   BaseType: %9s\t", baseTypeToString(p->type));
	slen = strnlen(buf, maxLength);
	maxLength -= slen;
	if (maxLength <= 1) return;
	buf = buf + slen;
	tmp = (char *)TW_CALLOC(maxLength, 1);
	if (!tmp) {
		snprintf(buf, maxLength, "ERROR ALLOCATING BUFFER FOR LOGGING ");
		return;
	}
	switch (p->typeFamily) {
	case 	TW_NOTHING:
		break;
	case 	TW_BLOB:
		{
		snprintf(buf, maxLength, "Value: IMAGE/BLOB\tLength: %d\n", p->val.bytes.len);
		break;
		}
	case	TW_STRING:
		{
		snprintf(buf, maxLength, "Value: %s\tLength: %d\n", p->val.bytes.data, p->val.bytes.len);
		break;
		}
	case	TW_VARIANT:
		{
		snprintf(buf, maxLength, "Value: ");
		slen = strnlen(buf, maxLength);
		maxLength -= slen;
		if (maxLength <= 1) break;
		buf = buf + slen;
		logPrimitive(p->val.variant, buf, maxLength);
		break;
		}
	case	TW_NUMBER:
		{
		snprintf(buf, maxLength, "Value: %f\n", p->val.number);
		break;
		}
	case	TW_INTEGER:
		{
		snprintf(buf, maxLength, "Value: %d\n", p->val.integer);
		break;
		}
	case	TW_BOOLEAN:
		{
		snprintf(buf, maxLength, "Value: %s\n", p->val.boolean ? "TRUE" : "FALSE");
		break;
		}
	case	TW_DATETIME:
		{
		snprintf(buf, maxLength, "Value: %llu\n", (unsigned long long)(p->val.datetime));
		break;
		}
	case	TW_LOCATION:
		{
		snprintf(buf, maxLength, "Value: Long: %f, Lat: %f, Elev: %f\n",
			p->val.location.longitude,p->val.location.latitude,p->val.location.elevation);
		break;
		}
	case	TW_INFOTABLE:
		{
		snprintf(buf, maxLength, "Value:\n");
		slen = strnlen(buf, maxLength);
		maxLength -= slen;
		if (maxLength <= 1) break;
		buf = buf + slen;
		logInfoTable(p->val.infotable, buf, maxLength);
		break;
		}
	default:
		snprintf(buf, maxLength, "INVALID BASE TYPE\n");
		break;
	}
	TW_FREE(tmp);
}

void logInfoTable(twInfoTable * it, char * buf, size_t maxLength) {
	size_t slen;
	if (it && it->ds && it->ds->entries && maxLength > 1) {
		ListEntry * le = NULL;
		snprintf(buf, maxLength, "DataShape:\n");
		slen = strnlen(buf, maxLength);
		maxLength -= slen;
		if (maxLength <= 1) return;
		buf = buf + slen;
		le = twList_Next(it->ds ->entries, NULL);
		while (le && le->value && maxLength > 0) {
			twDataShapeEntry * dse = (twDataShapeEntry *)le->value;
			snprintf(buf, maxLength, "   Name: %16s\tBaseType: %s\n",
				dse->name, baseTypeToString(dse->type));
			slen = strnlen(buf, maxLength);
			maxLength -= slen;
			if (maxLength <= 1) return;
			buf += slen;
			le = twList_Next(it->ds ->entries, le);
		}
		if (it->rows && maxLength > 0) {
			int i = 1;
			le = twList_Next(it->rows, NULL);
			while (le && le->value && maxLength > 0) {
				twInfoTableRow * row = (twInfoTableRow *)le->value;
				snprintf(buf, maxLength, "Row %d:\n", i++);
				slen = strnlen(buf, maxLength);
				maxLength -= slen;
				if (maxLength <= 1) return;
				buf = buf + slen;
				if (row->fieldEntries) {
					ListEntry * field = NULL;
					field = twList_Next(row->fieldEntries, NULL);
					while (field && field->value && maxLength > 0) {
						twPrimitive * p = (twPrimitive *)field->value;
						logPrimitive(p, buf, maxLength);
						slen = strnlen(buf, maxLength);
						maxLength -= slen;
						if (maxLength <= 1) return;
						buf += slen;
						field = twList_Next(row->fieldEntries, field);
					}
				}
				le = twList_Next(it->rows, le);
			}
		}
	}
}

void logRequestBody( twRequestBody * b, char * buf, size_t maxLength) {
	size_t slen;
	if (b && maxLength > 1) {
		snprintf(
			buf,
			maxLength,
			"EntityType: %s\nEntityName: %s\nCharacteristicType: %s\nCharacteristicName: %s%s",
			twEntityToString(b->entityType),
			b->entityName,
			twCharacteristicToString(b->characteristicType),
			b->characteristicName,
			b->numHeaders ? "Headers:\n":"\n"
		);
		slen = strnlen(buf, maxLength);
		maxLength -= slen;
		if (maxLength <= 1) return;
		buf += slen;
		if (b->numHeaders && b->headers) {
			ListEntry * le = NULL;
			le = twList_Next(b->headers, NULL);
			while (le && le->value && maxLength > 0) {
				twHeader * h = (twHeader *)le->value;
				snprintf(buf, maxLength, "   Name: %s\n   Value: %s\n",
					h->name ? h->name : "NULL", h->value? h->value : "NULL");
				slen = strnlen(buf, maxLength);
				maxLength -= slen;
				if (maxLength <= 1) return;
				buf += slen;
				le = twList_Next(b->headers, le);
			}
		}
		snprintf(buf, maxLength, "Parameter Type: %s\n", b->params ? "INFOTABLE" : "NONE");
		slen = strnlen(buf, maxLength);
		maxLength -= maxLength - slen;
		if (maxLength <= 1) return;
		buf += slen;
		if (b->params && maxLength > 1) {
			logInfoTable(b->params, buf, maxLength);
		}
	}
}

void logResponseBody( twResponseBody * b, char * buf, size_t maxLength) {
	size_t slen;
	if (b && maxLength > 1) {
		if (b->reason) {
			snprintf(buf, maxLength, "Reason: %s\n", b->reason);
			slen = strnlen(buf, maxLength);
			maxLength -= slen;
			if (maxLength <= 1) return;
			buf = buf + slen;
		}
		snprintf(buf, maxLength, "Result Type: %s\n", baseTypeToString(b->contentType));
		slen = strnlen(buf, maxLength);
		maxLength = maxLength - slen;
		if (maxLength <= 1) return;
		buf += slen;
		if (b->contentType == TW_INFOTABLE && b->content && maxLength > 0) {
			logInfoTable(b->content, buf, maxLength);
		}
	}
}

void twLogMessage (void * msg, char * preamble) {
	char * buf = NULL;
	twMessage * m = (twMessage *)msg;
	size_t bytesUsed = 0;
	twLogger * l = twLogger_Instance();
	if (!m|| !l || !l->isVerbose || TW_TRACE < l->level) return;
	if (!(buf = (char *) TW_CALLOC(TW_LOGGER_BUF_SIZE, 1))) {
		return;
	}

	snprintf(
		buf,
		TW_LOGGER_BUF_SIZE,
		"%s\nMessage Details:\nVersion: %d\nMethod/Code: 0x%x (%s)\nRequestID: %d\nEndpointID:%d\nSessionID: %d\nMultipart: %d\n",
		preamble ? preamble : "",
		m->version,
		m->code,
		twCodeToString(m->code),
		m->requestId,
		m->endpointId,
		m->sessionId,
		m->multipartMarker
	);
	bytesUsed = strnlen(buf, TW_LOGGER_BUF_SIZE);
	switch (m->type) {
	case TW_AUTH:
		logAuthBody((twAuthBody *)m->body, buf + bytesUsed, TW_LOGGER_BUF_SIZE - bytesUsed);
		break;
	case TW_BIND:
		logBindBody((twBindBody *)m->body, buf + bytesUsed, TW_LOGGER_BUF_SIZE - bytesUsed);
		break;
	case TW_REQUEST:
		logRequestBody((twRequestBody *)m->body, buf + bytesUsed, TW_LOGGER_BUF_SIZE - bytesUsed);
		break;
	case TW_RESPONSE:
		logResponseBody((twResponseBody *)m->body, buf + bytesUsed, TW_LOGGER_BUF_SIZE - bytesUsed);
		break;
	default:
		break;
	}
	TW_LOG(TW_TRACE, "%s", buf);
	TW_FREE(buf);
}

void twLogHexString(const char * msg, char * preamble, size_t length) {
	char * tmp = NULL;
	char hex[4];
	int i;
	size_t size;
	size_t newLength = length;
	twLogger * l = twLogger_Instance();
	if (!msg || !preamble || !l || !l->isVerbose || TW_TRACE < l->level) return;
	/* prepare the message */
	size = length * 3 + 1;
	if (size > TW_LOGGER_BUF_SIZE) {
		size = TW_LOGGER_BUF_SIZE;
		newLength = size / 3 - 1;
	}
	tmp = (char *)TW_CALLOC(size, 1);
	if (tmp) {
		for (i = 0; i < newLength; i++) {
			sprintf(hex, "%02X ", (unsigned char)msg[i]);
			strcat(tmp, hex);
		}
		TW_LOG(TW_TRACE, "%s%s", preamble, tmp);
		TW_FREE(tmp);
	} else {
		TW_LOG(TW_ERROR, "twLogHexString: Error allocating storage");
	}
}
