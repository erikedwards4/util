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
    const valarray<uint8_t> oktypes = {1,2,3,8,9,10,16,17,32,33,64,65,101,102,103};
    const size_t I = 1;
    ifstream ifs1; ofstream ofs1;
    int8_t stdi1, stdo1, wo1;
    ioinfo i1;
    size_t r, c, s, h, n;
    string delim;


    //Description
    string descr;
    descr += "Converts the cmli-format binary file X to an ascii text file.\n";
    descr += "If no inputs, then assumes that input is piped in from stdin.\n";
    descr += "\n";
    descr += "Examples:\n";
    descr += "$ bin2txt X -o X.txt \n";
    descr += "$ bin2txt X > X.txt \n";
    descr += "$ cat X | bin2txt > X.txt \n";


    //Argtable
    int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
    struct arg_str  *a_del = arg_strn("d","delim","<str>",0,1,"output delimiter [default=' ']");
    struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,1,"output file (Y)");
    struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
    struct arg_end  *a_end = arg_end(5);
    void *argtable[] = {a_fi, a_del, a_fo, a_help, a_end};
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
    stdo1 = (a_fo->count==0 || strlen(a_fo->filename[0])==0 || strcmp(a_fo->filename[0],"-")==0);
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

    //Get delim
    if (a_del->count==0) { delim = string(" "); }
    else
    {
        try { delim = string(*a_del->sval); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem getting delim string" << endl; return 1; }
    }


    //Open output
    if (wo1)
    {
        if (stdo1) { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs1.open(a_fo->filename[0]); }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file 1" << endl; return 1; }
    }


    //Other prep


    //Process
    if (i1.T==1)
    {
        valarray<float> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        try { ofs1 << X[n]; }
                        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            try { ofs1 << delim << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==2)
    {
        valarray<double> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        try { ofs1 << X[n]; }
                        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            try { ofs1 << delim << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==3)
    {
        valarray<long double> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        try { ofs1 << X[n]; }
                        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            try { ofs1 << delim << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==8)
    {
        valarray<int8_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==9)
    {
        valarray<uint8_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==10)
    {
        valarray<bool> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==16)
    {
        valarray<int16_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==17)
    {
        valarray<uint16_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==32)
    {
        valarray<int32_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==33)
    {
        valarray<uint32_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==64)
    {
        valarray<int64_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==65)
    {
        valarray<uint64_t> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        if (i1.T<16)
                        {
                            try { ofs1 << int(X[n]); }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        else
                        {
                            try { ofs1 << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            if (i1.T<16)
                            {
                                try { ofs1 << delim << int(X[n]); }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                            else
                            {
                                try { ofs1 << delim << X[n]; }
                                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                            }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==101)
    {
        valarray<complex<float>> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        try { ofs1 << X[n]; }
                        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            try { ofs1 << delim << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==102)
    {
        valarray<complex<double>> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        try { ofs1 << X[n]; }
                        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            try { ofs1 << delim << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else if (i1.T==103)
    {
        valarray<complex<long double>> X(i1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
        for (h=0u; h<i1.H; h++)
        {
            if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
            for (s=0u; s<i1.S; s++)
            {
                if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
                if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
                for (r=0u; r<i1.R; r++)
                {
                    if (i1.C>0u)
                    {
                        n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                        try { ofs1 << X[n]; }
                        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        for (c=1u; c<i1.C; c++)
                        {
                            n = (i1.iscolmajor()) ? r + c*i1.R + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + c*i1.S*i1.H + r*i1.C*i1.S*i1.H;
                            try { ofs1 << delim << X[n]; }
                            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                        }
                    }
                    try { ofs1 << endl; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                }
            }
        }
    }
    else
    {
        cerr << progstr+": " << __LINE__ << errstr << "data type not supported" << endl; return 1;
    }
    

    //Exit
    return ret;
}

