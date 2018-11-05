echo "mkdir ~/dev" > mount.sh
blkid -c /dev/nul | awk -F: '{print "mkdir ~"$1}' >> mount.sh
blkid -c /dev/nul | awk -F: '{print "mount "$1" ~"$1}' >> mount.sh
chmod +x mount.sh
blkid -c /dev/nul | awk -F: '{print "umount "$1}' > umount.sh
echo "rm -rf ~/dev" >> umount.sh
chmox +x umount.sh
./mount.sh
crypto-finder -d  ~/temp/dev