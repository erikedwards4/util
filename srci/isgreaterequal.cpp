//Includes
#include <cmath>
#include <complex>

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 2u, O = 1u;
size_t ri1, ri2, ci1, ci2, si1, si2, hi1, hi2;
gslice GS1, GS2;

//Description
string descr;
descr += "<cmath> function, isgreaterequal.\n";
descr += "\n";
descr += "Checks if X1>=X2, element-wise.\n";
descr += "For complex inputs, checks if |X1|>=|X2|, element-wise.\n";
descr += "\n";
descr += "X1 and X2 must have the same data type.\n";
descr += "X1 and X2 must have the same size or broadcast-compatible sizes.\n";
descr += "\n";
descr += "Output (Y) has int8_t data type (since boolean).\n";
descr += "Output (Y) has size max(R1,R2) x max(C1,C2).\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ isgreaterequal X1 X2 -o Y \n";
descr += "$ isgreaterequal X1 X2 > Y \n";
descr += "$ cat X2 | isgreaterequal X1 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input files (X1,X2)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks
if (i2.iscolmajor()!=i1.iscolmajor()) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have the same row/col major format" << endl; return 1; }
if (i2.T!=i1.T) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have the same data type" << endl; return 1; }
if (!bcast_compat(i1,i2)) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have same size or broadcast-compatible sizes" << endl; return 1; }

//Set output header
o1.F = i1.F; o1.T = 10u;
o1.R = (i1.R>i2.R) ? i1.R : i2.R;
o1.C = (i1.C>i2.C) ? i1.C : i2.C;
o1.S = (i1.S>i2.S) ? i1.S : i2.S;
o1.H = (i1.H>i2.H) ? i1.H : i2.H;

//Other prep
ri1 = !(i1.R==1u && o1.R>1u); ci1 = !(i1.C==1u && o1.C>1u); si1 = !(i1.S==1u && o1.S>1u); hi1 = !(i1.H==1u && o1.H>1u);
ri2 = !(i2.R==1u && o1.R>1u); ci2 = !(i2.C==1u && o1.C>1u); si2 = !(i2.S==1u && o1.S>1u); hi2 = !(i2.H==1u && o1.H>1u);
if (o1.iscolmajor())
{
    try { GS1 = gslice(0,{o1.H,o1.S,o1.C,o1.R},{hi1*i1.R*i1.C*i1.S,si1*i1.R*i1.C,ci1*i1.R,ri1}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem making gslice for input 1" << endl; return 1; }
    try { GS2 = gslice(0,{o1.H,o1.S,o1.C,o1.R},{hi2*i2.R*i2.C*i2.S,si2*i2.R*i2.C,ci2*i2.R,ri2}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem making gslice for input 2" << endl; return 1; }
}
else
{
    try { GS1 = gslice(0,{o1.R,o1.C,o1.S,o1.H},{ri1*i1.C*i1.S*i1.H,ci1*i1.S*i1.H,si1*i1.H,hi1}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem making gslice for input 1" << endl; return 1; }
    try { GS2 = gslice(0,{o1.R,o1.C,o1.S,o1.H},{ri2*i2.C*i2.S*i2.H,ci2*i2.S*i2.H,si2*i2.H,hi2}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem making gslice for input 2" << endl; return 1; }
}

//Process
if (i1.T==1u)
{
    valarray<float> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
    try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
    try { X1 = valarray<float>(X1[GS1]); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
    try { transform (begin(X1),end(X1),begin(valarray<float>(X2[GS2])),begin(Y),[](float x1,float x2){return isgreaterequal(x1,x2);}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
    if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
}
else if (i1.T==101u)
{
    valarray<complex<float>> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
    try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
    try { X1 = valarray<complex<float>>(X1[GS1]); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
    try { transform (begin(X1),end(X1),begin(valarray<complex<float>>(X2[GS2])),begin(Y),[](complex<float> x1,complex<float> x2){return isgreaterequal(abs(x1),abs(x2));}); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
    if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
}

//Finish
