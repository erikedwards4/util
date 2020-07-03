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
    const size_t I = 1, O = 1;
    ifstream ifs1; ofstream ofs1;
    int8_t stdi1, stdo1, wo1;
    ioinfo i1, o1;
    size_t Lr, Lc, Ls, Lh, Nr, Nc, Ns, Nh, p1, p2;
    uint rbeg, rend, cbeg, cend, sbeg, send, hbeg, hend, cnt;
    uint rstp, cstp, sstp, hstp;
    valarray<uint32_t> Ir, Ic, Is, Ih;
    string rowstr, colstr, slicestr, hyperslicestr;
    int s2i;


    //Description
    string descr;
    descr += "Selects rows, columns, slices and hyperslices of X,\n";
    descr += "using strings with Matlab/Python-like syntax.\n";
    descr += "\n";
    descr += "Use -r (--rows) to specify the rows with a str [default=':'].\n";
    descr += "Use -c (--cols) to specify the columns with a str [default=':'].\n";
    descr += "Use -s (--slices) to specify the slices with a str [default=':'].\n";
    descr += "Use -y (--hyperslices) to specify the hyperslices with a str [default=':'].\n";
    descr += "\n";
    descr += "For example:\n";
    descr += "-r'2'         --> select row 2, all cols.\n";
    descr += "-c'2'         --> select all rows, col 2.\n";
    descr += "-r'2' -c'2'   --> select element at row 2, col 2.\n";
    descr += "-r'2' -c'0:2' --> select row 2, cols 0 to 2.\n";
    descr += "-c'0:2'       --> select all rows, cols 0 to 2.\n";
    descr += "-c':2'        --> select all rows, cols 0 to 2.\n";
    descr += "-c'0:2:6'     --> select all rows, cols 0, 2, 4, 6.\n";
    descr += "-c'0:2:end'   --> select all rows, cols 0, 2, ..., end.\n";
    descr += "-c':end-1'    --> select all rows, cols 0 to Ci-2.\n";
    descr += "\n";
    descr += "Examples:\n";
    descr += "$ sel -c'0:3:9' X -o Y \n";
    descr += "$ sel -c'0:3:9' X > Y \n";
    descr += "$ cat X | sel -c'0:3:9' > Y \n";


    //Argtable
    int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file (X)");
    struct arg_str    *a_r = arg_strn("r","rows","<str>",0,1,"row numbers to get [default=':']");
    struct arg_str    *a_c = arg_strn("c","cols","<str>",0,1,"col numbers to get [default=':']");
    struct arg_str    *a_s = arg_strn("s","slices","<str>",0,1,"slice numbers to get [default=':']");
    struct arg_str    *a_h = arg_strn("y","hyperslices","<str>",0,1,"hyperslice numbers to get [default=':']");
    struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file (Y)");
    struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
    struct arg_end  *a_end = arg_end(5);
    void *argtable[] = {a_fi, a_r, a_c, a_s, a_h, a_fo, a_help, a_end};
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

    //Get rowstr
    if (a_r->count==0) { rowstr = ":"; }
    else
    {
        try { rowstr = string(a_r->sval[0]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading row string" << endl; return 1; }
    }
    Lr = rowstr.length();
    if (Lr==0) { cerr << progstr+": " << __LINE__ << errstr << "row string found to be empty" << endl; return 1; }

    //Get colstr
    if (a_c->count==0) { colstr = ":"; }
    else
    {
        try { colstr = string(a_c->sval[0]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading column string" << endl; return 1; }
    }
    Lc = colstr.length();
    if (Lc==0) { cerr << progstr+": " << __LINE__ << errstr << "column string found to be empty" << endl; return 1; }

    //Get slicestr
    if (a_s->count==0) { slicestr = ":"; }
    else
    {
        try { slicestr = string(a_s->sval[0]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading slice string" << endl; return 1; }
    }
    Ls = slicestr.length();
    if (Ls==0) { cerr << progstr+": " << __LINE__ << errstr << "slice string found to be empty" << endl; return 1; }

    //Get hyperslicestr
    if (a_h->count==0) { hyperslicestr = ":"; }
    else
    {
        try { hyperslicestr = string(a_h->sval[0]); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading hyperslice string" << endl; return 1; }
    }
    Lh = hyperslicestr.length();
    if (Lh==0) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice string found to be empty" << endl; return 1; }

    //Get Nr, Nc, Ns, Nh
    Nr = size_t(count(rowstr.begin(),rowstr.end(),':'));
    if (Nr>2) { cerr << progstr+": " << __LINE__ << errstr << "invalid row string, only 2 ':' can be used" << endl; return 1; }
    Nc = size_t(count(colstr.begin(),colstr.end(),':'));
    if (Nc>2) { cerr << progstr+": " << __LINE__ << errstr << "invalid column string, only 2 ':' can be used" << endl; return 1; }
    Ns = size_t(count(slicestr.begin(),slicestr.end(),':'));
    if (Ns>2) { cerr << progstr+": " << __LINE__ << errstr << "invalid slicestr string, only 2 ':' can be used" << endl; return 1; }
    Nh = size_t(count(hyperslicestr.begin(),hyperslicestr.end(),':'));
    if (Nh>2) { cerr << progstr+": " << __LINE__ << errstr << "invalid hyperslicestr string, only 2 ':' can be used" << endl; return 1; }

    //Get rbeg, rstp, rend for rows
    if (Nr>0) { p1 = rowstr.find_first_of(':'); } else { p1 = Lr; }
    if (Nr>1) { p2 = rowstr.find_last_of(':'); } else { p2 = p1; }
    if (p1==0) { rbeg = 0; }
    else
    {
        try { s2i = stoi(rowstr.substr(0,p1)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from row string" << endl; return 1; }
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "row nums must be nonnegative" << endl; return 1; }
        rbeg = uint(s2i);
        if (rbeg>=i1.R) { cerr << progstr+": " << __LINE__ << errstr << "row nums must be in [0 R-1]" << endl; return 1; }
    }
    if (Nr<2 || p2==p1+1) { rstp = 1; }
    else
    {
        try { s2i = stoi(rowstr.substr(p1+1,p2-p1-1)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from row string" << endl; return 1; }
        if (s2i<1) { cerr << progstr+": " << __LINE__ << errstr << "row step must be positive" << endl; return 1; }
        rstp = uint(s2i);
        if (rstp==0) { cerr << progstr+": " << __LINE__ << errstr << "row step cannot equal 0" << endl; return 1; }
    }
    if (Nr==0) { rend = rbeg; }
    else if (p2==Lr-1) { rend = i1.R - 1; }
    else if (Lr-p2>=4 && rowstr.substr(p2+1,3)=="end")
    {
        if (Lr-p2==4) { rend = i1.R - 1; }
        else if (Lr-p2==5) { cerr << progstr+": " << __LINE__ << errstr << "invalid row string" << endl; return 1; }
        else if (rowstr.substr(p2+4,1)!="-") { cerr << progstr+": " << __LINE__ << errstr << "invalid row string" << endl; return 1; }
        else
        {
            try { s2i = stoi(rowstr.substr(p2+5,Lr-p2-4)); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from row string" << endl; return 1; }
            if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "row nums must be nonnegative" << endl; return 1; }
            if (uint32_t(s2i+1)>i1.R) { cerr << progstr+": " << __LINE__ << errstr << "row end must be nonnegative" << endl; return 1; }
            rend = i1.R - uint32_t(s2i+1);
        }
    }
    else
    {
        try { s2i = stoi(rowstr.substr(p2+1,Lr-p2)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from row string" << endl; return 1; }
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "row nums must be nonnegative" << endl; return 1; }
        rend = uint(s2i);
    }
    if (rend>=i1.R) { cerr << progstr+": " << __LINE__ << errstr << "row nums must be in [0 R-1]" << endl; return 1; }
    if (rbeg>rend) { cerr << progstr+": " << __LINE__ << errstr << "row start must be <= row end" << endl; return 1; }
    if (rstp>0 && rbeg>rend) { cerr << progstr+": " << __LINE__ << errstr << "row start must be <= row end for positive row step" << endl; return 1; }
    //if (rstp<0 && rbeg<rend) { cerr << progstr+": " << __LINE__ << errstr << "row start must be >= row end for negative row step" << endl; return 1; }

    //Get cbeg, cstp, cend for cols
    if (Nc>0) { p1 = colstr.find_first_of(':'); } else { p1 = Lc; }
    if (Nc>1) { p2 = colstr.find_last_of(':'); } else { p2 = p1; }
    if (p1==0) { cbeg = 0; }
    else
    {
        try { s2i = stoi(colstr.substr(0,p1)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from col string" << endl; return 1; }
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "col nums must be nonnegative" << endl; return 1; }
        cbeg = uint(s2i);
        if (cbeg>=i1.C) { cerr << progstr+": " << __LINE__ << errstr << "col nums must be in [0 C-1]" << endl; return 1; }
    }
    if (Nc<2 || p2==p1+1) { cstp = 1; }
    else
    {
        try { s2i = stoi(colstr.substr(p1+1,p2-p1-1)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from col string" << endl; return 1; }
        if (s2i<1) { cerr << progstr+": " << __LINE__ << errstr << "col step must be positive" << endl; return 1; }
        cstp = uint(s2i);
        if (cstp==0) { cerr << progstr+": " << __LINE__ << errstr << "col step cannot equal 0" << endl; return 1; }
    }
    if (Nc==0) { cend = cbeg; }
    else if (p2==Lc-1) { cend = i1.C - 1; }
    else if (Lc-p2>=4 && colstr.substr(p2+1,3)=="end")
    {
        if (Lc-p2==4) { cend = i1.C - 1; }
        else if (Lc-p2==5) { cerr << progstr+": " << __LINE__ << errstr << "invalid col string" << endl; return 1; }
        else if (colstr.substr(p2+4,1)!="-") { cerr << progstr+": " << __LINE__ << errstr << "invalid col string" << endl; return 1; }
        else
        {
            try { s2i = stoi(colstr.substr(p2+5,Lc-p2-4)); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from col string" << endl; return 1; }
            if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "col nums must be nonnegative" << endl; return 1; }
            if (uint32_t(s2i+1)>i1.C) { cerr << progstr+": " << __LINE__ << errstr << "col end must be nonnegative" << endl; return 1; }
            cend = i1.C - uint32_t(s2i+1);
        }
    }
    else
    {
        try { s2i = stoi(colstr.substr(p2+1,Lc-p2)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from col string" << endl; return 1; }
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "col nums must be nonnegative" << endl; return 1; }
        cend = uint(s2i);
    }
    if (cend>=i1.C) { cerr << progstr+": " << __LINE__ << errstr << "col nums must be in [0 C-1]" << endl; return 1; }
    if (cstp>0 && cbeg>cend) { cerr << progstr+": " << __LINE__ << errstr << "col start must be <= col end for positive col step" << endl; return 1; }
    //if (cstp<0 && cbeg<cend) { cerr << progstr+": " << __LINE__ << errstr << "col start must be >= col end for negative col step" << endl; return 1; }

    //Get sbeg, sstp, send for slices
    if (Ns>0) { p1 = slicestr.find_first_of(':'); } else { p1 = Ls; }
    if (Ns>1) { p2 = slicestr.find_last_of(':'); } else { p2 = p1; }
    if (p1==0) { sbeg = 0; }
    else
    {
        try { s2i = stoi(slicestr.substr(0,p1)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from slice string" << endl; return 1; }
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "slice nums must be nonnegative" << endl; return 1; }
        sbeg = uint(s2i);
        if (sbeg>=i1.S) { cerr << progstr+": " << __LINE__ << errstr << "slice nums must be in [0 S-1]" << endl; return 1; }
    }
    if (Ns<2 || p2==p1+1) { sstp = 1; }
    else
    {
        try { s2i = stoi(slicestr.substr(p1+1,p2-p1-1)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from slice string" << endl; return 1; }
        if (s2i<1) { cerr << progstr+": " << __LINE__ << errstr << "slice step must be positive" << endl; return 1; }
        sstp = uint(s2i);
        if (sstp==0) { cerr << progstr+": " << __LINE__ << errstr << "slice step cannot equal 0" << endl; return 1; }
    }
    if (Ns==0) { send = sbeg; }
    else if (p2==Ls-1) { send = i1.S - 1; }
    else if (Ls-p2>=4 && slicestr.substr(p2+1,3)=="end")
    {
        if (Ls-p2==4) { send = i1.S - 1; }
        else if (Ls-p2==5) { cerr << progstr+": " << __LINE__ << errstr << "invalid slice string" << endl; return 1; }
        else if (slicestr.substr(p2+4,1)!="-") { cerr << progstr+": " << __LINE__ << errstr << "invalid slice string" << endl; return 1; }
        else
        {
            try { s2i = stoi(slicestr.substr(p2+5,Ls-p2-4)); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from slice string" << endl; return 1; }
            if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "slice nums must be nonnegative" << endl; return 1; }
            if (uint32_t(s2i+1)>i1.S) { cerr << progstr+": " << __LINE__ << errstr << "slice end must be nonnegative" << endl; return 1; }
            send = i1.S - uint32_t(s2i+1);
        }
    }
    else
    {
        try { s2i = stoi(slicestr.substr(p2+1,Ls-p2)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from slice string" << endl; return 1; }
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "slice nums must be nonnegative" << endl; return 1; }
        send = uint(s2i);
    }
    if (send>=i1.S) { cerr << progstr+": " << __LINE__ << errstr << "slice nums must be in [0 S-1]" << endl; return 1; }
    if (sstp>0 && sbeg>send) { cerr << progstr+": " << __LINE__ << errstr << "slice start must be <= slice end for positive slice step" << endl; return 1; }
    //if (sstp<0 && sbeg<send) { cerr << progstr+": " << __LINE__ << errstr << "slice start must be >= slice end for negative slice step" << endl; return 1; }

    //Get hbeg, hstp, hend for hyperslices
    if (Nh>0) { p1 = hyperslicestr.find_first_of(':'); } else { p1 = Lh; }
    if (Nh>1) { p2 = hyperslicestr.find_last_of(':'); } else { p2 = p1; }
    if (p1==0) { hbeg = 0; }
    else
    {
        try { s2i = stoi(hyperslicestr.substr(0,p1)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from hyperslice string" << endl; return 1; }
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice nums must be nonnegative" << endl; return 1; }
        hbeg = uint(s2i);
        if (hbeg>=i1.H) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice nums must be in [0 H-1]" << endl; return 1; }
    }
    if (Nh<2 || p2==p1+1) { hstp = 1; }
    else
    {
        try { s2i = stoi(hyperslicestr.substr(p1+1,p2-p1-1)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from hyperslice string" << endl; return 1; }
        if (s2i<1) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice step must be positive" << endl; return 1; }
        hstp = uint(s2i);
        if (hstp==0) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice step cannot equal 0" << endl; return 1; }
    }
    if (Nh==0) { hend = hbeg; }
    else if (p2==Lh-1) { hend = i1.H - 1; }
    else if (Lh-p2>=4 && hyperslicestr.substr(p2+1,3)=="end")
    {
        if (Lh-p2==4) { hend = i1.H - 1; }
        else if (Lh-p2==5) { cerr << progstr+": " << __LINE__ << errstr << "invalid hyperslice string" << endl; return 1; }
        else if (hyperslicestr.substr(p2+4,1)!="-") { cerr << progstr+": " << __LINE__ << errstr << "invalid hyperslice string" << endl; return 1; }
        else
        {
            try { s2i = stoi(hyperslicestr.substr(p2+5,Lh-p2-4)); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from hyperslice string" << endl; return 1; }
            if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "slice nums must be nonnegative" << endl; return 1; }
            if (uint32_t(s2i+1)>i1.H) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice end must be nonnegative" << endl; return 1; }
            hend = i1.H - uint32_t(s2i+1);
        }
    }
    else
    {
        try { s2i = stoi(hyperslicestr.substr(p2+1,Lh-p2)); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading int from hyperslice string" << endl; return 1; }
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice nums must be nonnegative" << endl; return 1; }
        hend = uint(s2i);
    }
    if (hend>=i1.H) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice nums must be in [0 H-1]" << endl; return 1; }
    if (hstp>0 && hbeg>hend) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice start must be <= hyperslice end for positive hyperslice step" << endl; return 1; }
    //if (hstp<0 && hbeg<hend) { cerr << progstr+": " << __LINE__ << errstr << "hyperslice start must be >= hyperslice end for negative hyperslice step" << endl; return 1; }

    //Get output nrows
    o1.R = 0u; cnt = rbeg;
    while (cnt<=rend && cnt<i1.R) { o1.R++; cnt+=rstp; }
    //if (rstp<0) { while (cnt>=rend && cnt>=0) { o1.R++; cnt+=rstp; } }
    //else { while (cnt<=rend && cnt<i1.R) { o1.R++; cnt+=rstp; } }

    //Get output ncols
    o1.C = 0u; cnt = cbeg;
    while (cnt<=cend && cnt<i1.C) { o1.C++; cnt+=cstp; }
    //if (cstp<0) { while (cnt>=cend && cnt>=0) { o1.C++; cnt+=cstp; } }
    //else { while (cnt<=cend && cnt<i1.C) { o1.C++; cnt+=cstp; } }

    //Get output nslices
    o1.S = 0u; cnt = sbeg;
    while (cnt<=send && cnt<i1.S) { o1.S++; cnt+=sstp; }
    //if (sstp<0) { while (cnt>=send && cnt>=0) { o1.S++; cnt+=sstp; } }
    //else { while (cnt<=send && cnt<i1.S) { o1.S++; cnt+=sstp; } }

    //Get output nhyperslices
    o1.H = 0u; cnt = hbeg;
    while (cnt<=hend && cnt<i1.H) { o1.H++; cnt+=hstp; }
    //if (hstp<0) { while (cnt>=hend && cnt>=0) { o1.H++; cnt+=hstp; } }
    //else { while (cnt<=hend && cnt<i1.H) { o1.H++; cnt+=hstp; } }


    //Set output header info
    o1.F = i1.F; o1.T = i1.T;


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
    if (i1.T==1)
    {
        valarray<float> X(i1.N()), Y(o1.N());
        try { ifs1.read(reinterpret_cast<char*>(&X[0]),i1.nbytes()); }
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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
        catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading data for input file 1 (X)" << endl; return 1; }
        if (i1.iscolmajor())
        {
            //try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C,{o1.S,o1.C,o1.R},{sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            try { Y = X[gslice(rbeg+cbeg*i1.R+sbeg*i1.R*i1.C+hbeg*i1.R*i1.C*i1.S,{o1.H,o1.S,o1.C,o1.R},{hstp*i1.R*i1.C*i1.S,sstp*i1.R*i1.C,cstp*i1.R,rstp})]; }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem selecting from input" << endl; return 1; }
        }
        else
        {
            //try { Y = X[gslice(sbeg+cbeg*i1.S+rbeg*i1.S*i1.C,{o1.R,o1.C,o1.S},{rstp*i1.S*i1.C,cstp*i1.S,sstp})]; }
            try { Y = X[gslice(hbeg+sbeg*i1.H+cbeg*i1.S*i1.H+rbeg*i1.C*i1.S*i1.H,{o1.R,o1.C,o1.S,o1.H},{rstp*i1.C*i1.S*i1.H,cstp*i1.S*i1.H,sstp*i1.H,hstp})]; }
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

