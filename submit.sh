#!/bin/bash

# clean binaries
make clean

# remove a.out
rm -f a.out

# create a tar by excluding the git folder 
tar cjvf ../hw.tar.gz . --exclude=".git"

echo "tarball created successfully"
