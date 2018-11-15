# cryptoFileFinder

Requires qt5

Boot from USB, login as root and then:

```
git clone https://github.com/xQsme/cryptoFileFinder.git
cd cryptoFileFinder/release
./cryptoFinder
```

Usage:
```
-m   --mount               Mount all partitions at "~/dev"
-d   --dir   --directory   Directory to search ("~/dev" by default)
-s   --search              Search for encrypted files
-o   --output              Output file ("output.txt" by default)
-f   --fast                Read only a certain number of bytes from each file
-b   --bytes               Number of bytes to read when using the fast option (512 by default)
```

Common Usage:
```
./cryptoFinder -m -f -s
```
