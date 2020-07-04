//Includes
#include <complex>

//Declarations
const valarray<uint8_t> oktypes = {1,2,3,8,9,16,17,32,33,64,65,101,102,103};
const size_t I = 2, O = 1;
size_t r, ro;
valarray<size_t> X2;

//Description
string descr;
descr += "Gets rows of X using array of row indices.\n";
descr += "\n";
descr += "The 2nd input is I, the file with the row indices.\n";
descr += "This must be in binary format with size_t data type,\n";
descr += "e.g., echo '0 2 3 7' | txt2bin -f33 > I \n";
descr += "\n";
descr += "The output Y will have rows in the same order as X.\n";
descr += "That is, I is effectively sorted to ascending order.\n";
descr += "This is unlike cols, so cannot be used to reorder rows.\n";
descr += "However, indices can be repeated to repeat rows in the output.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ rows X I -o Y \n";
descr += "$ rows X I > Y \n";
descr += "$ cat I | rows X > Y \n";
descr += "$ cat X | rows - I > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input files (X,X2)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks
if (i2.T!=33) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (I) data type must be 33 (size_t)" << endl; return 1; }
if (!i2.isvec()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (I) must be a vector of indices" << endl; return 1; }

//Set output header info
o1.F = i1.F; o1.T = i1.T;
o1.R = i2.N(); o1.C = i1.C; o1.S = i1.S; o1.H = i1.H;

//Other prep
try { X2.resize(i2.N(),0u); }
catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input 2 (I)" << endl; return 1; }
try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (I)" << endl; return 1; }
if ((X2>=i1.R).sum()!=0) { cerr << progstr+": " << __LINE__ << errstr << "row indices must be < nrows X" << endl; return 1; }

//Process
if (i1.T==1)
{
    valarray<float> X(i1.N()), Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
    for (ro=0u; ro<o1.R; ro++)
    {
        r = X2[ro];
        if (i1.iscolmajor())
        {
            //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
            try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
            try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
    }
    if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
    }
}

//Finish
