# util

util: basic utility functions

Erik Edwards (erik.edwards4@gmail.com)

================================================

This is a set command-line tools in C++ that do conversion and selection utilities.  

The command-line programs are written in C++ with a consistent style and interface.  
These util functions support all data types and up to 4-D tensors.  

This also has the all-important header file cmli.cpp.  
This is from a general project "command-line libraries" to allow intercommunication between NumPy and various C++ numerical libraries.

Input/output is supported for NumPy tensors (https://numpy.org/), and several C++ tensor formats:  
Armadillo (http://arma.sourceforge.net/),  
ArrayFire (https://arrayfire.com/),  
and my own a minimal format for Eigen (http://eigen.tuxfamily.org/).  

The C++ command-line programs are written in a consistent style that was developed for command-line tools in general.  
All of these command-line tools use argtable2 (http://argtable.sourceforge.net/) for parsing inputs and option flags.  
For any of these, use -h (--help) as a flag to get help (description and usage examples).  


## Dependencies
Requires argtable2, openBLAS.  
For Ubuntu, these are available by apt-get:  
```
sudo apt-get install libargtable2-0 libblas3 libopenblas-base
```


## Installation
```
cd /opt/codee  
git clone https://github.com/erikedwards4/util  
cd /opt/codee/util  
make  
```


## Usage
See each resulting command-line tool for help (use -h or --help option).  
For example:  
```
/opt/codee/util/bin/sel --help
```


## Contributing
This is currently for viewing and usage only.  
Feel free to contact the author (erik.edwards4@gmail.com) if any suggestions!


## License
[BSD 3-Clause](https://choosealicense.com/licenses/bsd-3-clause/)

