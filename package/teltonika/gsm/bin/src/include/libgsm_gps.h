#pragma once

typedef struct {
        double longitude;
        double latitude;
        double altitude;
        double angle;
        double speed;
        double accuracy;
        uint32_t satellites;
        uint32_t fix_status;
        uint64_t timestamp;
} gps_table;

int ubus_gps_query(gps_table *gps);

int get_gps_coordinates(char *output);
