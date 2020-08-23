//Includes

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 1u, O = 1u;
size_t y;

//Description
string descr;
descr += "Gets the number of bytes for one element of X,\n";
descr += "as would be returned by the sizeof function.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ sizeof X -o Y \n";
descr += "$ sizeof X > Y \n";
descr += "$ cat X | sizeof > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks

//Set output header info
o1.F = i1.F;
o1.T = (sizeof(size_t)==32u) ? 33u : 65u;
o1.R = o1.C = o1.S = o1.H = 1u;

//Other prep

//Process
if (i1.T==1u)
{
    y = sizeof(float);
}
else if (i1.T==8u)
{
    y = sizeof(int8_t);
}
else if (i1.T==101u)
{
    y = 2*sizeof(float);
}

//Finish
if (wo1)
{
    try { ofs1.write(reinterpret_cast<char*>(&y),o1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
}
