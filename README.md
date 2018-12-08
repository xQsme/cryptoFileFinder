# cryptoFileFinder

Requires qt5

The point of this program is to detect encrypted files within a system you don't have access to, so having a linux installation in a USB stick would be the typical usage scenario.

Boot from USB, login as root and then:

```
git clone https://github.com/xQsme/cryptoFileFinder.git
cd cryptoFileFinder/release
./cryptoFinder
```

Usage:
```
-m   --mnt      --mount       Mount all partitions at "~/dev"
-u   --umount   --unmount     Unmount all partitions at "~/dev" 
-d   --dir      --directory   Directory to search ("~/dev" by default)
-s   --search                 Search for encrypted files
-o   --output                 Output file ("output.txt" by default)
-t   --test     --testing     Generate csv test data (statistical values for every file)
```

Common Usage:
```
./cryptoFinder -m -u -s
```
