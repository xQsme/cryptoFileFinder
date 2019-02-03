echo "mkdir ~/dev" > mount.sh
blkid -c /dev/nul | awk -F: '{print "mkdir ~"$1}' >> mount.sh
blkid -c /dev/nul | awk -F: '{print "mount -r "$1" ~"$1}' >> mount.sh
chmod +x mount.sh
blkid -c /dev/nul | awk -F: '{print "umount "$1}' > umount.sh
blkid -c /dev/nul | awk -F: '{print "rm ~"$1}' >> umount.sh
echo "rm ~/dev" >> umount.sh
chmod +x umount.sh
./mount.sh