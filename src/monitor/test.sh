#!/usr/bin/bash

# Tests for inotify.c

CURDIR=$(pwd)
#gcc inotify.c -o a.out
#./a.out $CURDIR


mkdir $CURDIR/tempdir
touch $CURDIR/tempdir
rmdir $CURDIR/tempdir

echo "sample" > $CURDIR/sample.txt
chmod -w $CURDIR/sample.txt
rm -f $CURDIR/sample.txt

