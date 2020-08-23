//@author Erik Edwards
//@date 2018-present
//@license BSD 3-clause


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <valarray>
#include <unordered_map>
#include <argtable2.h>
#include "../util/cmli.hpp"
#include <complex>

#ifdef I
#undef I
#endif


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
    size_t s;


    //Description
    string descr;
    descr += "Gets a single slice of cube X.\n";
    descr += "\n";
    descr += "Examples:\n";
    descr += "$ slice -s2 X -o Y \n";
    descr += "$ slice -s2 X > Y \n";
    descr += "$ cat X | slice -s2 > Y \n";


    //Argtable
    int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
    struct arg_int    *a_s = arg_intn("s","slice","<uint>",0,1,"slice number to get [default=0]");
    struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");
    struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
    struct arg_end  *a_end = arg_end(5);
    void *argtable[] = {a_fi, a_s, a_fo, a_help, a_end};
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
        for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1u]) ? "}" : ","); }
        cerr << endl; return 1;
    }


    //Get options

    //Get s
    if (a_s->count==0) { s = 0u; }
    else if (a_s->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "s must be nonnegative" << endl; return 1; }
    else { s = size_t(a_s->ival[0]); }
    if (s>=i1.S) { cerr << progstr+": " << __LINE__ << errstr << "s must be int in [0 S-1]" << endl; return 1; }


    //Set output header info
    o1.F = i1.F; o1.T = i1.T;
    o1.R = i1.R; o1.C = i1.C;
    o1.S = 1u; o1.H = i1.H;


    //Open output
    if (wo1)
    {
        if (stdo1) { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs1.open(a_fo->filename[0]); }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file 1" << endl; return 1; }
    }


    //Write output header
    if (wo1 && !write_output_header(ofs1,o1)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file 1" << endl; return 1; }


    //Other prep


    //Process
    if (i1.T==1u)
    {
        valarray<float> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==2)
    {
        valarray<double> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==3)
    {
        valarray<long double> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==8)
    {
        valarray<int8_t> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==9)
    {
        valarray<uint8_t> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==16)
    {
        valarray<int16_t> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==17)
    {
        valarray<uint16_t> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==32)
    {
        valarray<int32_t> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==33)
    {
        valarray<uint32_t> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==64)
    {
        valarray<int64_t> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==65)
    {
        valarray<uint64_t> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==101)
    {
        valarray<complex<float>> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==102)
    {
        valarray<complex<double>> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==103)
    {
        valarray<complex<long double>> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[slice(s*i1.R*i1.C,i1.R*i1.C,1)]; }
            try { Y = X[gslice(s*i1.R*i1.C,{i1.H,i1.R*i1.C},{i1.R*i1.C*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(s,{i1.R,i1.C},{i1.S*i1.C,i1.S})]; }
            try { Y = X[gslice(s*i1.H,{i1.R,i1.C,i1.H},{i1.H*i1.S*i1.C,i1.H*i1.S,1})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file (Y)" << endl; return 1; }
        }
    }
    else
    {
        cerr << progstr+": " << __LINE__ << errstr << "data type not supported" << endl; return 1;
    }
    

    //Exit
    return ret;
}

