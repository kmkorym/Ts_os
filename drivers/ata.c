#include "ata.h"
#include "../lib/print.h"

#define ATA_IDENT_DEV_INFO     0 
#define ATA_IDENT_CYLINDERS    1
#define ATA_IDENT_HEADS        3
#define ATA_IDENT_SECTORS      6
#define ATA_IDENT_SERIAL       10
#define ATA_IDENT_CAPABILITIES 49
#define ATA_IDENT_MAX_LBA      60
#define ATA_IDENT_MAJOR        80
#define ATA_IDENT_COMMANDSETS  82
#define ATA_IDENT_MAX_LBA48    100

#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30



/*
    problem : ata read / write not working ....
     // https://stackoverflow.com/questions/68081888/strange-data-when-read-disk-sector-with-ata-ide-pio


*/


//#define ATA_CTRL_MSK_INTR

struct IDE_Device ide_device;

uint8_t buffer[1024];



IDE_Device * get_ide_device(uint32_t bus,uint32_t dev_nb){
   return &ide_device;
} 


//  must take endianess into account
//  read order --> ABCD EFGH
//  memory layout CD AB GH EF = EFGHABCD

void parse_identify_command(struct IDE_Device *ide_device,uint8_t *_read_buf){

    uint16_t * read_buf = (uint16_t *) _read_buf;

    //print_hex(read_buf[ATA_IDENT_DEV_INFO]);
    //printl("");


    if( ! (read_buf[ATA_IDENT_DEV_INFO]>>15) & 1){
        ide_device->is_ata = 1;
    }else{
        panic("ata identity: not ata");
    }

    ide_device->major =  read_buf[ATA_IDENT_MAJOR];

    if(ide_device->major == 0x0 || ide_device->major == 0xFFFF){
        panic("don't support ata major number");
    }

    if( (ide_device->major & 0x70) != 0x70){
        print_hex(ide_device->major);
        printl("");
        panic("support ata protocol error");
    }

    ide_device->dma_support  = (read_buf[ATA_IDENT_CAPABILITIES]>>8) & 1; 
    ide_device->lba_support  = (read_buf[ATA_IDENT_CAPABILITIES]>>9) & 1;

    if(!ide_device->lba_support){
        panic("ata device don't support lba");
    } 

    ide_device->cylinders    = read_buf[ATA_IDENT_CYLINDERS];
    ide_device->heads        = read_buf[ATA_IDENT_HEADS];
    ide_device->sectors      = read_buf[ATA_IDENT_SECTORS]; 

    ide_device->lba48_support  = (read_buf[ATA_IDENT_COMMANDSETS+1]>>10) & 1;
    ide_device->lba_num = (uint32_t)read_buf[ATA_IDENT_MAX_LBA+1]<<16 + read_buf[ATA_IDENT_MAX_LBA];
}


void summary_ide_device(struct IDE_Device *device){
    printstr("Is ATA: ");      print_hex(device->is_ata);printl("");
    printstr("HEAD: "   );     print_hex(device->heads);printl("");
    printstr("Cylinder: ");    print_hex(device->cylinders);printl("");
    printstr("Sector: ");      print_hex(device->sectors);printl("");
    printstr("LBA Support: "); print_hex(device->lba_support);printl("");
    printstr("LBA 48: "); print_hex(device->lba48_support);printl("");
    printstr("#LBA "); print_hex(device->lba_num);printl("");

}



void ata_wait(uint32_t bus,uint32_t dev_nb){
    uint32_t port = ATA_PR_MST_CTRL_BASE;
    inb(port+ATA_ALT_STATUS);
    inb(port+ATA_ALT_STATUS);
    inb(port+ATA_ALT_STATUS);
    inb(port+ATA_ALT_STATUS);
}


int ata_read_buffer(struct IDE_Device *ide_device,uint8_t *buf,uint32_t cnt){
    uint32_t i;
    uint16_t data;
    uint32_t port = ATA_PR_MST_IO_BASE+ATA_DATA;
    for(i=0;i<cnt;i+=2){
        ata_poll_bsy_drq(ide_device,1);
        data = inw(port);
        buf[i] = (uint8_t)(data &0xFF);
        buf[i+1] = (uint8_t)((data>>8)&0xFF);
    }
    return cnt;
}


int ata_write_buffer(struct IDE_Device *device,uint8_t *buf,uint32_t cnt){
    uint32_t i;
    uint32_t port = ATA_PR_MST_IO_BASE+ATA_DATA;
    for(i=0;i<cnt;i+=2){
        ata_poll_bsy_drq(device,1);
        outw(port,(uint16_t)buf[i]+((uint16_t)buf[i+1]>>8));
    }
    return cnt;
}


void ata_soft_reset(IDE_Device *device){
    int i;
    uint8_t  state = 0; 
    ata_poll_bsy(device);
    outb(ATA_PR_MST_IO_BASE+ATA_HDDEVSEL,ATA_SEL_MASTER);
    ata_poll_bsy(device);
    outb(ATA_PR_MST_CTRL_BASE,0x4);
    inb(ATA_PR_MST_CTRL_BASE+ATA_ALT_STATUS);
    outb(ATA_PR_MST_CTRL_BASE,0x0);
    for(i=0;i<21;++i){
        inb(ATA_PR_MST_CTRL_BASE+ATA_ALT_STATUS);
    }
    do{
        state = inb(ATA_PR_MST_IO_BASE+ATA_STATUS);
    }while(state&ATA_STATUS_BSY);

    uint8_t cl=inb(ATA_PR_MST_IO_BASE+ATA_LBA0);	/* get the "signature bytes" */
    uint8_t cm=inb(ATA_PR_MST_IO_BASE+ATA_LBA1);
	uint8_t ch=inb(ATA_PR_MST_IO_BASE+ATA_LBA2);
    ata_poll_bsy_drq(device,0);
    printl("ATA signatures");
    print_hex(cl);printl("");
    print_hex(cm);printl("");
    print_hex(ch);printl("");
}



uint8_t ata_poll_bsy(struct IDE_Device *device){  
    uint8_t  state = 0;  
    uint32_t io_base = ATA_PR_MST_IO_BASE;
    do{
        state = inb(io_base+ATA_STATUS);
    }while(state&ATA_STATUS_BSY);

    state  = inb(ATA_PR_MST_CTRL_BASE+ATA_ALT_STATUS);
    if(state & ATA_STATUS_ERR || state & ATA_STATUS_DF){
        panic("ata pollling err"); 
    }
    return state;
}


void ata_poll_drdy(struct IDE_Device *device){
    uint8_t  state = 0; 
    do{
        state = inb(ATA_PR_MST_IO_BASE+ATA_STATUS);
    }while(!(state&ATA_STATUS_DRDY));   
}

uint8_t ata_poll_bsy_drq(struct IDE_Device *device,int set_drq){
    uint8_t state = ata_poll_bsy(device);
    if(((set_drq << 3) & state) | (~state & ~(set_drq << 3))){
        return state;
    }
    panic("ata polling drq error");
}



void ata_detect(uint32_t bus,uint32_t dev_nb){
    outb(ATA_PR_MST_IO_BASE+ATA_HDDEVSEL,ATA_SEL_MASTER);
    ata_poll_bsy_drq(&ide_device,0);
    ata_poll_drdy(&ide_device);
    outb(ATA_PR_MST_IO_BASE+ATA_SECCOUNT,0);
    outb(ATA_PR_MST_IO_BASE+ATA_LBA0,0);
    outb(ATA_PR_MST_IO_BASE+ATA_LBA1,0);
    outb(ATA_PR_MST_IO_BASE+ATA_LBA2,0);
    // identify command
    outb(ATA_PR_MST_IO_BASE+ATA_COMMAND,0xEC);
    ata_wait(bus,dev_nb);
    uint8_t status = inb(ATA_PR_MST_IO_BASE+ATA_STATUS); 
    if(!status){
        panic("ata device not found");
    }

    // read signature
    uint8_t cl=inb(ATA_PR_MST_IO_BASE+ATA_LBA0);	/* get the "signature bytes" */
    uint8_t cm=inb(ATA_PR_MST_IO_BASE+ATA_LBA1);
	uint8_t ch=inb(ATA_PR_MST_IO_BASE+ATA_LBA2);
    ata_poll_bsy_drq(&ide_device,0);
   // printl("ATA signatures");
   // print_hex(cl);printl("");
    //print_hex(cm);printl("");
   // print_hex(ch);printl("");

	/* differentiate ATA, ATAPI, SATA and SATAPI */
	//if (cl==0x14 && ch==0xEB) return ATADEV_PATAPI;
	//if (cl==0x69 && ch==0x96) return ATADEV_SATAPI;
	//if (cl==0 && ch == 0) return ATADEV_PATA;
	//if (cl==0x3c && ch==0xc3) return ATADEV_SATA;

    //while(1){}

    ata_read_buffer(&ide_device,buffer,256);
    parse_identify_command(&ide_device,buffer);
    summary_ide_device(&ide_device);
    ata_wait(bus,dev_nb);
}

// read one sector
uint32_t ata_read_sector(Disk_IO_Command *command){
    
    int is_slave=0;
    int disable_intr=1;

    uint32_t i;
    uint32_t sec_num = command->sec_num;
    uint32_t lba_start = command->lba_start;
    IDE_Device *device = command->device;
    outb(ATA_PR_MST_IO_BASE+ATA_HDDEVSEL,0xE0|is_slave<<4|(uint8_t)(lba_start>>24 & 0x0F));
    ata_poll_bsy_drq(device,0);
    ata_poll_drdy(device);
    outb(ATA_PR_MST_CTRL_BASE+ATA_DEV_CTRL,0x0|disable_intr<<1);
    //outb(ATA_PR_MST_IO_BASE+ATA_HDDEVSEL,0xE0|is_slave<<4|(uint8_t)(lba_start>>24 & 0x0F));
    outb(ATA_PR_MST_IO_BASE+ATA_SECCOUNT,sec_num);
    outb(ATA_PR_MST_IO_BASE+ATA_LBA0,(uint8_t)(lba_start & 0xFF));
    outb(ATA_PR_MST_IO_BASE+ATA_LBA1,(uint8_t)(lba_start>>8 & 0xFF));
    outb(ATA_PR_MST_IO_BASE+ATA_LBA2,(uint8_t)(lba_start>>16 & 0xFF));
    outb(ATA_PR_MST_IO_BASE+ATA_COMMAND,ATA_CMD_READ_SECTORS);
    ata_wait(0,0);
    for(i=0;i<sec_num;++i){
        if(i>0){
            ata_poll_bsy_drq(device,1);
        }
        ata_read_buffer(&ide_device,command->buf,512);
    }
}


// read one sector
uint32_t ata_write_sector(Disk_IO_Command *command){
    int is_slave=0;
    int disable_intr=1;
    uint32_t i;
    outb(ATA_PR_MST_IO_BASE+ATA_HDDEVSEL,0xE0|is_slave<<4|(uint8_t)(command->lba_start>>24 & 0x0F));
    ata_poll_bsy_drq(command->device,0);
    ata_poll_drdy(command->device);   
    outb(ATA_PR_MST_CTRL_BASE+ATA_DEV_CTRL,0x0|disable_intr<<1);
    outb(ATA_PR_MST_IO_BASE+ATA_SECCOUNT,command->sec_num);
    outb(ATA_PR_MST_IO_BASE+ATA_LBA0,(uint8_t)(command->lba_start & 0xFF));
    outb(ATA_PR_MST_IO_BASE+ATA_LBA1,(uint8_t)(command->lba_start>>8 & 0xFF));
    outb(ATA_PR_MST_IO_BASE+ATA_LBA2,(uint8_t)(command->lba_start>>16 & 0xFF));
    outb(ATA_PR_MST_IO_BASE+ATA_COMMAND,ATA_CMD_WRITE_SECTORS);
    ata_wait(0,0);
    
    for(i=0;i<command->sec_num;++i){
        if(i>0){
            // according to ata 6 protocol , when enter from PIOO1 state
            //.the host shall wait one PIO transfer cycle time before reading the Status register. The wait may be
            // accomplished by reading the Alternate Status register and ignoring the result
            inb(ATA_PR_MST_CTRL_BASE+ATA_ALT_STATUS); 
            ata_poll_bsy_drq(command->device,1);
        }
        ata_write_buffer(command->device,&command->buf[512*i],512);
      
    }
    
    inb(ATA_PR_MST_CTRL_BASE+ATA_ALT_STATUS); 
    ata_poll_bsy_drq(command->device,0);
    ata_poll_drdy(command->device);   
    //flush the cache
    outb(ATA_PR_MST_IO_BASE+ATA_COMMAND,0xE7);
    ata_wait(0,0);
    ata_poll_bsy_drq(command->device,0);
}