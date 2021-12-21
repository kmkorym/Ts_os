#include "../drivers/ata.h"
#include "../kernel/common.h"

#define FAT_ENT_NUM_MAX 512  // remove this by using kernel heap in future


#define FAT_CLUS_ROOT 0
#define FAT_CLUS_NORMAL(c)(c>=3 && c<=0xFFEF)
#define FAT_CLUS_FREE(c)(c==0)
#define FAT_CLUS_BAD(c)(c==0xFFF7)
#define FAT_CLUS_EOF(c)(c>=0xFFF8 && c<=0xFFFF)


#define FAT_FILE_EOF -1
// 1. get superblock, get parameter
//    read sector 0 , parse the content
//     


// 2. get root directory and save them to memory


// 3. iterate throught ditectory and then match file name

// 4. read fat table , implement cluster -> LBA transfer ...


#define FAT16_CTL_DIR        0x01
#define FAT16_CTL_WRITE      0x02
#define FAT16_CTL_APPEND     0x04
#define FAT16_CTL_CREATE     0x08
#define FAT16_CTL_IS_ROOT    0x80000000


struct FatSuperBlock{
    uint8_t jmp_instrction[3]; // ignore
    uint8_t oem_name[8];
    //BPB
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserve_sectors;
    uint8_t  fat_num;
    uint16_t root_ents_num;
    uint16_t small_sectors_num;
    uint8_t  media_desc;
    uint16_t sectors_fat;
    uint16_t sectors_per_track;
    uint16_t head_num;
    uint32_t hidden_sectors;  // when partition, offset of first sector of this volume
    uint32_t large_sectors_num;
}__attribute__((packed));
typedef struct FatSuperBlock FatSuperBlock;

struct FatDirEntry{
    char filename[8];
    char ext[3];
    uint8_t attr;
    char reserved;
    uint8_t create_mill; // not support now
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access;
    uint16_t reserve_32;
    uint16_t last_wtime;
    uint16_t last_wdate;
    uint16_t start_cluster;
    uint32_t file_size;
}__attribute__((packed));
typedef struct FatDirEntry FatDirEntry;



struct Fat16Volume{
    IDE_Device * device;
    FatSuperBlock sb       ;
    FatDirEntry root;
    uint32_t root_sectors;
    uint16_t volume_start  ;
    uint16_t fat_start     ;
    uint16_t root_dir_start ;
    uint16_t data_start    ;
    uint32_t cluster_bytes;
    
};
typedef struct Fat16Volume Fat16Volume;


struct Fat16File{
    char *path;
    Fat16Volume *volume;
    FatDirEntry *dirent;
    uint32_t  dirent_sector; //only meaningful in for root entry
    uint16_t  dirent_cluster; 
    uint16_t current_cluster;
    uint32_t byte_offset;
    uint32_t flag;
};
typedef  struct Fat16File Fat16File;

Fat16File* new_fat16_file(char *path,Fat16Volume * volume,FatDirEntry *ent,uint32_t flag);
void dump_fat(Fat16Volume *volume);
int fat16_dirlist(Fat16File * file);
Fat16File* fat_close(Fat16File* file);
Fat16File * fat16_find(Fat16File* file,char *path);
Fat16File* fat_open(Fat16Volume * volume,char *path,uint32_t flag);
int fat_get_volume(Fat16Volume *volume);
void fat16_display_super_block(FatSuperBlock * sb);
int fat_read_file(Fat16File *file,char *buf,uint32_t buf_size);
int  fat_create_file(Fat16Volume * volume,char *base_dir,char *name,uint32_t flag);
int fat_rewind(Fat16File *file,int offset);
int fat_write_file(Fat16File *file,char *user_buf,uint32_t buf_size);
int fat_delete_file(Fat16Volume * volume,char *path);

