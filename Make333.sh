#!/bin/bash
set -x

if [ -d "/tmp/cse333-18su-ghye" ]; then
	cd /tmp/cse333-18su-ghye
	git stash
	git pull
else
	git clone git@gitlab.cs.washington.edu:cse333-18su-students/cse333-18su-ghye.git /tmp/cse333-18su-ghye
fi
chmod o-r /tmp/cse333-18su-ghye
chmod g-r /tmp/cse333-18su-ghye
cd /tmp/cse333-18su-ghye
cd ./hw0
make
cd ../hw1
make
cd ../hw2
make
cd ../hw3
make
cd ../hw4
make
