//Includes

//Declarations
const valarray<uint8_t> oktypes = {1,2,8,9,16,17,32,33,64,65,101,102};
const size_t I = 1, O = 1;

//Description
string descr;
descr += "Utility to take a raw binary data input and add a header.\n";
descr += "The -f (--fmt) option controls which type of header is added.\n";
descr += "The content of the header is determined by all of the options.\n";
descr += "\n";
descr += "Use -r, -c, -s, -y to specify the output size in number of\n";
descr += "rows, columns, slices, and hyperslices, respectively.\n";
descr += "\n";
descr += "Use -t (--type) to specify data type [default=2 -> double].\n";
descr += "\n";
descr += "Use -f (--fmt) to specify file format [default=102 -> col major].\n";
descr += "\n";
descr += "The user must know all of these in advance! No checks are done here.\n";
descr += "This program gives undefined output or may crash if incorrect params.\n";
descr += "The purpose is to give some way to get into the CMLI system from an \n";
descr += "arbitrary binary format, so long as the user knows the data parameters.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ raw2bin -r2 -c3 -o Y \n";
descr += "$ raw2bin -r2 -c3 > Y \n";
descr += "$ raw2bin -r2 -c3 -s2 -t8 -f101 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int   *a_nr = arg_intn("r","n_rows","<uint>",0,1,"num rows in output [default=1]");
struct arg_int   *a_nc = arg_intn("c","n_cols","<uint>",0,1,"num cols in output [default=1]");
struct arg_int   *a_ns = arg_intn("s","n_slices","<uint>",0,1,"num slices in output [default=1]");
struct arg_int   *a_nh = arg_intn("y","n_hyperslices","<uint>",0,1,"num hyperslices in output [default=1]");
struct arg_int *a_otyp = arg_intn("t","type","<uint>",0,1,"data type [default=2]");
struct arg_int *a_ofmt = arg_intn("f","fmt","<uint>",0,1,"file format [default=102]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get o1.F
if (a_ofmt->count==0) { o1.F = 147; }
else if (a_ofmt->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be nonnegative" << endl; return 1; }
else if (a_ofmt->ival[0]>255) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be < 256" << endl; return 1; }
else { o1.F = uint8_t(a_ofmt->ival[0]); }

//Get o1.T
if (a_otyp->count==0) { o1.T = 1; }
else if (a_otyp->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "data type must be positive int" << endl; return 1; }
else { o1.T = uint8_t(a_otyp->ival[0]); }
if ((o1.T==oktypes).sum()==0)
{
    cerr << progstr+": " << __LINE__ << errstr << "output data type must be in " << "{";
    for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1]) ? "}" : ","); }
    cerr << endl; return 1;
}

//Get o1.R
if (a_nr->count==0) { o1.R = 1u; }
else if (a_nr->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "R (nrows) must be nonnegative" << endl; return 1; }
else { o1.R = uint32_t(a_nr->ival[0]); }

//Get o1.C
if (a_nc->count==0) { o1.C = 1u; }
else if (a_nc->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "C (ncols) must be nonnegative" << endl; return 1; }
else { o1.C = uint32_t(a_nc->ival[0]); }

//Get o1.S
if (a_ns->count==0) { o1.S = 1u; }
else if (a_ns->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "S (nslices) must be nonnegative" << endl; return 1; }
else { o1.S = uint32_t(a_ns->ival[0]); }

//Get o1.H
if (a_nh->count==0) { o1.H = 1u; }
else if (a_nh->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "H (nhyperslices) must be nonnegative" << endl; return 1; }
else { o1.H = uint32_t(a_nh->ival[0]); }

//Checks
if (o1.H!=1u && o1.only_3D()) { cerr << progstr+": " << __LINE__ << errstr << "4D (hypercubes) not supported for arma file format" << endl; return 1; }

//Set output header info

//Other prep
i1.F = 0; i1.T = o1.T;
i1.R = o1.N();
i1.C = i1.S = i1.H = 1u;

//Process
if (o1.T==1)
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
