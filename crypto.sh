blkid -c /dev/nul | awk -F: '{print "mkdir "$1}' > mount.sh
blkid -c /dev/nul | awk -F: '{print "mount "$1" "$1}' >> mount.sh
chmod +x mount.sh
./mount.sh