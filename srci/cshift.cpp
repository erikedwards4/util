//Includes

//Declarations
const valarray<uint8_t> oktypes = {1,2,3,8,9,16,17,32,33,64,65,101,102,103};
const size_t I = 1, O = 1;
int cnt;

//Description
string descr;
descr += "valarray class member function, X.cshift(cnt).\n";
descr += "\n";
descr += "Circular shift of elements by cnt elements.\n";
descr += "Postive cnt shifts up, so that X[1] -> Y[0].\n";
descr += "\n";
descr += "Indices are in global (flattened) order.\n";
descr += "For col-major, shifts along cols, wrapping around to next col.\n";
descr += "For row-major, shifts along rows, wrapping around to next row.\n";
descr += "\n";
descr += "Output (Y) has the same size, data type and file format as X.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ cshift -n3 X -o Y \n";
descr += "$ cshift -n3 X > Y \n";
descr += "$ cat X | cshift -n3 > Y \n";
descr += "$ cat X | cshift -n-3 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int  *a_cnt = arg_intn("n","count","<uint>",0,1,"number of elements to shift by [default=0]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get cnt
cnt = (a_cnt->count>0) ? a_cnt->ival[0] : 0;

//Checks

//Set output header
o1.F = i1.F; o1.T = i1.T;
o1.R = i1.R; o1.C = i1.C; o1.S = i1.S; o1.H = i1.H;

//Other prep

//Process
if (i1.T==1)
{
    valarray<float> X(i1.N()), Y(o1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file (X)" << endl; return 1; }
    try { Y = X.cshift(cnt); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem during call to function" << endl; return 1; }
    if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
    if (wo1)
    {
        try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
    }
}

//Finish

