old_size=$(df /tmp/ | tail -1 | awk '{print $2}')
echo old_size: $old_size
mount -t tmptfs -o remount,size=200M /tmp

tar -xvzf /tmp/modem.tar.gz /tmp
rm /tmp/modem.tar.gz

old_size=$old_size"K"

mount -t tmpfs -o remount,size=$old_size /tmp
