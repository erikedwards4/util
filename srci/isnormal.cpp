//Includes
#include <cmath>
#include <complex>

//Declarations
const valarray<uint8_t> oktypes = {1,2,3,101,102,103};
const size_t I = 1, O = 1;

//Description
string descr;
descr += "<cmath> function, isnormal.\n";
descr += "\n";
descr += "Checks each element of X if is normal (not in {nan,inf,0,subnormal}).\n";
descr += "For complex input, output is true if both real and imag part is normal.\n";
descr += "Integer data types are not supported.\n";
descr += "\n";
descr += "Output (Y) has the same size as X with int8_t data type (since boolean).\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ isnormal X -o Y \n";
descr += "$ isnormal X > Y \n";
descr += "$ cat X | isnormal > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks

//Set output header
o1.F = i1.F; o1.T = 8;
o1.R = i1.R; o1.C = i1.C; o1.S = i1.S; o1.H = i1.H;

//Other prep

//Process
if (i1.T==1)
{
    valarray<float> X(i1.N()); valarray<int8_t> Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    try { transform(begin(X),end(X),begin(Y),[](float x){return isnormal(x);}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem during call to function" << endl; return 1; }
    if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
}
else if (i1.T==101)
{
    valarray<complex <float>> X(i1.N()); valarray<int8_t> Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    try { transform(begin(X),end(X),begin(Y),[](complex<float> x){return (isnormal(x.real()) && isnormal(x.imag()));}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem during call to function" << endl; return 1; }
    if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
}

//Finish
