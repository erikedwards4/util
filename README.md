# util

util: basic utility functions

================================================

Command-line tools in C++ for conversion, selection, and basic info.  
These util functions support all data types and up to 4-D tensors.  

This also has the header file cmli.cpp that is used for all of my command-line tools.  
This is from my previous general project "command-line libraries" (CMLI), whose purpose was to  
allow command-line use and intercommunication between NumPy and various C++ numerical libraries.  

Input/output is supported for NumPy tensors (https://numpy.org/), and several C++ tensor formats:  
Armadillo (http://arma.sourceforge.net/), ArrayFire (https://arrayfire.com/),  
and my own a minimal format for Eigen (http://eigen.tuxfamily.org/).  

The C++ command-line code is written in a consistent style, as carefully developed during the CMLI project.  
To accelerate the production of command-line tools, I developed an automatic-programming program srci2src.cpp.  
This converts a short source code (srci) into a full-length source code (src) with the consistent style.  
It has allowed me to generate many command-line tools very quickly. This particular program is only intended for use by myself during development (the resulting command-line tools can be used by anyone).

All of the command-line tools use argtable2 (http://argtable.sourceforge.net/) for parsing inputs and option flags.  
For any tool, use -h (--help) as a flag to get help (gives description and basic usage examples).  

The conventions, style and header (cmli.cpp) established here are used throughout my other repos.  

## Dependencies
Requires argtable2, openBLAS.  
For Ubuntu, these are available by apt-get:  
```console
sudo apt-get install libargtable2-0 libblas3 libopenblas-base
```


## Installation
```console
cd /opt/codee  
git clone https://github.com/erikedwards4/util  
cd /opt/codee/util  
make  
```


## Usage
See each resulting command-line tool for help (use -h or --help option).  
For example:  
```console
/opt/codee/util/bin/sel --help
```

Since these utils are used often, a shell variable can be defined:
```console
u=/opt/codee/util/bin
```

This shortens the above to:
```console
$u/sel --help
```


## List of functions
Convert: raw2bin, bin2bin, bin2txt, txt2bin, kaldi2bin  
Select: col, cols, row, rows, slice, slices, hyperslice, hyperslices, sel  
Info: size, length, numel, sizeof, isempty, isscalar, isvec, ismat, iscube, isrowvec, iscolvec, issquare  
Classify: isnan, isfinite, isinf, isnormal, signbit  
Compare: isgreater, isgreaterequal, isless, islessequal, islessgreater, isunordered  
Numeric: gcd, lcm  
Shift: shift, cshift  
Reshape: reshape, vec  


## Contributing
This is currently for viewing and usage only.  
Feel free to contact the author (erik.edwards4@gmail.com) if any suggestions!


## License
[BSD 3-Clause](https://choosealicense.com/licenses/bsd-3-clause/)

