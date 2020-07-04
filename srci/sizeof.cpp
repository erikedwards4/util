//Includes

//Declarations
const valarray<uint8_t> oktypes = {1,2,3,8,9,16,17,32,33,64,65,101,102,103};
const size_t I = 1, O = 1;
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
o1.T = (sizeof(size_t)==32) ? 33 : 65;
o1.R = o1.C = o1.S = o1.H = 1u;

//Other prep

//Process
if (i1.T==1)
{
    y = sizeof(float);
}
else if (i1.T==8)
{
    y = sizeof(int8_t);
}
else if (i1.T==101)
{
    y = 2*sizeof(float);
}

//Finish
if (wo1)
{
    try { ofs1.write(reinterpret_cast<char*>(&y),o1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
}
