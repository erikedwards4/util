//Includes
#include <complex>

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 2u, O = 1u;
size_t c;
valarray<size_t> X2;

//Description
string descr;
descr += "Gets columns of X using array of column indices.\n";
descr += "\n";
descr += "The 2nd input is I, the file with the col indices.\n";
descr += "This must be in binary format with size_t data type,\n";
descr += "e.g., echo '0 2 3 7' | txt2bin -f33 > I \n";
descr += "\n";
descr += "This can also be used to repeat or reorder the cols of X,\n";
descr += "by using an I with col indices repeated or reordered.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ cols X I -o Y \n";
descr += "$ cols X I > Y \n";
descr += "$ cat I | cols X > Y \n";
descr += "$ cat X | cols - I > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input files (X,X2)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks
if (i2.T!=33u && i2.T!=65u) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (I) data type must be 33 (size_t)" << endl; return 1; }
if (!i2.isvec()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (I) must be a vector of indices" << endl; return 1; }

//Set output header info
o1.F = i1.F; o1.T = i1.T;
o1.R = i1.R; o1.C = i2.N();
o1.S = i1.S; o1.H = i1.H;

//Other prep
try { X2.resize(i2.N(),0u); }
catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input 2 (I)" << endl; return 1; }
try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (I)" << endl; return 1; }
if ((X2>=i1.C).sum()!=0) { cerr << progstr+": " << __LINE__ << errstr << "col indices must be < ncols X" << endl; return 1; }

//Process
if (i1.T==1u)
{
    valarray<float> X(i1.N()), Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
    for (size_t co=0u; co<o1.C; ++co)
    {
        c = X2[co];
        if (i1.iscolmajor())
        {
            try { Y[gslice(co*o1.R,{o1.H,o1.S,o1.R},{o1.R*o1.C*o1.S,o1.R*o1.C,1})] = X[gslice(c*i1.R,{i1.H,i1.S,i1.R},{i1.R*i1.C*i1.S,i1.R*i1.C,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            try { Y[gslice(co*o1.S*o1.H,{o1.R,o1.S,o1.H},{o1.C*o1.S*o1.H,o1.H,1})] = X[gslice(c*i1.S*i1.H,{i1.R,i1.S,i1.H},{i1.C*i1.S*i1.H,i1.H,1})]; }
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
