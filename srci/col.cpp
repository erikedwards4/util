//Includes
#include <complex>

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 1u, O = 1u;
size_t c;

//Description
string descr;
descr += "Gets a single column of X.\n";
descr += "\n";
descr += "Use -c (--col) to specify the column number [default=0].\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ col -c2 X -o Y \n";
descr += "$ col -c2 X > Y \n";
descr += "$ cat X | col -c2 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int    *a_c = arg_intn("c","col","<uint>",0,1,"column number to get [default=0]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get c
if (a_c->count==0) { c = 0u; }
else if (a_c->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "c must be nonnegative" << endl; return 1; }
else { c = size_t(a_c->ival[0]); }
if (c>=i1.C) { cerr << progstr+": " << __LINE__ << errstr << "c must be int in [0 C-1]" << endl; return 1; }

//Checks

//Set output header info
o1.F = i1.F; o1.T = i1.T;
o1.R = i1.R; o1.C = 1u;
o1.S = i1.S; o1.H = i1.H;

//Other prep

//Process
if (i1.T==1u)
{
    valarray<float> X(i1.N()), Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
    if (i1.iscolmajor())
    {
        //try { Y = X[gslice(c*i1.R,{i1.S,i1.R},{i1.R*i1.C,1})]; }
        try { Y = X[gslice(c*i1.R,{i1.H,i1.S,i1.R},{i1.S*i1.C*i1.R,i1.C*i1.R,1})]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
    }
    else
    {
        //try { Y = X[gslice(c*i1.S,{i1.R,i1.S},{i1.C*i1.S,1})]; }
        try { Y = X[gslice(c*i1.S*i1.H,{i1.R,i1.S,i1.H},{i1.C*i1.S*i1.H,i1.H,1})]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
    }
    if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
    }
}

//Finish
