#!/bin/bash


arch='i386'
layout=src
fda=build/kimage
sym=build/kernel.sym 
common_run_opts="-boot order=a"
gdb_file='gdb.txt'
tdesc='l' # l = no op


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
fi

case "$action" in
    run)
        make build
        make deploy
        qemu-system-i386  $common_run_opts -drive file=$fda,index=0,if=floppy,format=raw 
        shift;;
    debug)
        qemu-system-i386 $common_run_opts -drive file=$fda,index=0,if=floppy,format=raw    -S -s  &
        sleep 1
        gdb -ex 'target remote localhost:1234' -x $gdb_file  -ex "$tdesc"  -ex "set architecture $arch" -ex  "symbol-file $sym" \
        -ex 'break_points'   -ex 'display_regs' -ex "layout $layout"
        exit
        ;;
    *)
        echo "unknown action : $action !"
        exit;;
esac

# https://stackoverflow.com/questions/32955887/how-to-disassemble-16-bit-x86-boot-sector-code-in-gdb-with-x-i-pc-it-gets-tr
# https://sourceware.org/bugzilla/show_bug.cgi?id=22869




