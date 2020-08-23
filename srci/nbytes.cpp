//Includes

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,10u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 1u, O = 1u;
streamsize y;

//Description
string descr;
descr += "Gets number of bytes in X (not counting header).\n";
descr += "This should be the product of numel and sizeof.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ nbytes X -o Y \n";
descr += "$ nbytes X > Y \n";
descr += "$ cat X | nbytes > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks

//Set output header info
o1.F = i1.F;
o1.T = (sizeof(streamsize)==32u) ? 32u : 64u;
o1.R = o1.C = o1.S = o1.H = 1u;

//Other prep
y = i1.nbytes();

//Process
if (i1.T==1u)
{
}
else if (i1.T==8u)
{
}
else if (i1.T==101u)
{
}

//Finish
if (wo1)
{
    try { ofs1.write(reinterpret_cast<char*>(&y),o1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
}
