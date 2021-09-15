if [ "$1" = "" ];then
	echo "error! empty filanme"
	exit
fi
echo "transfer file to com1"
bytes="$( cat $1 | wc -c )"
printf "$bytes\0"  > com1.in
# dd if=$1  of=com1.in bs=1
cat $1 > com1.in
echo "end"

