#@author Erik Edwards
#@date 2018-present
#@license BSD 3-clause

#util is my own library of utility functions in C++.

#These are command-line tools to do basic support, conversion, and selection tasks.
#The programs are written in pure C++ and support many data types and tensors up to 4D.

SHELL=/bin/bash
ss=bin/srci2src
CC=clang++

ifeq ($(CC),clang++)
	STD=-std=c++17
	WFLAG=-Weverything -Wno-c++98-compat -Wno-old-style-cast -Wno-gnu-imaginary-constant
else
	STD=-std=gnu++17
	WFLAG=-Wall -Wextra
endif

INCLS=-Ic -I../util
CFLAGS=$(WFLAG) $(STD) -O3 -march=native $(INCLS)


All: all
all: Dirs CLI_gen Convert Select Info Classify Compare Numeric Shift Clean

Dirs:
	mkdir -pm 777 bin obj


#CLI_gen: for generating command-line programs, i.e. converting from shorter srci files to full src files
CLI_gen: srci2src
srci2src: src/srci2src.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Convert: for converting between file types
Convert: raw2bin bin2bin bin2txt txt2bin kaldi2bin
#raw2bin: srci/raw2bin.cpp; $(ss) srci/$@.cpp > src/$@.cpp; #then delete read input header section
raw2bin: src/raw2bin.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
#bin2bin: srci/bin2bin.cpp; $(ss) srci/$@.cpp > src/$@.cpp; #then delete sections
bin2bin: src/bin2bin.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
bin2txt: srci/bin2txt.cpp
	$(ss) -O srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
txt2bin: src/txt2bin.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
kaldi2bin: src/kaldi2bin.cpp
	$(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Select: for sub-matrix access
Select: col cols row rows slice slices hyperslice hyperslices sel
col: srci/col.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
cols: srci/cols.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
row: srci/row.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
rows: srci/rows.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
slice: srci/slice.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
slices: srci/slices.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
hyperslice: srci/hyperslice.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
hyperslices: srci/hyperslices.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
sel: srci/sel.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Info: obtain info about matrix size and shape (directly from header!)
Info: size length numel sizeof isempty isscalar isvec ismat iscube isrowvec iscolvec issquare
size: srci/size.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
length: srci/length.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
numel: srci/numel.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
nbytes: srci/nbytes.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
sizeof: srci/sizeof.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isempty: srci/isempty.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isscalar: srci/isscalar.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isvec: srci/isvec.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
ismat: srci/ismat.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
iscube: srci/iscube.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isrowvec: srci/isrowvec.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
iscolvec: srci/iscolvec.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
issquare: srci/issquare.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Classify: 1 input, 1 output elementwise (returns int8_t datatype since boolean)
Classify: isnan isfinite isinf isnormal signbit
isnan: srci/isnan.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isfinite: srci/isfinite.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isinf: srci/isinf.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isnormal: srci/isnormal.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
signbit: srci/signbit.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Compare: 2 inputs, 1 output elementwise with broadcasting (returns int8_t datatype since boolean)
Compare: isgreater isgreaterequal isless islessequal islessgreater isunordered
isgreater: srci/isgreater.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isgreaterequal: srci/isgreaterequal.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isless: srci/isless.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
islessequal: srci/islessequal.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
islessgreater: srci/islessgreater.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
isunordered: srci/isunordered.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Numeric: 2 inputs, 1 output elementwise with broadcasting (only integer datatypes)
Numeric: gcd lcm
gcd: srci/gcd.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
lcm: srci/lcm.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Shift: 1 input, 1 output, shifts elements in flattened (global) order
Shift: shift cshift
shift: srci/shift.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
cshift: srci/cshift.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#Reshape: 1 input, 1 output with same total size, but different shape (by hdr change only!)
Reshape: reshape vec
reshape: srci/reshape.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2
vec: srci/vec.cpp
	$(ss) srci/$@.cpp > src/$@.cpp; $(CC) -c src/$@.cpp -oobj/$@.o $(CFLAGS); $(CC) obj/$@.o -obin/$@ -largtable2


#make clean
Clean: clean
clean:
	find ./obj -type f -name *.o | xargs rm -f
	rm -f 7 X* Y* x* y*
