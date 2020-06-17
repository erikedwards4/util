//Includes

//Declarations
const valarray<uint8_t> oktypes = {1,2,3,8,9,16,17,32,33,64,65,101,102,103};
const size_t I = 1, O = 1;
gslice GS;

//Description
string descr;
descr += "Converts cmli binary file to a different data type.\n";
descr += "The input data type is obtained from the file header.\n";
descr += "\n";
descr += "Use -t (--type) to give the output data type [default=2].\n";
descr += "\n";
descr += "Use -f (--fmt) to give the output file format.\n";
descr += "This can be 1 (arrayfire), 65 (arma), 101 (cmli row major),\n";
descr += "102 (cmli col major), or 147 (npy), with default from input.\n";
descr += "\n";
descr += "Examples:\n";
descr += "$ bin2bin X -o Y \n";
descr += "$ bin2bin -f1 -t102 X > Y \n";
descr += "$ cat X | bin2bin -f1 -t102 > Y \n";

//Argtable
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_int *a_otyp = arg_intn("t","type","<uint>",0,1,"output data type [default as input]");
struct arg_int *a_ofmt = arg_intn("f","fmt","<uint>",0,1,"output file format [default as input]");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");

//Get options

//Get o1.F
if (a_ofmt->count==0) { o1.F = i1.F; }
else if (a_ofmt->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be nonnegative" << endl; return 1; }
else if (a_ofmt->ival[0]>255) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be < 256" << endl; return 1; }
else { o1.F = uint8_t(a_ofmt->ival[0]); }

//Get o1.T
if (a_otyp->count==0) { o1.T = i1.T; }
else if (a_otyp->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "data type must be positive" << endl; return 1; }
else if (a_otyp->ival[0]>103) { cerr << progstr+": " << __LINE__ << errstr << "data type must be <= 103" << endl; return 1; }
else { o1.T = uint8_t(a_otyp->ival[0]); }
if ((o1.T==oktypes).sum()==0)
{
    cerr << progstr+": " << __LINE__ << errstr << "output data type must be in " << "{";
    for (auto o : oktypes) { cerr << (int)o << ((o==oktypes[oktypes.size()-1]) ? "}" : ","); }
    cerr << endl; return 1;
}

//Checks
if (!i1.isreal() && o1.isreal()) { cerr << progstr+": " << __LINE__ << errstr << "cannot convert from complex to real data type" << endl; return 1; }

//Set output header info
o1.R = i1.R; o1.C = i1.C; o1.S = i1.S; o1.H = i1.H;

//Other prep
if (i1.iscolmajor()!=o1.iscolmajor())
{
    if (i1.iscolmajor()) { GS = gslice(0,{i1.R,i1.C,i1.S,i1.H},{1,i1.R,i1.R*i1.C,i1.R*i1.C*i1.S}); }
    else { GS = gslice(0,{i1.H,i1.S,i1.C,i1.R},{1,i1.H,i1.H*i1.S,i1.H*i1.S*i1.C}); }
}

//Process
if (i1.T==1)
{
    valarray<float> X(i1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
    if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<float>(X[GS]); }
    if (o1.T==1)
    {
        valarray<float> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==2)
    {
        valarray<double> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==3)
    {
        valarray<long double> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==8)
    {
        valarray<int8_t> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==9)
    {
        valarray<uint8_t> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==16)
    {
        valarray<int16_t> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==17)
    {
        valarray<uint16_t> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==32)
    {
        valarray<int32_t> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==33)
    {
        valarray<uint32_t> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==64)
    {
        valarray<int64_t> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==65)
    {
        valarray<uint64_t> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==101)
    {
        valarray<complex<float>> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==102)
    {
        valarray<complex<double>> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==103)
    {
        valarray<complex<long double>> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else { cerr << progstr+": " << __LINE__ << errstr << "output data type not recognized" << endl; return 1; }
}
else if (i1.T==101)
{
    valarray<complex<float>> X(i1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
    if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<complex<float>>(X[GS]); }
    if (o1.T==101)
    {
        valarray<complex<float>> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==102)
    {
        valarray<complex<double>> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else if (o1.T==103)
    {
        valarray<complex<long double>> Y(o1.N());
        try { copy(begin(X),end(X),begin(Y)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem copying from input to output" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
        }
    }
    else { cerr << progstr+": " << __LINE__ << errstr << "output data type not recognized" << endl; return 1; }
}

//Finish

