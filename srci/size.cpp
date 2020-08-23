//Includes

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,10u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 1u, O = 1u;
size_t Y[4];

//Description
string descr;
descr += "Gets size of input X as 4-element row vector Y.\n";
descr += "This is [R C S H], where: \n";
descr += "R is number of rows in X \n";
descr += "C is number of columns in X \n";
descr += "S is number of slices in X \n";
descr += "H is number of hyperslices in X \n";
descr += "\n";
descr += "Examples:\n";
descr += "$ size X -o Y \n";
descr += "$ size X > Y \n";
descr += "$ cat X | size > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks

//Set output header info
o1.F = i1.F;
o1.T = (sizeof(size_t)==32u) ? 33u : 65u;
o1.R = o1.S = o1.H = 1u;
o1.C = 4u;

//Other prep
Y[0] = i1.R; Y[1] = i1.C; Y[2] = i1.S; Y[3] = i1.H;

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
    try { ofs1.write(reinterpret_cast<char*>(&Y),o1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
}
