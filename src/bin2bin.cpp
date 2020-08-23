//@author Erik Edwards
//@date 2018-present
//@license BSD 3-clause


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <valarray>
#include <complex>
#include <unordered_map>
#include <argtable2.h>
#include "../util/cmli.hpp"


int main(int argc, char *argv[])
{
    using namespace std;


    //Declarations
    int ret = 0;
    const string errstr = ": \033[1;31merror:\033[0m ";
    const string warstr = ": \033[1;35mwarning:\033[0m ";
    const string progstr(__FILE__,string(__FILE__).find_last_of("/")+1,strlen(__FILE__)-string(__FILE__).find_last_of("/")-5);
    const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
    const size_t I = 1u, O = 1u;
    ifstream ifs1; ofstream ofs1;
    int8_t stdi1, stdo1, wo1;
    ioinfo i1, o1;
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
    int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
    struct arg_int *a_otyp = arg_intn("t","type","<uint>",0,1,"output data type [default as input]");
    struct arg_int *a_ofmt = arg_intn("f","fmt","<uint>",0,1,"output file format [default as input]");
    struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");
    struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
    struct arg_end  *a_end = arg_end(5);
    void *argtable[] = {a_fi, a_otyp, a_ofmt, a_fo, a_help, a_end};
    if (arg_nullcheck(argtable)!=0) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating argtable" << endl; return 1; }
    nerrs = arg_parse(argc, argv, argtable);
    if (a_help->count>0)
    {
        cout << "Usage: " << progstr; arg_print_syntax(stdout, argtable, "\n");
        cout << endl; arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        cout << endl << descr; return 1;
    }
    if (nerrs>0) { arg_print_errors(stderr,a_end,(progstr+": "+to_string(__LINE__)+errstr).c_str()); return 1; }


    //Check stdin
    stdi1 = (a_fi->count==0 || strlen(a_fi->filename[0])==0 || strcmp(a_fi->filename[0],"-")==0);
    if (stdi1>0 && isatty(fileno(stdin))) { cerr << progstr+": " << __LINE__ << errstr << "no stdin detected" << endl; return 1; }


    //Check stdout
    if (a_fo->count>0) { stdo1 = (strlen(a_fo->filename[0])==0 || strcmp(a_fo->filename[0],"-")==0); }
    else { stdo1 = (!isatty(fileno(stdout))); }
    wo1 = (stdo1 || a_fo->count>0);


    //Open input
    if (stdi1) { ifs1.copyfmt(cin); ifs1.basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs1.open(a_fi->filename[0]); }
    if (!ifs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input file" << endl; return 1; }


    //Read input header
    if (!read_input_header(ifs1,i1)) { cerr << progstr+": " << __LINE__ << errstr << "problem reading header for input file" << endl; return 1; }
    if ((i1.T==oktypes).sum()==0)
    {
        cerr << progstr+": " << __LINE__ << errstr << "input data type must be in " << "{";
        for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1]) ? "}" : ","); }
        cerr << endl; return 1;
    }


    //Get options

    //Get o1.F
    if (a_ofmt->count==0) { o1.F = i1.F; }
    else if (a_ofmt->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "file format must be nonnegative" << endl; return 1; }
    else if (a_ofmt->ival[0]>255) { cerr << progstr+": " << __LINE__ << errstr << "file format must be < 256" << endl; return 1; }
    else { o1.F = size_t(a_ofmt->ival[0]); }

    //Get o1.T
    if (a_otyp->count==0) { o1.T = i1.T; }
    else if (a_otyp->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "data type must be positive" << endl; return 1; }
    else if (a_otyp->ival[0]>103) { cerr << progstr+": " << __LINE__ << errstr << "data type must be <= 103" << endl; return 1; }
    else { o1.T = size_t(a_otyp->ival[0]); }
    if ((o1.T==oktypes).sum()==0)
    {
        cerr << progstr+": " << __LINE__ << errstr << "output data type must be in " << "{";
        for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1]) ? "}" : ","); }
        cerr << endl; return 1;
    }


    //Checks
    if (!i1.isreal() && o1.isreal()) { cerr << progstr+": " << __LINE__ << errstr << "cannot convert from complex to real data type" << endl; return 1; }


    //Set output header info
    o1.R = i1.R; o1.C = i1.C; o1.S = i1.S; o1.H = i1.H;


    //Open output
    if (wo1)
    {
        if (stdo1) { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs1.open(a_fo->filename[0]); }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file 1" << endl; return 1; }
    }


    //Write output header
    if (wo1 && !write_output_header(ofs1,o1)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file 1" << endl; return 1; }


    //Other prep
    if (i1.iscolmajor()!=o1.iscolmajor())
    {
        if (i1.iscolmajor()) { GS = gslice(0,{i1.R,i1.C,i1.S,i1.H},{1,i1.R,i1.R*i1.C,i1.R*i1.C*i1.S}); }
        else { GS = gslice(0,{i1.H,i1.S,i1.C,i1.R},{1,i1.H,i1.H*i1.S,i1.H*i1.S*i1.C}); }
    }
    

    //Process
    if (i1.T==1u)
    {
        valarray<float> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<float>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==2u)
    {
        valarray<double> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<double>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==3u)
    {
        valarray<long double> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<long double>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==8u)
    {
        valarray<int8_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<int8_t>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==9u)
    {
        valarray<uint8_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<uint8_t>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==16u)
    {
        valarray<int16_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<int16_t>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==17u)
    {
        valarray<uint16_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<uint16_t>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==32u)
    {
        valarray<int32_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<int32_t>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==33u)
    {
        valarray<uint32_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<uint32_t>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==64u)
    {
        valarray<int64_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<int64_t>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==65u)
    {
        valarray<uint64_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<uint64_t>(X[GS]); }
        if (o1.T==1u)
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
        else if (o1.T==2u)
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
        else if (o1.T==3u)
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
        else if (o1.T==8u)
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
        else if (o1.T==9u)
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
        else if (o1.T==16u)
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
        else if (o1.T==17u)
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
        else if (o1.T==32u)
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
        else if (o1.T==33u)
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
        else if (o1.T==64u)
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
        else if (o1.T==65u)
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
        else if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==101u)
    {
        valarray<complex<float>> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<complex<float>>(X[GS]); }
        if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==102u)
    {
        valarray<complex<double>> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<complex<double>>(X[GS]); }
        if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else if (i1.T==103u)
    {
        valarray<complex<long double>> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        if (i1.iscolmajor()!=o1.iscolmajor()) { X = valarray<complex<long double>>(X[GS]); }
        if (o1.T==101u)
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
        else if (o1.T==102u)
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
        else if (o1.T==103u)
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
    else
    {
        cerr << progstr+": " << __LINE__ << errstr << "data type not supported" << endl; return 1;
    }
    

    //Exit
    return ret;
}

