//@author Erik Edwards
//@date 2019-2020


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <valarray>
#include <unordered_map>
#include <argtable2.h>
#include "/home/erik/codee/util/cmli.hpp"
#include <cmath>
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
    const valarray<uint8_t> oktypes = {1,2,3,8,9,16,17,32,33,64,65,101,102,103};
    const size_t I = 2u, O = 1u;
    ifstream ifs1, ifs2; ofstream ofs1;
    int8_t stdi1, stdi2, stdo1, wo1;
    ioinfo i1, i2, o1;
    size_t ri1, ri2, ci1, ci2, si1, si2, hi1, hi2;
    gslice GS1, GS2;


    //Description
    string descr;
    descr += "<cmath> function, isgreaterequal.\n";
    descr += "\n";
    descr += "Checks if X1>=X2, element-wise.\n";
    descr += "For complex inputs, checks if |X1|>=|X2|, element-wise.\n";
    descr += "\n";
    descr += "X1 and X2 must have the same data type.\n";
    descr += "X1 and X2 must have the same size or broadcast-compatible sizes.\n";
    descr += "\n";
    descr += "Output (Y) has int8_t data type (since boolean).\n";
    descr += "Output (Y) has size max(R1,R2) x max(C1,C2).\n";
    descr += "\n";
    descr += "Examples:\n";
    descr += "$ isgreaterequal X1 X2 -o Y \n";
    descr += "$ isgreaterequal X1 X2 > Y \n";
    descr += "$ cat X2 | isgreaterequal X1 > Y \n";


    //Argtable
    int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input files (X1,X2)");
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
    if (i2.iscolmajor()!=i1.iscolmajor()) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have the same row/col major format" << endl; return 1; }
    if (i2.T!=i1.T) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have the same data type" << endl; return 1; }
    if (!bcast_compat(i1,i2)) { cerr << progstr+": " << __LINE__ << errstr << "inputs must have same size or broadcast-compatible sizes" << endl; return 1; }


    //Set output header info
    o1.F = i1.F; o1.T = 8;
    o1.R = (i1.R>i2.R) ? i1.R : i2.R;
    o1.C = (i1.C>i2.C) ? i1.C : i2.C;
    o1.S = (i1.S>i2.S) ? i1.S : i2.S;
    o1.H = (i1.H>i2.H) ? i1.H : i2.H;


    //Open output
    if (wo1)
    {
        if (stdo1) { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs1.open(a_fo->filename[0]); }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file 1" << endl; return 1; }
    }


    //Write output header
    if (wo1 && !write_output_header(ofs1,o1)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file 1" << endl; return 1; }


    //Other prep
    ri1 = !(i1.R==1u && o1.R>1u); ci1 = !(i1.C==1u && o1.C>1u); si1 = !(i1.S==1u && o1.S>1u); hi1 = !(i1.H==1u && o1.H>1u);
    ri2 = !(i2.R==1u && o1.R>1u); ci2 = !(i2.C==1u && o1.C>1u); si2 = !(i2.S==1u && o1.S>1u); hi2 = !(i2.H==1u && o1.H>1u);
    if (o1.iscolmajor())
    {
        try { GS1 = gslice(0,{o1.H,o1.S,o1.C,o1.R},{hi1*i1.R*i1.C*i1.S,si1*i1.R*i1.C,ci1*i1.R,ri1}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem making gslice for input 1" << endl; return 1; }
        try { GS2 = gslice(0,{o1.H,o1.S,o1.C,o1.R},{hi2*i2.R*i2.C*i2.S,si2*i2.R*i2.C,ci2*i2.R,ri2}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem making gslice for input 2" << endl; return 1; }
    }
    else
    {
        try { GS1 = gslice(0,{o1.R,o1.C,o1.S,o1.H},{ri1*i1.C*i1.S*i1.H,ci1*i1.S*i1.H,si1*i1.H,hi1}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem making gslice for input 1" << endl; return 1; }
        try { GS2 = gslice(0,{o1.R,o1.C,o1.S,o1.H},{ri2*i2.C*i2.S*i2.H,ci2*i2.S*i2.H,si2*i2.H,hi2}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem making gslice for input 2" << endl; return 1; }
    }
    

    //Process
    if (i1.T==1)
    {
        valarray<float> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<float>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<float>(X2[GS2])),begin(Y),[](float x1,float x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==2)
    {
        valarray<double> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<double>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<double>(X2[GS2])),begin(Y),[](double x1,double x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==3)
    {
        valarray<long double> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<long double>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<long double>(X2[GS2])),begin(Y),[](long double x1,long double x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==8)
    {
        valarray<int8_t> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<int8_t>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<int8_t>(X2[GS2])),begin(Y),[](int8_t x1,int8_t x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==9)
    {
        valarray<uint8_t> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<uint8_t>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<uint8_t>(X2[GS2])),begin(Y),[](uint8_t x1,uint8_t x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==16)
    {
        valarray<int16_t> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<int16_t>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<int16_t>(X2[GS2])),begin(Y),[](int16_t x1,int16_t x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==17)
    {
        valarray<uint16_t> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<uint16_t>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<uint16_t>(X2[GS2])),begin(Y),[](uint16_t x1,uint16_t x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==32)
    {
        valarray<int32_t> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<int32_t>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<int32_t>(X2[GS2])),begin(Y),[](int32_t x1,int32_t x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==33)
    {
        valarray<uint32_t> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<uint32_t>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<uint32_t>(X2[GS2])),begin(Y),[](uint32_t x1,uint32_t x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==64)
    {
        valarray<int64_t> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<int64_t>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<int64_t>(X2[GS2])),begin(Y),[](int64_t x1,int64_t x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==65)
    {
        valarray<uint64_t> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<uint64_t>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<uint64_t>(X2[GS2])),begin(Y),[](uint64_t x1,uint64_t x2){return isgreaterequal(x1,x2);}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==101)
    {
        valarray<complex<float>> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<complex<float>>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<complex<float>>(X2[GS2])),begin(Y),[](complex<float> x1,complex<float> x2){return isgreaterequal(abs(x1),abs(x2));}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==102)
    {
        valarray<complex<double>> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<complex<double>>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<complex<double>>(X2[GS2])),begin(Y),[](complex<double> x1,complex<double> x2){return isgreaterequal(abs(x1),abs(x2));}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else if (i1.T==103)
    {
        valarray<complex<long double>> X1(i1.N()), X2(i2.N()); valarray<int8_t> Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X1[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 1 (X1)" << endl; return 1; }
        try { ifs2.read(reinterpret_cast<char*>(&X2[0]),i2.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file 2 (X2)" << endl; return 1; }
        try { X1 = valarray<complex<long double>>(X1[GS1]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem slicing input 1" << endl; return 1; }
        try { transform (begin(X1),end(X1),begin(valarray<complex<long double>>(X2[GS2])),begin(Y),[](complex<long double> x1,complex<long double> x2){return isgreaterequal(abs(x1),abs(x2));}); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem applying function" << endl; return 1; }
        if (Y.size()!=o1.N()) { cerr << progstr+": " << __LINE__ << errstr << "unexpected output size" << endl; return 1; }
        if (wo1)
        {
            try { ofs1.write(reinterpret_cast<char*>(&Y[0]),o1.nbytes()); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file (Y)" << endl; return 1; }
        }
    }
    else
    {
        cerr << progstr+": " << __LINE__ << errstr << "data type not supported" << endl; return 1;
    }
    

    //Exit
    return ret;
}

