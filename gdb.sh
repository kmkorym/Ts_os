PWD=$(pwd)
qemu-system-i386 -boot order=a -fda kimage -S -s &

gdb -ex 'target remote localhost:1234' \
    -ex 'set architecture i386' \
    -ex 'symbol-file kernel.sym'\  
    -ex 'b main'\
    -ex 'c'
