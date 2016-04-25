# twfs
twfs is "Twitter Filesystem".
You can operate Twitter through file system operations on the Twitter File System.
The Twitter File System is Linux application based on FUSE(Filesystem in USErspace).

## Starting twfs
Refer to the [Twitter Filesystem](http://softwaretechnique.jp/DownLoad/twfs_en.html).

## Development Environment for twfs
To compile the twfs, you may install clang, fuse and openssl development libraries.
The following installation descriptions are assumed Ubuntu.

### Install clang development environment
Execute the following command on your termianl.

`$ sudo apt-get install clang`

### Install fuse development environment
Execute the following command on your termianl.

`$ sudo apt-get install libfuse-dev`

### Install openssl development environment
Execute the following command on your termianl.

`$ sudo apt-get install libssl-dev`

## Compile
Download the sources and then you can just make!

`$ make`
