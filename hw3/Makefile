# Copyright ©2018 Hal Perkins.  All rights reserved.  Permission is
# hereby granted to students registered for University of Washington
# CSE 333 for use solely during Summer Quarter 2018 for purposes of
# the course.  No other use, copying, distribution, or modification
# is permitted without prior written consent. Copyrights for
# third-party components of this work must be honored.  Instructors
# interested in reusing these course materials should contact the
# author.

AR = ar
ARFLAGS = rcs
CC = gcc
CXX = g++
CFLAGS = -g -Wall -Wpedantic -std=c++11 -I. -I..
LDFLAGS = -L./libhw2 -lhw2 -L./libhw1/ -lhw1
HEADERS =   fileindexutil.h \
            fileindexwriter.h \
            FileIndexReader.h \
            HashTableReader.h \
            DocTableReader.h \
            IndexTableReader.h \
            DocIDTableReader.h \
            QueryProcessor.h filelayout.h

              

OBJS = fileindexutil.o fileindexwriter.o FileIndexReader.o \
       IndexTableReader.o DocTableReader.o HashTableReader.o \
       DocIDTableReader.o QueryProcessor.o

all: buildfileindex filesearchshell libhw3.a

filesearchshell: filesearchshell.o libhw3.a $(HEADERS)
	$(CXX) $(CFLAGS) -o filesearchshell filesearchshell.o \
	-L. -lhw3 $(LDFLAGS)

buildfileindex: buildfileindex.o libhw3.a $(HEADERS)
	$(CXX) $(CFLAGS) -o buildfileindex buildfileindex.o \
	-L. -lhw3 $(LDFLAGS)

dumpfileindex: dumpfileindex.o libhw3.a $(HEADERS)
	$(CXX) $(CFLAGS) -o dumpfileindex dumpfileindex.o \
	-L. -lhw3 $(LDFLAGS)

libhw3.a: $(OBJS) $(HEADERS)
	$(AR) $(ARFLAGS) libhw3.a $(OBJS)

%.o: %.cc $(HEADERS)
	$(CXX) $(CFLAGS) -c $<

clean: FORCE
	/bin/rm -f *.o *~ libhw3.a buildfileindex \
        dumpfileindex filesearchshell

FORCE:
