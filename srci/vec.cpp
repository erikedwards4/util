//Includes
#include <complex>

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 1u, O = 1u;
size_t dim;

//Description
string descr;
descr += "Reshape function.\n";
descr += "Makes vector from elements of X in flattened order.\n";
descr += "\n";
descr += "Use -d (--dim) to give the dimension (axis) [default=0].\n";
descr += "Y is column vector for d=0, a row vector for d=1, etc.\n";
descr += "\n";
descr += "Elements are taken from X in global (flattened) order.\n";
descr += "For col-major file format, this takes elements along cols.\n";
descr += "For row-major file format, this takes elements along rows.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ vec -d0 X -o Y \n";
descr += "$ vec -d1 X > Y \n";
descr += "$ cat X | vec -d2 > Y \n";
descr += "$ cat X | vec > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int    *a_d = arg_intn("d","dim","<uint>",0,1,"dimension [default=0]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get dim
if (a_d->count==0) { dim = 0u; }
else if (a_d->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "dim must be nonnegative" << endl; return 1; }
else { dim = size_t(a_d->ival[0]); }
if (dim>3) { cerr << progstr+": " << __LINE__ << errstr << "dim must be in {0,1,2,3u}" << endl; return 1; }

//Checks

//Set output header
o1.F = i1.F; o1.T = i1.T;
o1.R = (dim==0) ? i1.N() : 1u;
o1.C = (dim==1) ? i1.N() : 1u;
o1.S = (dim==2) ? i1.N() : 1u;
o1.H = (dim==3) ? i1.N() : 1u;

//Other prep

//Process
if (i1.T==1u)
{
    float *X;
    try { X = new float[i1.N()]; }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input file (X)" << endl; return 1; }
    try { ifs1.read(reinterpret_cast<char*>(X),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(X),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
    delete[] X;
}

//Finish
