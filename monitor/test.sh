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

echo "Testing JSON Validation"

mkdir $CURDIR/temp\ dir
touch $CURDIR/temp\ dir
rmdir $CURDIR/temp\ dir

echo "sample" > $CURDIR/sample\ \".txt
rm -f $CURDIR/sample\ \".txt

