#define MAX_SIM_ID	 2
#define SIM_SECTION_SIZE 0x8

#define MTD_CONFIG_RO "/dev/mtd1"
#define MTD_CONFIG_RW "/dev/mtd1"

#define MAC_OFFSET    0x00
#define MAC_LENGTH    6
#define NAME_OFFSET   0x10
#define NAME_LENGTH   12
#define WPS_OFFSET    0x20
#define WPS_LENGTH    8
#define SERIAL_OFFSET 0x30
#define SERIAL_LENGTH 10
#define BATCH_OFFSET  0x40
#define BATCH_LENGTH  4
#define HWVER_OFFSET  0x50
#define HWVER_LENGTH  4
#define SIMPIN_OFFSET 0x70
#define WIFI_OFFSET   0x90
#define WIFI_LENGTH   16
#define PASSW_OFFSET  0xa0
#define PASSW_LENGTH  106
#define BLVER_OFFSET  0x1FFF6
#define BLVER_LENGTH  10