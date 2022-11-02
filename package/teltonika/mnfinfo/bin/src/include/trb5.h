#define MAX_SIM_ID	 1
#define SIM_SECTION_SIZE 0x10

#define MTD_CONFIG_RO "/dev/mtd15"
#define MTD_CONFIG_RW "/dev/mtd15"

#define MAC_OFFSET    0x00
#define MAC_LENGTH    6
#define NAME_OFFSET   0x10
#define NAME_LENGTH   12
#define SERIAL_OFFSET 0x30
#define SERIAL_LENGTH 10
#define BATCH_OFFSET  0x40
#define BATCH_LENGTH  4
#define HWVER_OFFSET  0x50
#define HWVER_LENGTH  4
#define SIMPIN_OFFSET 0x70
#define PASSW_OFFSET  0xa0
#define PASSW_LENGTH  106
