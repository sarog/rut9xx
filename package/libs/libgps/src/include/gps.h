#ifndef GPSD_QUERY_H
#define GPSD_QUERY_H

#include <stddef.h>

#define GPS_IPC_SOCKET_PATH "/var/gpsd.sock"
#define GPS_QUERY_SEPARATOR "\n"
#define GPS_QUERY_BUFF_SIZE 512

typedef enum {
	GPS_REQUEST_UNKNOWN,
	GPS_REQUEST_INPUT
} gps_request_id_type;

typedef enum {
	GPS_QUERY_UNKNOWN,
	GPS_QUERY_LATITUDE,
	GPS_QUERY_LONGITUDE,
	GPS_QUERY_TIMESTAMP,
	GPS_QUERY_ALTITUDE,
	GPS_QUERY_SPEED,
	GPS_QUERY_SATELLITE_COUNT,
	GPS_QUERY_ANGLE,
	GPS_QUERY_FIX_STATUS,
	GPS_QUERY_ACCURACY,
	GPS_QUERY_DATE_TIME
} gps_query_id_type;

extern const char *gps_query_strings[];
extern const size_t gps_query_strings_size;
extern const char *gps_request_strings[];
extern const size_t gps_request_strings_size;

int gps_query(gps_query_id_type *queries, size_t query_count, char *buff, size_t buff_size);
void change_input_status(char *input);

// Depreciated functions to support legacy applications
void gps_new_wan(char *input) __attribute__((deprecated));
void change_registered_status(char *input) __attribute__((deprecated));

#endif
