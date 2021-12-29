#!/bin/bash


arch='i386'
layout=src
fda=build/kimage
sym=build/kernel.sym 
disk_image_name=''
common_run_opts="-boot order=a -global ide-hd.logical_block_size=512  -global ide-hd.physical_block_size=512"
gdb_file='gdb.txt'
tdesc='l' # l = no op
do_img_backup=1


function parse_args()
{
    local arg
    while [ $# -gt 0 ] ; 
    do
        arg=$1
        case "$arg" in
            run)
                action=run
                shift
                ;;
            debug)
                action=debug
                shift
                ;;
            -arch)
                arch=$2
                shift;shift
                ;;         
            -fda)
                fda=$2
                shift;shift;
                ;;
            -hd)
                disk_image_name=$2
                shift;shift;
                ;;
            -sym)
                sym=$2
                shift;shift;
                ;;
            -tdesc)
                tdesc='set tdesc filename target.xml'
                arch=i8086
                shift;
                ;;
            *)
                echo "argument error [$arg]"
                exit
        esac
    done
}


parse_args "$@"


if [ $arch = 'i8086' ];then
    layout=asm
    tdesc='set tdesc filename target.xml'
fi

if [ $do_img_backup -eq 1 ]  && [ "$disk_image_name" != ""  ];then
    cp -f $disk_image_name $disk_image_name.bak
fi

if [ "$disk_image_name" != ""  ];then
    disk_param="-drive file=$disk_image_name,if=ide,index=0,format=raw" 
else
    disk_param=""
fi


case "$action" in
    run)
        make build
        make deploy
        #qemu-system-i386  $common_run_opts 
        qemu-system-i386  $common_run_opts $disk_param   -drive file=$fda,index=0,if=floppy,format=raw 
        shift;;
    debug)
        qemu-system-i386 $common_run_opts  $disk_param   -drive file=$fda,index=0,if=floppy,format=raw    -S -s  &
        sleep 1
        gdb -ex 'target remote localhost:1234' -x $gdb_file  -ex "$tdesc"  -ex "set architecture $arch" -ex  "symbol-file $sym" \
        -ex 'break_points'  -ex 'p_vars' -ex "layout $layout"
        exit
        ;;
    *)
        echo "unknown action : $action !"
        exit;;
esac

#set tdesc filename path

# https://stackoverflow.com/questions/32955887/how-to-disassemble-16-bit-x86-boot-sector-code-in-gdb-with-x-i-pc-it-gets-tr
# https://sourceware.org/bugzilla/show_bug.cgi?id=22869




