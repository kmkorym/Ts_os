#!/bin/bash

# if invoke by image.sh
# environment variables are set 
# MOUNT_PATH

echo "build image on $MOUNT_PATH"
mkdir $MOUNT_PATH/usr
echo 1234 > $MOUNT_PATH/1234.txt
echo abcd > $MOUNT_PATH/usr/abcd.txt
cp kernel/main.c $MOUNT_PATH
cp kernel/main.c $MOUNT_PATH/usr

# create directory > 64 entry for test delete file will shrink ditectory

mkdir  $MOUNT_PATH/trash

for i in $(seq 1 66); do 
    echo "I am $i th file for deleted" > $MOUNT_PATH/trash/$i.txt
done

sync




