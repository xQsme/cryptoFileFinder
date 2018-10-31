echo "mkdir ~/temp" > mount.sh
echo "mkdir ~/temp/dev" > mount.sh
blkid -c /dev/nul | awk -F: '{print "mkdir ~/temp"$1}' >> mount.sh
blkid -c /dev/nul | awk -F: '{print "mount "$1" ~/temp"$1}' >> mount.sh
chmod +x mount.sh
./mount.sh
crypto-finder -d  ~/temp/dev