#include "fat16.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include "../kernel/malloc.h"

#define ENT_ATTR_DIR 0x10
#define FAT_ROOT_ENT ((FatDirEntry*)(1))


FatDirEntry * search_name_in_block(uint8_t *block,uint32_t bsize,char *name);
int fat_read_dirent_block(Fat16File *file,uint8_t* buf);
int  fat16_get_superblock(IDE_Device *device,FatSuperBlock * sb);
int is_directory(FatDirEntry * dirent);
uint16_t read_fat(Fat16Volume *volume,uint16_t cluster,char *buf);
uint16_t fat_get_next_cluster(Fat16Volume *volume,uint16_t cluster);
int  _fat_create_file(Fat16File *parent,char *name,uint32_t flag);
char *  get_basedir(char *path,char *buf);


/*
    this interface shoud add dirent_cluster ..
*/
Fat16File* new_fat16_file(char *path,Fat16Volume * volume,FatDirEntry *ent,uint32_t flag){
    
    Fat16File    *file = (Fat16File *) kmalloc(sizeof(Fat16File) ) ;
   
    if(!ent){
        file->dirent =  (FatDirEntry *) kmalloc(sizeof(FatDirEntry) ) ;
        file->current_cluster = 0xFF ;
    }else{
        file->dirent = ent ;
        file->current_cluster = ent->start_cluster;
    }

    file->byte_offset = 0;
    file->volume = volume;
    file->flag = flag;
    
    if(path){
        file->path = (char*) kmalloc(sizeof(char)*(strlen(path)+1));
        strcpy(path,file->path);
    }else{
        file->path = NULL;
    }

    file->dirent_cluster = 0xFF;
    file->dirent_sector  = 0xFF;

    return file;
}


char* get_filename(char *path,char *buf){
    
    if(!path){
        return NULL;
    }

    char *p,*q;
    int l = strlen(path);
    
    p = path + l -1 ;
    
    while(path <= p && *p == '/' ){
        --p;
    }

    if(path > p){
        return NULL;
    }

    q = p ;

    while(path <= p && *p!='/'){
        --p;
    }

    ++p;
    
    memcpy(p,buf,q-p+1);
    buf[q-p+1] = 0;

    return p;

}


char*  get_basedir(char *path,char *buf){
    char *s = NULL;
    char *d = NULL;
    s =  path;
    while(*s){
        if( *s =='/' && *(s+1)){
            d = s;
        }
        ++s;
    }
    if(!(s-d)){
        return NULL;
    }
    if(buf){
        memcpy(path,buf,d-path+1);
    }
    buf[d-path+1] = 0;
    return d;
}



int fat_rewind(Fat16File *file,int offset){
    return -1;
}
int fat16_dirlist(Fat16File * file){
    return 1;
}

/*
int fat16_dirlist(Fat16File * file){
    
    if(!file){
        return -1;
    }
    
    if(! is_directory(file->it->dirent)){
        return -2;
    }

    FatDirEntry *dirent = NULL;
    char buf[file->it->volume->cluster_bytes];
    memset(buf,0,file->it->volume->cluster_bytes);

    while(EOF!=fat_read_file(file,buf,file->it->volume->cluster_bytes)){
        dirent = (FatDirEntry *) buf;
        while( (char*)dirent - buf < file->it->volume->cluster_bytes){
            if(FAT_NORMAL(dirent->start_cluster)){
                
                if(!is_directory(dirent)){
                    print_chars(dirent->filename,8);printstr(".");print_chars(dirent->ext,3);
                }else{
                    print_chars(dirent->filename,8);
                }
                printl("");
            }
            ++dirent;
        }
    }

    return 0;
}
*/

int  fat16_get_superblock(IDE_Device *device,FatSuperBlock * sb){
    
    uint8_t buf[512];

    Disk_IO_Command  cmd;
    cmd.device = device;
    cmd.buf_size = 512;
    cmd.lba_start = 0;
    cmd.sec_num = 1;
    cmd.buf= buf;
     
    if(ata_read_sector(&cmd)<1){
        printl("fat 16 read superblock 512 bytes failed");
        return -1;
    }

    FatSuperBlock *sb2 = (FatSuperBlock*) (buf);
    *sb=*sb2;

    return 1;
}



int fat_get_volume(Fat16Volume *volume){

    if(!volume){
        printl("null volume device");
        return -1;
    }

    fat16_get_superblock(volume->device,&volume->sb);

    volume->volume_start = volume->sb.hidden_sectors;
    volume->fat_start = volume->volume_start+volume->sb.reserve_sectors;
    volume->root.start_cluster = FAT_CLUS_ROOT;
    volume->root_dir_start =  volume->fat_start + volume->sb.sectors_fat*volume->sb.fat_num;
    volume->data_start = volume->root_dir_start + (volume->sb.root_ents_num*sizeof(FatDirEntry)/volume->sb.bytes_per_sector);
    volume->cluster_bytes = volume->sb.sectors_per_cluster*volume->sb.bytes_per_sector ;
    volume->root_sectors =  (volume->sb.root_ents_num*sizeof(FatDirEntry)+volume->sb.bytes_per_sector-1)/volume->sb.bytes_per_sector;
    memset( (char*) &volume->root,0,sizeof(FatDirEntry));
    return 1;
}



void dump_fat(Fat16Volume *volume){
    int i;
    uint16_t buf[2048] = {0};
    Disk_IO_Command cmd = {.buf = (char*) buf, .buf_size = 4096, .sec_num=4, 
                        .device = volume->device , .lba_start=volume->fat_start };
    printl("dump fats:");
    ata_read_sector(&cmd);
    for(i=0;i<16;++i){
        print_hex(buf[i]);printstr(" ");
    }

}


void fat16_pad(char *buf,char *s){
    memcpy(s,buf,9);
    str_pad(buf,' ',8);
    buf[8] = ' ';
}


void fat16_filename_fmt(char *buf,char *s){
    int i;
    fat16_pad(buf,s);
    for(i=0;i<9;++i){
        buf[i] = upper_case(buf[i]);
    }
}




int is_directory(FatDirEntry * dirent){
    if(dirent->attr & ENT_ATTR_DIR){
        return 1;
    }
    return 0;
}


int fat_filename_match(FatDirEntry *ent,char *name){
    return str_equal_range(ent->filename,name,0,7);
}



int fat_write_cluster(Fat16Volume *volume,char * buf,int cluster_start,int sec_n){
    
    Disk_IO_Command  cmd;
    cmd.device = volume->device;
    cmd.buf= buf;
    cmd.buf_size = volume->cluster_bytes;
    cmd.lba_start = volume->data_start+( cluster_start-2)*volume->sb.sectors_per_cluster;
    cmd.sec_num =  volume->sb.sectors_per_cluster;

    while(sec_n--){
        cmd.lba_start+=ata_write_sector(&cmd);
    }

    return 1;
}


/*
uint16_t _fat_search_root_for_sector(Fat16Volume * volume,char *name){
    Disk_IO_Command cmd;
    char buf[volume->cluster_bytes];
   
}
*/


int _fat_search_root(Fat16File   *file,char *name){

    Disk_IO_Command cmd;
    Fat16Volume * volume = file->volume;
    uint32_t read_sector = 0;
    char buf[volume->cluster_bytes];
    FatDirEntry *target = NULL;
 
    cmd.buf = buf; cmd.buf_size = volume->cluster_bytes;
    cmd.device = volume->device; cmd.lba_start = volume->root_dir_start; cmd.sec_num = volume->sb.sectors_per_cluster;
    
    while( (cmd.lba_start - volume->root_dir_start) < volume->root_sectors ){
        read_sector = ata_read_sector(&cmd);
        target = search_name_in_block(buf,read_sector*volume->sb.bytes_per_sector,name);
        if(target){
            break;
        }
        cmd.lba_start = cmd.lba_start + read_sector;
    }

    if(!target){
        return -1;
    }

    *(file->dirent) = *target;
    file->current_cluster =  file->dirent->start_cluster;
    file->dirent_cluster = FAT_CLUS_ROOT;
    file->dirent_sector = cmd.lba_start + ((char*)target-buf)/volume->sb.bytes_per_sector;

    return 1;
}




int fat_read_cluster(Fat16Volume *volume,uint16_t cluster,char *buf){
    Disk_IO_Command  cmd;
    cmd.device    = volume->device;
    cmd.buf = buf; cmd.buf_size  = volume->cluster_bytes;
    cmd.lba_start = volume->data_start+(cluster-2)*volume->sb.sectors_per_cluster;
    cmd.sec_num = volume->sb.sectors_per_cluster;
    return   ata_read_sector(&cmd);
}



int write_fat(Fat16Volume *volume,uint16_t cluster,uint16_t val){

    char buf[volume->sb.bytes_per_sector];
    uint16_t sector = 0;

    sector = read_fat(volume,cluster,buf);
    ((uint16_t*)buf)[cluster%(volume->sb.bytes_per_sector/2)] = val;

    Disk_IO_Command  cmd = {.buf = buf, .buf_size =volume->sb.bytes_per_sector, .device = volume->device,
                            .lba_start = sector , .sec_num=1 };
    
    ata_write_sector(&cmd);
    return sector;
}

uint16_t read_fat(Fat16Volume *volume,uint16_t cluster,char *buf){

    uint16_t fat_sector =   volume->fat_start+cluster*2/volume->sb.bytes_per_sector;
    Disk_IO_Command  cmd;

    cmd.device    = volume->device;
    cmd.buf_size  = volume->sb.bytes_per_sector;
    cmd.lba_start = fat_sector;
    cmd.sec_num   = 1;
    cmd.buf       = buf;
    ata_read_sector(&cmd);

    return fat_sector;
}



uint16_t fat_get_next_cluster(Fat16Volume *volume,uint16_t cluster){
    char buf[volume->sb.bytes_per_sector];
    read_fat(volume,cluster,buf);
    return ((uint16_t*)buf)[cluster%(volume->sb.bytes_per_sector/2)];
}





#define PRINT_STRUCT_MEMBER(name,field,ftype)\
    printstr(#field ": "); print##ftype(name->field);printl("");\

void fat16_display_super_block(FatSuperBlock * sb){
    PRINT_STRUCT_MEMBER(sb,bytes_per_sector,_hex);
    PRINT_STRUCT_MEMBER(sb,sectors_per_cluster,_hex);
    PRINT_STRUCT_MEMBER(sb,reserve_sectors,_hex);
    PRINT_STRUCT_MEMBER(sb,fat_num,_hex);
    PRINT_STRUCT_MEMBER(sb,root_ents_num,_hex);
    PRINT_STRUCT_MEMBER(sb,small_sectors_num,_hex);
    PRINT_STRUCT_MEMBER(sb,sectors_fat,_hex);
    PRINT_STRUCT_MEMBER(sb,sectors_per_track,_hex);
    PRINT_STRUCT_MEMBER(sb,head_num,_hex);
    PRINT_STRUCT_MEMBER(sb,hidden_sectors,_hex);
    PRINT_STRUCT_MEMBER(sb,large_sectors_num,_hex);
}

FatDirEntry*  search_name_in_block(uint8_t *block,uint32_t bsize,char *name){
    FatDirEntry* ent = NULL;
    uint8_t *p = block;
    while( bsize > p-block){
        ent = (FatDirEntry*)p;
        if(!FAT_CLUS_FREE(ent->start_cluster) && fat_filename_match(ent,name)){
            return ent;
        }
        p+=sizeof(FatDirEntry);
    }
    return NULL;
}


// name is formated name
Fat16File* _fat_search_file(Fat16File *file,char *fmt_name,uint16_t start_cluster){

    Fat16Volume *volume = file->volume;
    FatDirEntry *dirent;
    uint8_t buf[volume->cluster_bytes];
    uint16_t cluster = start_cluster;
    

    if( start_cluster ==  FAT_CLUS_ROOT ){
        return _fat_search_root(file,fmt_name) > 0 ? file:NULL;
    }

    while(FAT_CLUS_NORMAL(cluster)){

        if(!fat_read_cluster(volume,cluster,buf)){
            panic("fat16: fat_read_dirent,read 0 bytes ");
        }

        if ( dirent = search_name_in_block(buf,volume->cluster_bytes,fmt_name) ){
            *(file->dirent)       = *dirent;
            file->dirent_cluster  = cluster;
            file->current_cluster = file->dirent->start_cluster;
            return file;
        }

        cluster = fat_get_next_cluster(volume,cluster);
    }

    return NULL;
}

Fat16File* fat_search_file(Fat16File* file,char *name,uint16_t start_cluster){
    char fmt_name[16] = {0};
    fat16_filename_fmt(fmt_name,name);
    return _fat_search_file(file,fmt_name,start_cluster);
}

 /*
        Subdirectory. (Since MS-DOS 1.40 and PC DOS 2.0) 
        Indicates that the cluster-chain associated with this entry gets interpreted as subdirectory instead of as a file.
        Subdirectories have a filesize entry of zero.
    */


// read ditectory or file cluster by cluster
int fat_read_file_one_cluster(Fat16File *file,uint8_t* buf){   

    Fat16Volume *volume       = file->volume;
    uint16_t current_cluster  = file->current_cluster;
    int ret = 0;

    if(FAT_CLUS_EOF(current_cluster)){
        return FAT_FILE_EOF;
    }

    ret = fat_read_cluster(volume,current_cluster,buf);

    if(!ret){
        panic("fat16: fat_read_dirent,read 0 bytes ");
    }
    
    file->byte_offset+= ret*volume->sb.bytes_per_sector;
    file->current_cluster = fat_get_next_cluster(volume,current_cluster);

    return ret*volume->sb.bytes_per_sector;
}





Fat16File* fat_open(Fat16Volume * volume,char *path,uint32_t flag){

    char base_dir[128] = {0};
    char filename[128] = {0};
    char *s = NULL;
    Fat16File * parent;
    Fat16File * file = NULL;
    uint16_t cluster;
    uint16_t tmp;
   
    if(flag&FAT16_CTL_CREATE){
        // if file path exists, return ..
        if(file=fat_open(volume,path,flag&~(FAT16_CTL_CREATE))){
            file->flag = flag;
            return file;
        }
        
        if( get_basedir(path,base_dir) == NULL || get_filename(path,filename) == NULL ){
            printl("fat create file, path parsing error");
            return NULL;
        }
        
        parent =  fat_open(volume,base_dir,0); 
        if(!parent){
            return NULL;
        }
        if(_fat_create_file(parent,filename,flag)){
            file = fat_open(volume,path,flag&~(FAT16_CTL_CREATE));
            file->flag |= FAT16_CTL_CREATE;
        }else{
            panic("create file failed");
        }
        return file;
    }

    file = new_fat16_file(path,volume, NULL,flag);
    file =  fat16_find(file,path);
    
    if(!file){
        return NULL;
    }

    if(flag&FAT16_CTL_APPEND){
        file->flag|=FAT16_CTL_WRITE;
        cluster =  file->dirent->start_cluster;
        while(1){
            tmp = fat_get_next_cluster(volume,cluster);
            if(!FAT_CLUS_NORMAL(tmp)){
                break;
            }
            cluster = tmp;
        }
        file->current_cluster = cluster;
        file->byte_offset     = file->dirent->file_size;
    }

    return file;
}

Fat16File* fat_close(Fat16File* file){
    if(file->path){
        kfree((uint32_t*)file->path);
    }
    kfree((uint32_t*)file->dirent);
    kfree((uint32_t*)file);
}




int fat_rw_root(Fat16File *file,char *user_buf,uint32_t buf_size){

    if(! (file->dirent->start_cluster == FAT_CLUS_ROOT)){
        return -2;
    }

    Disk_IO_Command cmd;
    Fat16Volume * volume = file->volume;
    char block_buf[volume->cluster_bytes];
    int  offset = file->byte_offset;
    int  sector = volume->root_dir_start+offset/volume->sb.bytes_per_sector;
    int  next_offset;
    int  rw_cnt = 0;
    uint32_t flag  = file->flag;

    if( sector - volume->root_dir_start >= volume->root_sectors ){
        return FAT_FILE_EOF;
    }

    while( offset<file->byte_offset+buf_size ){
        if( offset/volume->sb.bytes_per_sector >= volume->root_sectors  ){
            break;
        }
        cmd.buf = block_buf; cmd.buf_size = volume->cluster_bytes; 
        cmd.sec_num = min(volume->sb.sectors_per_cluster, volume->root_sectors-offset/volume->sb.bytes_per_sector);
        cmd.device = volume->device ; cmd.lba_start = volume->root_dir_start+offset/volume->sb.bytes_per_sector;
        next_offset = min(offset+cmd.sec_num * volume->sb.bytes_per_sector,file->byte_offset+buf_size);
        // when writing , read content before write and write back to block
        ata_read_sector(&cmd);
        if(flag & FAT16_CTL_WRITE){
            memcpy(user_buf+offset-file->byte_offset,block_buf+offset-(offset/volume->sb.bytes_per_sector)*volume->sb.bytes_per_sector\
                  ,next_offset-offset);
            ata_write_sector(&cmd);
        }else{
            memcpy(block_buf+offset-(offset/volume->sb.bytes_per_sector)*volume->sb.bytes_per_sector,user_buf+rw_cnt,next_offset-offset);
        }
        rw_cnt+=next_offset-offset;
        offset = next_offset;
    }

    file->byte_offset+=rw_cnt;
    return rw_cnt;
}

int fat_read_file(Fat16File *file,char *user_buf,uint32_t buf_size){
    
    if(!file){
        panic("open NULL file");
    
    }

    if( file->dirent->start_cluster == FAT_CLUS_ROOT){
        return fat_rw_root(file,user_buf,buf_size);
    }

    if( !is_directory(file->dirent) && file->byte_offset >= file->dirent->file_size ){
        return FAT_FILE_EOF;
    }

    if(FAT_CLUS_EOF(file->current_cluster)){
        return FAT_FILE_EOF;
    }

    Fat16Volume * volume = file->volume;
    char block_buf[volume->cluster_bytes];
    int  cluster = file->current_cluster;
    int  offset = file->byte_offset;
    int  next_offset;
    int  read_cnt = 0;

    while(offset<file->byte_offset+buf_size && !FAT_CLUS_EOF(cluster)){
        
        fat_read_cluster(volume,cluster,block_buf);
        next_offset = min( file->byte_offset + buf_size,(offset+volume->cluster_bytes)/volume->cluster_bytes * volume->cluster_bytes);
        memcpy(block_buf+ (offset%volume->cluster_bytes) ,user_buf+read_cnt,next_offset-offset);
        read_cnt+=next_offset-offset;

        if(next_offset/volume->cluster_bytes != offset/volume->cluster_bytes){
            cluster = fat_get_next_cluster(volume,cluster);
        }

        offset = next_offset;
    }

    if(!is_directory(file->dirent)){
        if(offset >= file->dirent->file_size){
            read_cnt-= offset - file->dirent->file_size;
        }
    }

    file->current_cluster = cluster;
    file->byte_offset     += read_cnt;  

    return  read_cnt; 
}


uint16_t fat_get_free_cluster(Fat16Volume * volume){
    // read super block get first secotr of fat table
    // search fat table sector by sector and find first 0x0 fat entry 
    //       record fat count and be careful of cluster -2 

    // if not found return 0
    
    char buf[volume->cluster_bytes];
    Disk_IO_Command cmd = {.device=volume->device,.buf=buf,.buf_size=volume->cluster_bytes,
                           .lba_start=volume->fat_start,.sec_num=volume->sb.sectors_per_cluster};

    int        rd_sec  = 0;
    uint16_t  *ptr_fat = 0;
    uint16_t   fat_cnt = 0;

    while( (cmd.lba_start - volume->fat_start) < volume->sb.sectors_fat){
       
        rd_sec  = ata_read_sector(&cmd);
        ptr_fat = (uint16_t*) buf;

        while(((char*)ptr_fat-buf)<volume->cluster_bytes){
            if( fat_cnt > 2 && FAT_CLUS_FREE(*ptr_fat)){
                return fat_cnt;
            }
            ++fat_cnt;
            ++ptr_fat;
        }

        cmd.lba_start+=rd_sec;

    }
    panic("fat16: run out of clusters");
    return 0;
}


/*
    first find direntry of the file
    it must exist before write 

    it will write string to file from file's byte offset 
    and adjust it's file size it exceed the current file size


    when the file content exceed current cluster, create new cluster and
    update the fat table 


*/


int  fat_update_file_dirent(Fat16File *file,FatDirEntry *new_dirent){

    Disk_IO_Command cmd;
    Fat16Volume *volume = file->volume;
    FatDirEntry *dirent ;
    char block_buf[volume->cluster_bytes];

    if( file->dirent_cluster  == FAT_CLUS_ROOT ){
        cmd.buf = block_buf; cmd.buf_size = volume->cluster_bytes;
        cmd.device = volume->device; cmd.lba_start = file->dirent_sector; cmd.sec_num = 1;
        ata_read_sector(&cmd);
    }else{
        fat_read_cluster(volume,file->dirent_cluster,block_buf);
    }
    
    if(dirent=search_name_in_block(block_buf,volume->cluster_bytes,file->dirent->filename) ){
       
        if(new_dirent){
            *dirent = *(new_dirent);
        }else{
            *dirent = *(file->dirent) ; 
        }
       
        if( file->dirent_cluster  == FAT_CLUS_ROOT ){
            ata_write_sector(&cmd);
        }else{
            fat_write_cluster(volume,block_buf,file->dirent_cluster,1);
        }
    }else{
        panic("fat16 : write but no dirent find");
    }

    return 0;
}

int fat_write_file(Fat16File *file,char *user_buf,uint32_t buf_size){

    if(!file){
        panic("fat16 : open NULL file");
    }
    
    if( file->dirent->attr & ENT_ATTR_DIR){
        printl("fat16 :error, write on directory");
        return -2;
    }

    if(!(file->flag&FAT16_CTL_WRITE)){
        printl("fat16 : write on read mode file");
        return -2;
    }


    if(FAT_CLUS_EOF(file->current_cluster)){
        return FAT_FILE_EOF;
    }

    Fat16Volume *volume = file->volume;
   
    char block_buf[volume->cluster_bytes];
    int  cluster = file->current_cluster;
    int  write_cnt = 0;
    uint32_t tmp;


    while(write_cnt < buf_size){
        fat_read_cluster(volume,cluster,block_buf);
        tmp = min(volume->cluster_bytes-(file->byte_offset % volume->cluster_bytes),buf_size-write_cnt);
        memcpy(user_buf + write_cnt,block_buf + (file->byte_offset % volume->cluster_bytes),tmp);
        fat_write_cluster(volume,block_buf,cluster,1);
        file->byte_offset+=tmp;
        write_cnt+=tmp;
        if(file->byte_offset % volume->cluster_bytes == 0){
            tmp = fat_get_next_cluster(volume,cluster);
            if(FAT_CLUS_EOF(tmp)){
                tmp =  fat_get_free_cluster(volume);
                write_fat(volume,cluster,tmp);
                write_fat(volume,tmp,0xFFFF);
                memset(block_buf,0,volume->cluster_bytes);
            }
            cluster = tmp;
        }       
    }

    file->dirent->file_size = file->byte_offset;
    file->current_cluster = cluster;
    fat_update_file_dirent(file,NULL);
    return  buf_size; 
}


/*
    // delete file
    1. find  dirent and check dirent type
    2.  delete content clusters by go through fat table
    3. go to dirent cluster 
        - check whether is ROOT CLUSTER
    4. read --> modify the dirent entry  --> write back


    // delete directory
    1. find  dirent and check dirent type
    2. go through cluster and each cluster iterate the entries
        - skip . and ..
    3. for each entry delete content clusters by go through fat table
    4. go to dirent cluster
        - check whether is ROOT CLUSTER
    5.  read --> modify the dirent entry  --> write back
    
*/

int fat_free_clusters_until_end(Fat16Volume * volume,uint16_t cluster){
    uint16_t tmp ;
    while(FAT_CLUS_NORMAL(cluster)){
        tmp = cluster;
        cluster = fat_get_next_cluster(volume,cluster);
        write_fat(volume,tmp,0x0);
    }
    return 0;
}


int  _fat_free_childs_under_dir(Fat16File *file){
    
    FatDirEntry  *ent_in_blk   = NULL;
    Fat16File    *sub_dir_file = new_fat16_file("",file->volume,NULL,0);
    FatDirEntry   new_ent;
    char block_buf[file->volume->cluster_bytes];
    uint16_t cluster = file->dirent->start_cluster;
    uint16_t tmp;
   
    while(FAT_CLUS_NORMAL(cluster)){

        fat_read_cluster(file->volume,cluster,block_buf);

        ent_in_blk = (FatDirEntry*)block_buf;

        while( (char*) ent_in_blk -  block_buf < file->volume->cluster_bytes){
            
            if( !FAT_CLUS_NORMAL(ent_in_blk->start_cluster)){
                ++ent_in_blk;
                continue;
            }

            if(    (fat_filename_match(ent_in_blk,".        ") ) 
                || (fat_filename_match(ent_in_blk,"..       ") ) 
            ) {
                ++ent_in_blk;
                continue;
            }

            if(!is_directory(ent_in_blk)){
                fat_free_clusters_until_end(file->volume,ent_in_blk->start_cluster);
            }else{
                // don't consider the case of delete directory under / 
                // because this function only called by fat_delete_file and the ent_in_blk
                // is the sub directory entry
                sub_dir_file->dirent_cluster = cluster     ;
                *(sub_dir_file->dirent)      = *ent_in_blk ;
                _fat_free_childs_under_dir(sub_dir_file);
            }

            ++ent_in_blk;
        }

        tmp = fat_get_next_cluster(file->volume,cluster);
        write_fat(file->volume,cluster,0x0);
        cluster = tmp;
    }

    fat_close(sub_dir_file);

    return 1;
}

int fat_delete_file(Fat16Volume * volume,char *path){

    FatDirEntry new_ent ;
    Fat16File  *file = fat_open(volume,path,0); 

    if(!file){
        return -1;
    }

    if(file->current_cluster == FAT_CLUS_ROOT){
        printl("error : can't delete root directory");
        return -1;
    }

    if( is_directory(file->dirent)){
        _fat_free_childs_under_dir(file);
    }else{
        fat_free_clusters_until_end(file->volume,file->dirent->start_cluster);
    }

    new_ent = *(file->dirent);
    // must set filename[0] to 0xE5
    new_ent.filename[0] = 0xE5;
    new_ent.file_size  = 0;
    new_ent.start_cluster = 0;
    fat_update_file_dirent(file,&new_ent);
    fat_close(file);
    return 1;
}

/*
 must test for extra two situtation
 1. expand cluster 
 2. directory cluster number >1 create file
*/

int  _fat_create_file(Fat16File *parent,char *name,uint32_t flag){

    Fat16Volume *volume =  parent->volume;
    FatDirEntry *dirent =  NULL;
    Disk_IO_Command cmd;

    char buf[volume->cluster_bytes];
    uint16_t new_cluster    =  0;
    uint32_t tmp            =  0;
    uint16_t parent_cluster =  parent->dirent->start_cluster;
 
    memset(buf,0,parent->volume->cluster_bytes);

    while(1){
        // dirent is full, must expand
        dirent = (FatDirEntry*)buf;
        tmp = parent->current_cluster;
        fat_read_file(parent,buf,parent->volume->cluster_bytes);
        if(FAT_CLUS_EOF(parent->current_cluster)){
            parent_cluster = tmp;
            tmp  =  fat_get_free_cluster(volume);
            // now , parent cluster is the last NON EOF cluster 
            // modify its fat table entry
            write_fat(volume,parent_cluster,tmp);
            write_fat(volume,tmp,0xFFFF);
            parent_cluster = tmp;
            memset(buf,0,volume->cluster_bytes);
            goto create_file_dirent;
        }
       
        while( (char*)dirent - buf < parent->volume->cluster_bytes){
            //  avoid the case of /usr/.. -->  dirent->start_cluster = 0 but it's not free entry
            if(FAT_CLUS_FREE(dirent->start_cluster) && !fat_filename_match(dirent,"..      ")){
                
                create_file_dirent:
                
                new_cluster = fat_get_free_cluster(volume);
                // attr : 0x10 for directory , 0x00 for normal file
                // only create file currently
                memset((char*)dirent,0,sizeof(FatDirEntry));
                dirent->attr =  0x00;
                fat16_filename_fmt(dirent->filename,name);

                if(flag&FAT16_CTL_DIR){
                    dirent->attr |= ENT_ATTR_DIR;
                }else{
                    memcpy("txt",dirent->ext,3);
                }

                dirent->start_cluster = new_cluster;
                // update dirent block

                if( parent->dirent->start_cluster == FAT_CLUS_ROOT ){
                    tmp = ((char*)dirent - buf)/volume->sb.bytes_per_sector;
                    cmd.buf       =  buf + tmp*volume->sb.bytes_per_sector;
                    cmd.buf_size  =  volume->sb.bytes_per_sector;
                    cmd.device    =  volume->device;
                    cmd.lba_start =  volume->root_dir_start + parent->byte_offset/volume->sb.bytes_per_sector
                                     -volume->sb.sectors_per_cluster + tmp ;
                    cmd.sec_num   = 1;
                    ata_write_sector(&cmd);

                }else{
                    fat_write_cluster(volume,buf,parent_cluster,1);
                }
                
                // update fat table
                write_fat(volume,new_cluster,0xFFFF);
                memset(buf,0,volume->cluster_bytes);
                // create dir entry . and .. if create ditectory
                
                if(flag&FAT16_CTL_DIR){  
                    dirent = (FatDirEntry*) buf;
                    memset((char*)dirent,0,sizeof(FatDirEntry));
                    dirent->attr = ENT_ATTR_DIR;
                    fat16_filename_fmt(dirent->filename,".");
                    dirent->start_cluster = new_cluster;
                    
                    ++dirent;
                    memset((char*)dirent,0,sizeof(FatDirEntry));
                    dirent->attr = ENT_ATTR_DIR;
                    fat16_filename_fmt(dirent->filename,"..");
                    dirent->start_cluster = parent_cluster;
                }
                

                // it not directory , just create an empty file
                fat_write_cluster(volume,buf,new_cluster,1);
                
                return 1;
            }
            ++dirent;
        }
        
        parent_cluster = parent->current_cluster;

    }
    return 0;
}


// path format : /dir/dir/dir/filename , filename without extenstion
Fat16File * fat16_find(Fat16File* file,char *path){
    
    if(string_equal("/",path)){
        *(file->dirent) = file->volume->root;
        file->current_cluster = FAT_CLUS_ROOT;
        file->dirent_cluster = 0xFF;
        file->dirent_sector  = 0xFF;
        file->flag |= FAT16_CTL_IS_ROOT;
        return file;
    }

    char seg[16] = {0};
    char *next = path;

    *(file->dirent) = file->volume->root;
    
    do{
        next    = strtok(next,"/",seg);
        if(!*seg){
            break;
        }
        if( !(file=fat_search_file(file,seg,file->dirent->start_cluster)) ){
            return NULL;
        }
    }while(next);
    return file;
}

