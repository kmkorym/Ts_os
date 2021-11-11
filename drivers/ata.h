#include "../kernel/common.h"

#define ATA_DATA       0x00
#define ATA_ERR        0x01
#define ATA_FEATURES   0x01
#define ATA_SECCOUNT   0x02
#define ATA_LBA0       0x03
#define ATA_LBA1       0x04
#define ATA_LBA2       0x05
#define ATA_HDDEVSEL   0x06
#define ATA_COMMAND    0x07
#define ATA_STATUS     0x07

#define ATA_ALT_STATUS 0x0
#define ATA_DEV_CTRL   0x0

#define ATA_PR_MST_IO_BASE   0x1F0 
#define ATA_PR_MST_CTRL_BASE 0x3F6

#define ATA_SEL_MASTER 0xA0
#define ATA_SEL_SLAVE  0xB0


#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_ERR 0x1
#define ATA_STATUS_DF 0x20
#define ATA_STATUS_DRQ 0x8


struct IDE_Device {
   unsigned char  is_ata;    // 0 (Empty) or 1 (This Drive really exists).
   unsigned char  channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
   unsigned char  drive;       // 0 (Master Drive) or 1 (Slave Drive).
   unsigned short type;        // 0: ATA, 1:ATAPI.
   unsigned short signature;   // Drive Signature
   uint8_t    dma_support;
   uint8_t    lba_support;
   uint8_t    lba48_support;
   uint16_t   cylinders;        // Size in Sectors.
   uint16_t   heads;
   uint16_t   sectors;        // Size in Sectors.
   uint16_t   major;        // Size in Sectors.
   uint32_t   lba_num;
} ;
typedef struct IDE_Device IDE_Device;



struct Disk_IO_Command{
   IDE_Device * device;
   uint32_t lba_start;
   uint32_t sec_num;
   uint8_t *buf;
   uint32_t buf_size;
};
typedef struct Disk_IO_Command Disk_IO_Command;





IDE_Device * get_ide_device(uint32_t bus,uint32_t dev_nb);
void parse_identify_command(struct IDE_Device *ide_device,uint8_t *read_buf);
void summary_ide_device(struct IDE_Device *device);
uint32_t ata_read_sector(Disk_IO_Command *command);
uint32_t ata_write_sector(Disk_IO_Command *command);
void ata_detect(uint32_t bus,uint32_t dev_nb);
void ata_reset_bus(uint32_t bus);
void ata_wait(uint32_t bus,uint32_t dev_nb);
void ata_detect(uint32_t bus,uint32_t dev_nb);
void ata_poll(uint32_t bus,uint32_t dev_nb);
uint8_t ata_poll_bsy(struct IDE_Device *device);
uint8_t ata_poll_bsy_drq(struct IDE_Device *device,int set_drq);
void ata_soft_reset(IDE_Device *device);