//Includes
#include <cmath>

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u};
const size_t I = 1u, O = 1u;

//Description
string descr;
descr += "<cmath> function, signbit.\n";
descr += "\n";
descr += "Checks each element of X if signbit is negative.\n";
descr += "Output is 1 (true) if negative and 0 (false) if positive.\n";
descr += "Complex and integer data types are not supported.\n";
descr += "\n";
descr += "Output (Y) has the same size as X with int8_t data type (since boolean).\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ signbit X -o Y \n";
descr += "$ signbit X > Y \n";
descr += "$ cat X | signbit > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks

//Set output header
o1.F = i1.F; o1.T = 10u;
o1.R = i1.R; o1.C = i1.C; o1.S = i1.S; o1.H = i1.H;

//Other prep

//Process
if (i1.T==1u)
{
    valarray<float> X(i1.N()); valarray<int8_t> Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    try { transform(begin(X),end(X),begin(Y),[](float x){return signbit(x);}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem during call to function" << endl; return 1; }
    if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
}

//Finish
