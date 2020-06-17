//Includes

//Declarations
const valarray<uint8_t> oktypes = {1,2,3,8,9,16,17,32,33,64,65,101,102,103};
const size_t I = 1, O = 1;
uint32_t r;

//Description
string descr;
descr += "Gets a single row of X.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ row -r2 X -o Y \n";
descr += "$ row -r2 X > Y \n";
descr += "$ cat X | row -r2 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int    *a_r = arg_intn("r","row","<uint>",0,1,"row number to get [default=0]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get r
if (a_r->count==0) { r = 0u; }
else if (a_r->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "r must be nonnegative" << endl; return 1; }
else { r = uint32_t(a_r->ival[0]); }
if (r>=i1.R) { cerr << progstr+": " << __LINE__ << errstr << "r must be int in [0 R-1]" << endl; return 1; }

//Checks

//Set output header info
o1.F = i1.F; o1.T = i1.T;
o1.R = 1u; o1.C = i1.C; o1.S = i1.S; o1.H = i1.H;

//Other prep

//Process
if (i1.T==1)
{
    valarray<float> X(i1.N()), Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
    if (i1.iscolmajor())
    {
        //try { Y = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
        try { Y = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
    }
    else
    {
        //try { Y = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
        try { Y = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
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

