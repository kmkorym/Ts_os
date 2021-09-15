#!/bin/bash

# build a image for testing bootloader load is correct or not
# the format of image is  [MBR][(512-GDB_TABLE_SIZE)*  byte value = 0 ]
# [512* byte value=1][512*byte value=C]..... MOST 255 sectors (and then truncate to 1MB)

action=''
filename=''
output='a2z.bin'
mode=1

function parse_args()
{
    local arg
    while [ $# -gt 0 ] ; 
    do
        arg=$1
        case "$arg" in
            build)
                action=build
                shift
                ;;
            test)
                action=test
                shift
                ;;
            -f)
                filename=$2
                shift;shift;
                ;;
            -m)
                mode=$2
                shift;shift
                ;;
            -o)
                output=$2
                shift;shift;
                ;;
            *)
                echo "argument error [$arg]"
                exit
        esac
    done
}

function build_test_kimage(){

    local mbr_file=$1
    local output=$2
    local mbr_file_sz=$(wc -c $mbr_file   | awk  '{print $1}')
    local cyl=1
    local oc_cyl=''
    local sz=0
    cp -f  $mbr_file $output
    # boot sector
    dd if=/dev/zero  bs=$((1024-mbr_file_sz)) count=1  2>/dev/null   >> $output 
     # reset of cylinder 0 head 0
    dd if=/dev/zero  bs=512  count=35  2>/dev/null   >> $output  
    # hexdump addr + 0x48 
    while [ $cyl -lt 80  ]; do
        oc_cyl=`printf '%o' $((cyl))`
        dd if=/dev/zero bs=512 count=36   2>/dev/null  | tr  '\000'  "$(printf '\\%03d' $oc_cyl)"  >> $output
        cyl=$(( (cyl + 1) % 256))
        # terminate when > 1MB
        sz=$(wc -c $output   | awk  '{print $1}')    
        if  (( sz > 1048576 )) ;then
            break
        fi
    done
    truncate -s 1M  $output
    echo 'done'


}


parse_args "$@"

echo "action : $action"
#echo "input  : $filename"
#



case "$action" in
    build)
        build_test_kimage $filename $output
        shift
        exit
        ;;
    test)
        make build_test_mbr
        case $mode in
        1)
            ./run.sh debug    -fda build/test_mbr.bin  -arch i8086 
            ;;
        2)
            ./run.sh debug    -fda build/test_mbr.bin  -arch i8086 -tdesc   
            ;;
        *)
            echo "undefined mode $mode"
            ;;
        esac
        ;;
    *)
        echo "unknown action : $action !"
        exit;;
esac


