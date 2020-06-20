//Includes

//Declarations
const valarray<uint8_t> oktypes = {1,2,3,8,9,10,16,17,32,33,64,65,101,102,103};
const size_t I = 1, O = 1;
bool y;

//Description
string descr;
descr += "Returns 0/1 if X is a matrix (num slices and hyperslices are 1).\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ ismat X -o Y \n";
descr += "$ ismat X > Y \n";
descr += "$ cat X | ismat > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Checks

//Set output header info
o1.F = i1.F; o1.T = 10;
o1.R = o1.C = o1.S = o1.H = 1u;

//Other prep
y = i1.ismat();

//Process
if (i1.T==1)
{
}
else if (i1.T==8)
{
}
else if (i1.T==101)
{
}

//Finish
if (wo1)
{
    try { ofs1.write(reinterpret_cast<char*>(&y),o1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
}
