//Includes
#include <complex>

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 1u, O = 1u;

//Description
string descr;
descr += "Reshape function.\n";
descr += "Reshapes input X to output Y with same total size, and \n";
descr += "with the same elements in the same global (flattened) order.\n";
descr += "\n";
descr += "Y has the same data type and file format as X.\n";
descr += "This only changes the header size info. \n";
descr += "\n";
descr += "Examples:\n";
descr += "$ reshape -r2 -c3 X -o Y \n";
descr += "$ reshape -s6 X > Y \n";
descr += "$ cat X | reshape -r3 -c2 > Y \n";
descr += "$ cat X | reshape -r2 -s3 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int   *a_nr = arg_intn("r","n_rows","<uint>",0,1,"num rows in output [default=1]");
struct arg_int   *a_nc = arg_intn("c","n_cols","<uint>",0,1,"num cols in output [default=1]");
struct arg_int   *a_ns = arg_intn("s","n_slices","<uint>",0,1,"num slices in output [default=1]");
struct arg_int   *a_nh = arg_intn("y","n_hyperslices","<uint>",0,1,"num hyperslices in output [default=1]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get o1.R
if (a_nr->count==0) { o1.R = 1u; }
else if (a_nr->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "R (nrows) must be nonnegative" << endl; return 1; }
else { o1.R = size_t(a_nr->ival[0]); }

//Get o1.C
if (a_nc->count==0) { o1.C = 1u; }
else if (a_nc->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "C (ncols) must be nonnegative" << endl; return 1; }
else { o1.C = size_t(a_nc->ival[0]); }

//Get o1.S
if (a_ns->count==0) { o1.S = 1u; }
else if (a_ns->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "S (nslices) must be nonnegative" << endl; return 1; }
else { o1.S = size_t(a_ns->ival[0]); }

//Get o1.H
if (a_nh->count==0) { o1.H = 1u; }
else if (a_nh->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "H (nhyperslices) must be nonnegative" << endl; return 1; }
else { o1.H = size_t(a_nh->ival[0]); }

//Checks
if (o1.H!=1u && o1.only_3D()) { cerr << progstr+": " << __LINE__ << errstr << "4D (hypercubes) not supported for Armadillo file format" << endl; return 1; }
if (o1.N()!=i1.N()) { cerr << progstr+": " << __LINE__ << errstr << "N (total num elements) must be same for input and output" << endl; return 1; }

//Set output header
o1.F = i1.F; o1.T = i1.T;

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
