#@author Erik Edwards
#@date 2019-2020

#util is my own library of utility functions in C++.
#This is the makefile for the command-line tools in C++.

#This is a collection of my own programs to do basic support, conversion, and selection tasks.
#These are the only functions written in pure C++ and that support all data types.

SHELL=/bin/bash

ss=bin/srci2src

CC=clang++

ifeq ($(CC),clang++)
	STD=-std=c++11
	WFLAG=-Weverything -Wno-c++98-compat -Wno-padded -Wno-old-style-cast -Wno-gnu-imaginary-constant
else
	STD=-std=gnu++14
	WFLAG=-Wall -Wextra
endif

CFLAGS=$(WFLAG) -O3 $(STD) -march=native -Ic


utils: CLI_gen Convert Select

#CLI_gen: for generating command-line programs, i.e. converting from shorter srci files to full src files
CLI_gen: srci2src
srci2src: src/srci2src.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Convert: for converting between file types
Convert: raw2bin bin2bin bin2txt txt2bin kaldi2bin
#raw2bin: srci/raw2bin.cpp; $(ss) srci/$@.cpp > src/$@.cpp; #then delete read input header section
raw2bin: src/raw2bin.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
#bin2bin: srci/bin2bin.cpp; $(ss) srci/$@.cpp > src/$@.cpp; #then delete
bin2bin: src/bin2bin.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
bin2txt: srci/bin2txt.cpp
	$(ss) -O srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
txt2bin: src/txt2bin.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
kaldi2bin: src/kaldi2bin.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Select: for sub-matrix access
Select: col row slice hyperslice cols rows slices hyperslices sel
col: srci/col.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
row: srci/row.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
slice: srci/slice.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
hyperslice: srci/hyperslice.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
cols: srci/cols.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
rows: srci/rows.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
slices: srci/slices.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
hyperslices: srci/hyperslices.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
sel: srci/sel.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


clean:
	find ./obj -type f -name *.o | xargs rm -f
	rm -f 7
