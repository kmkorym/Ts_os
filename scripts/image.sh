#!/bin/bash

# this script is for build disk image 

image_name=''
image_size=20M
image_init_script=""`dirname $(realpath $0)`"""/init_img.sh"
mount_path="./fat16"
dev=''


function parse_args()
{
    local arg
    while [ $# -gt 0 ] ; 
    do
        arg=$1
        case "$arg" in
            gen)
                action=gen
                shift
                ;;
            umnt)
                action=umnt
                shift
                ;;
            mnt)
                action=mnt
                shift
                ;;
            reload)
                action=reload
                shift
                ;;
            -s)
                image_size=$2
                shift; shift;
                ;;
            -d)
                dev=$2
                shift; shift;
                ;;
            -n)
                image_name=$2
                shift; shift;
                ;;
            -m)
                mount_path=$2
                shift; shift;
                ;;
            *)
                echo "argument error [$arg]"
                exit
        esac
    done
}

function cleanup(){

#losetup  -a | grep fat16 | sed -n -e 's/^\(\/dev\/loop[0-9][0-9]*\).*$/\1/p' |  xargs losetup -d

	return

}



function gen_image_disk(){

	if [ "$image_name" = ''  ];then
		echo  "image_name is required"	
		return 255
	fi

	umount_image_disk 


	local bytes="$( echo $image_size | sed -n -E 's/([0-9]+)([M|K])/\1/p')"
	local unit="$( echo $image_size | sed -n -E 's/([0-9]+)([M|K])/\2/p')"
	
	case "$unit" in
		M)
			bytes=$(( bytes*1024*1024 ))
		;;
		K)
			bytes=$(( bytes*1024))
		;;
	esac
	
	

	rm -f $image_name
	dd if=/dev/zero of=$image_name bs=512 count=$((bytes/512))
	chmod a+rwx $image_name

	if [ "$dev" = "" ];then
		dev=$(losetup -f)
	else
		losetup -d $dev
	fi
	
	losetup $dev $image_name
	mkfs.fat -F 16 $dev
	mount $dev  $mount_path
	sync
	MOUNT_PATH=$mount_path bash $image_init_script
	
}

function mount_image_disk(){

	if [ "$image_name" = ''  ];then
		echo  "image_name is required"	
		return 255;
	fi

	if [ "$dev" = "" ];then
		dev=$(losetup -f)
	else
		losetup -d $dev	
	fi
	
	
	losetup $dev $image_name
	mount $dev  $mount_path
	#sudo hexdump -v -e '"%02_ad   " 10/1  "%02X ""\n"' /dev/loop87 | head -n 100
}


function umount_image_disk(){
	if [ $image_name = "" ];then
		echo  "image_name is required"	
		return 255;
	fi
	losetup  -a | grep $image_name | sed -n -e 's/^\(\/dev\/loop[0-9][0-9]*\).*$/\1/p' |  xargs losetup -d
	umount $mount_path
	if [ $? -ne 0 ];then
		return 254
	fi

}

function reload_image_disk(){
	#losetup -d 
	umount_image_disk
	mount_image_disk 
}


parse_args "$@"

case "$action" in
	gen)
		gen_image_disk
	;;

	umnt)
		umount_image_disk
	;;
	mnt)
		mount_image_disk
	;;
	reload)
		reload_image_disk
	;;
	*)
	    echo "unknown action [$action]"
        exit	
	;;
esac
