//Includes
#include <complex>

//Declarations
const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,10u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
const size_t I = 1u, O = 0u;
size_t n;
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
struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
struct arg_str  *a_del = arg_strn("d","delim","<str>",0,1,"output delimiter [default=' ']");
struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,1,"output file (Y)");

//Get options

//Get delim
if (a_del->count==0) { delim = string(" "); }
else
{
    try { delim = string(*a_del->sval); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem getting delim string" << endl; return 1; }
}

//Checks

//Other prep

//Process
if (i1.T==1u)
{
    valarray<float> X(i1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
    for (size_t h=0u; h<i1.H; ++h)
    {
        if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
        for (size_t s=0u; s<i1.S; ++s)
        {
            if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
            for (size_t r=0u; r<i1.R; ++r)
            {
                if (i1.C>0u)
                {
                    n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                    try { ofs1 << X[n]; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                    for (size_t c=1u; c<i1.C; ++c)
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
else if (i1.T==8u)
{
    valarray<int8_t> X(i1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
    for (size_t h=0u; h<i1.H; ++h)
    {
        if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
        for (size_t s=0u; s<i1.S; ++s)
        {
            if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
            for (size_t r=0u; r<i1.R; ++r)
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
                    for (size_t c=1u; c<i1.C; ++c)
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
else if (i1.T==101u)
{
    valarray<complex<float>> X(i1.N());
    try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file" << endl; return 1; }
    for (size_t h=0u; h<i1.H; ++h)
    {
        if (i1.H>1u) { if (h>0u) { ofs1 << endl; } ofs1 << "hypercube hyperslice " << h << ":" << endl; }
        if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at hyperslice " << h << endl; return 1; }
        for (size_t s=0u; s<i1.S; ++s)
        {
            if (i1.S>1u) { if (s>0u) { ofs1 << endl; } ofs1 << "cube slice " << s << ":" << endl; }
            if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at slice " << s << endl; return 1; }
            for (size_t r=0u; r<i1.R; ++r)
            {
                if (i1.C>0u)
                {
                    n = (i1.iscolmajor()) ? r + s*i1.R*i1.C + h*i1.R*i1.C*i1.S : h + s*i1.H + r*i1.C*i1.S*i1.H;
                    try { ofs1 << X[n]; }
                    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output at row " << r << endl; return 1; }
                    for (size_t c=1u; c<i1.C; ++c)
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

//Finish
