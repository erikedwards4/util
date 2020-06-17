//@author Erik Edwards
//@date 2019-2020


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <valarray>
#include <complex>
#include <unordered_map>
#include <argtable2.h>
#include "/home/erik/codee/util/cmli.hpp"

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
    const valarray<uint8_t> oktypes = {1,2,3,8,9,16,17,32,33,64,65,101,102,103};
    const size_t I = 2, O = 1;
    ifstream ifs1, ifs2; ofstream ofs1;
    int8_t stdi1, stdi2, stdo1, wo1;
    ioinfo i1, i2, o1;
    uint32_t r, ro;
    valarray<uint32_t> X2;


    //Description
    string descr;
    descr += "Gets rows of X using array of row indices.\n";
    descr += "\n";
    descr += "The 2nd input is I, the file with the row indices.\n";
    descr += "This must be in binary format with uint32_t data type,\n";
    descr += "e.g., echo '0 2 3 7' | txt2bin -f33 > I \n";
    descr += "\n";
    descr += "The output Y will have rows in the same order as X.\n";
    descr += "That is, I is effectively sorted to ascending order.\n";
    descr += "This is unlike cols, so cannot be used to reorder rows.\n";
    descr += "However, indices can be repeated to repeat rows in the output.\n";
    descr += "\n";
    descr += "Examples:\n";
    descr += "$ rows X I -o Y \n";
    descr += "$ rows X I > Y \n";
    descr += "$ cat I | rows X > Y \n";
    descr += "$ cat X | rows - I > Y \n";


    //Argtable
    int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input files (X,X2)");
    struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");
    struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
    struct arg_end  *a_end = arg_end(5);
    void *argtable[] = {a_fi, a_fo, a_help, a_end};
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
    stdi2 = (a_fi->count<=1 || strlen(a_fi->filename[1])==0 || strcmp(a_fi->filename[1],"-")==0);
    if (stdi1+stdi2>1) { cerr << progstr+": " << __LINE__ << errstr << "can only use stdin for one input" << endl; return 1; }
    if (stdi1+stdi2>0 && isatty(fileno(stdin))) { cerr << progstr+": " << __LINE__ << errstr << "no stdin detected" << endl; return 1; }


    //Check stdout
    if (a_fo->count>0) { stdo1 = (strlen(a_fo->filename[0])==0 || strcmp(a_fo->filename[0],"-")==0); }
    else { stdo1 = (!isatty(fileno(stdout))); }
    wo1 = (stdo1 || a_fo->count>0);


    //Open inputs
    if (stdi1) { ifs1.copyfmt(cin); ifs1.basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs1.open(a_fi->filename[0]); }
    if (!ifs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input file 1" << endl; return 1; }
    if (stdi2) { ifs2.copyfmt(cin); ifs2.basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs2.open(a_fi->filename[1]); }
    if (!ifs2) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input file 2" << endl; return 1; }


    //Read input headers
    if (!read_input_header(ifs1,i1)) { cerr << progstr+": " << __LINE__ << errstr << "problem reading header for input file 1" << endl; return 1; }
    if (!read_input_header(ifs2,i2)) { cerr << progstr+": " << __LINE__ << errstr << "problem reading header for input file 2" << endl; return 1; }
    if ((i1.T==oktypes).sum()==0 || (i2.T==oktypes).sum()==0)
    {
        cerr << progstr+": " << __LINE__ << errstr << "input data type must be in " << "{";
        for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1]) ? "}" : ","); }
        cerr << endl; return 1;
    }


    //Get options


    //Checks
    if (i2.T!=33) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (I) data type must be 33 (uint32_t)" << endl; return 1; }
    if (!i2.isvec()) { cerr << progstr+": " << __LINE__ << errstr << "input 2 (I) must be a vector of indices" << endl; return 1; }


    //Set output header info
    o1.F = i1.F; o1.T = i1.T;
    o1.R = i2.N(); o1.C = i1.C; o1.S = i1.S; o1.H = i1.H;


    //Open output
    if (wo1)
    {
        if (stdo1) { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs1.open(a_fo->filename[0]); }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file 1" << endl; return 1; }
    }


    //Write output header
    if (wo1 && !write_output_header(ofs1,o1)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file 1" << endl; return 1; }


    //Other prep
    try { X2.resize(i2.N(),0u); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem allocating for input 2 (I)" << endl; return 1; }
    try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (I)" << endl; return 1; }
    if ((X2>=i1.R).sum()!=0) { cerr << progstr+": " << __LINE__ << errstr << "row indices must be < nrows X" << endl; return 1; }
    

    //Process
    if (i1.T==1)
    {
        valarray<float> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        for (ro=0u; ro<o1.R; ro++)
        {
            r = X2[ro];
            if (i1.iscolmajor())
            {
                //try { Y[gslice(ro,{o1.S,o1.C},{o1.R*o1.C,o1.R})] = X[gslice(r,{i1.S,i1.C},{i1.R*i1.C,i1.R})]; }
                try { Y[gslice(ro,{o1.H,o1.S,o1.C},{o1.R*o1.C*o1.S,o1.R*o1.C,o1.R})] = X[gslice(r,{i1.H,i1.S,i1.C},{i1.R*i1.C*i1.S,i1.R*i1.C,i1.R})]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
            else
            {
                //try { Y[slice(ro*o1.C*o1.S,o1.C*o1.S,1)] = X[slice(r*i1.C*i1.S,i1.C*i1.S,1)]; }
                try { Y[slice(ro*o1.C*o1.S*o1.H,o1.C*o1.S*o1.H,1)] = X[slice(r*i1.C*i1.S*i1.H,i1.C*i1.S*i1.H,1)]; }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
            }
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

