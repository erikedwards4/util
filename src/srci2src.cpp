//@author Erik Edwards
//@date 2018-present
//@license BSD 3-clause


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <argtable2.h>
#include <valarray>
#include <vector>
#include <unordered_map>
#include <cctype>


int main(int argc, char *argv[])
{
	using namespace std;
    //timespec tic, toc; clock_gettime(CLOCK_REALTIME,&tic);
	
	
	//Declarations
	int ret = 0;
	const string errstr = ": \033[1;31merror:\033[0m ";
	const string warstr = ": \033[1;35mwarning:\033[0m ";
    const string progstr(__FILE__,string(__FILE__).find_last_of("/")+1,strlen(__FILE__)-string(__FILE__).find_last_of("/")-5);
    valarray<char> stdi(1);
    ifstream ifs;
    string line;
    size_t pos1, pos2;
    //const vector<string> includes = {"<iostream>","<fstream>","<unistd.h>","<string>","<cstring>","<valarray>","<complex>","<unordered_map>","<argtable2.h>","\"/home/erik/codee/util/cmli.hpp\""};
    const vector<string> includes = {"<iostream>","<fstream>","<unistd.h>","<string>","<cstring>","<valarray>","<unordered_map>","<argtable2.h>","\"../util/cmli.hpp\""};

	const string ind = "    ";
    size_t I, Imin, O, Omin, t, T, A = 0u;
    int s2i;

    const vector<size_t> types = {0u,1u,2u,3u,8u,9u,10u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
    const unordered_map<size_t,string> zros = {{1u,"0.0f"},{2u,"0.0"},{3u,"0.0L"},{8u,"'\0'"},{9u,"'\0'"},{10u,"false"},{16u,"0"},{17u,"0u"},{32u,"0"},{33u,"0u"},{64u,"0l"},{65u,"0ul"},{101u,"0.0f"},{102u,"0.0"},{103u,"0.0L"}};
    const unordered_map<size_t,string> ones = {{1u,"1.0f"},{2u,"1.0"},{3u,"1.0L"},{8u,"'\1'"},{9u,"'\1'"},{10u,"true"},{16u,"1"},{17u,"1u"},{32u,"1"},{33u,"1u"},{64u,"1l"},{65u,"1ul"},{101u,"1.0f"},{102u,"1.0"},{103u,"1.0L"}};
    const unordered_map<size_t,string> aftyps = {{1u,"f32"},{2u,"f64"},{8u,"b8"},{9u,"u8"},{10u,"b8"},{16u,"s16"},{17u,"u16"},{32u,"s32"},{33u,"u32"},{64u,"s64"},{65u,"u64"},{101u,"c32"},{102u,"c64"}};
    unordered_map<size_t,string> fmts = {{0u,"0"},{1u,"1"},{65u,"65"},{101u,"101"},{102u,"102"},{147u,"147"}};
    unordered_map<size_t,string> typs = {{0u,"txt"},{1u,"float"},{2u,"double"},{3u,"long double"},{8u,"int8_t"},{9u,"uint8_t"},{10u,"bool"},{16u,"int16_t"},{17u,"uint16_t"},{32u,"int32_t"},{33u,"uint32_t"},{64u,"int64_t"},{65u,"uint64_t"},{101u,"complex<float>"},{102u,"complex<double>"},{103u,"complex<long double>"}};
    vector<size_t> oktypes;
    vector<string> oktypestrs;
    string oktypestr;
    
    size_t c, prevc;
    vector<string> inames, onames, anames;
    string ttstr, tistr, tcstr, zi, zc, oi, oc, ai, ac;
    FILE *tmpf = tmpfile(), *tmpf8 = tmpfile(), *tmpf101 = tmpfile();
    bool do_float = false, do_int = false, do_complex = false;
    char buff[256*16];
    string::size_type n;
    bool tictoc = false;
    

    //Description
    string descr;
	descr += "Generates a generic CLI (command-line interface) program,\n";
    descr += "printing it to stdout (so can make .cpp file to edit). \n";
	descr += "This takes an input .cpp file from isrc,\n";
	descr += "and adds boilerplate and other code to make the final .cpp.\n";
	descr += "\n";
	descr += "Examples:\n";
	descr += "$ srci2src srci/X.cpp > src/X.cpp \n";


    //Argtable
	int nerrs;
    struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",0,1,"input file");
    struct arg_lit   *a_dz = arg_litn("d","dz",0,1,"sub _s->_d and _c->_z for function names (e.g. for blas names)");
    struct arg_lit   *a_ov = arg_litn("v","voice",0,1,"sub _s->_d and _c->_z for function names (similar to blas names)");
	struct arg_lit    *a_t = arg_litn("t","time",0,1,"include timing code around Process section");
	struct arg_lit    *a_T = arg_litn("T","TIME",0,1,"include timing code around whole program");
    struct arg_lit   *a_oh = arg_litn("O","OH",0,1,"include to omit Write output headers so can write at Finish");
	struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
	struct arg_end  *a_end = arg_end(5);
	void *argtable[] = {a_fi, a_dz, a_ov, a_t, a_T, a_oh, a_help, a_end};
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
    stdi[0] = (a_fi->count<=0 || strlen(a_fi->filename[0])==0 || strcmp(a_fi->filename[0],"-")==0);
	if (isatty(fileno(stdin)) && stdi.sum()>0) { cerr << progstr+": " << __LINE__ << errstr << "no stdin detected" << endl; return 1; }


    //Open input
    if (stdi[0]==0) { ifs.open(a_fi->filename[0], ifstream::binary); }
	else { ifs.copyfmt(cin); ifs.basic_ios<char>::rdbuf(cin.rdbuf()); }
	if (!ifs) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input isrc file" << endl; return 1; }


    //Sub TIC and TOC
    ofstream oftmp; FILE* ttmpf = tmpfile();
    oftmp.open(to_string(fileno(ttmpf)));
    while (getline(ifs,line) && !ifs.eof())
    {
        pos1 = line.find("//TIC"); pos2 = line.find("//TOC");
        if (pos1<line.size()) { oftmp << line.substr(0,pos1) << "clock_gettime(CLOCK_REALTIME,&tic); //TIC" << endl; }
        else if (pos2<line.size())
        {
            oftmp << line.substr(0,pos2) << "clock_gettime(CLOCK_REALTIME,&toc); //TOC" << endl;
            oftmp << line.substr(0,pos2) << "cerr << \"elapsed time = \" << (toc.tv_sec-tic.tv_sec)*1e3 + (toc.tv_nsec-tic.tv_nsec)/1e6 << \" ms\" << endl;" << endl;
            if (a_t->count>0) { cerr << progstr+": " << __LINE__ << warstr << "using tic/toc and -t option could conflict" << endl; }
            if (a_T->count>0) { cerr << progstr+": " << __LINE__ << errstr << "cannot use -T opt with other timing options" << endl; return 1; }
            tictoc = true;
        }
        else { oftmp << line << endl; }
    }
    oftmp.close(); ifs.close();
    ifs.open(to_string(fileno(ttmpf)));


    //PRINT OUT CODE
    
    //Initial comments
    cout << "//@author Erik Edwards" << endl;
    cout << "//@date 2018-present" << endl;
    cout << "//@license BSD 3-clause" << endl;
    getline(ifs,line);
    while (line.size()>0u && line.compare(0u,9u,"//Include")!=0) { cout << line << endl; getline(ifs,line); }
    cout << endl;


    //Includes
    while (line.compare(0u,9u,"//Include")!=0) { getline(ifs,line); }
    cout << endl;
    if (tictoc || a_t->count>0 || a_T->count>0) { cout << "#include <ctime>" << endl; }
    for (size_t i=0u; i<includes.size(); ++i) { cout << "#include " << includes[i] << endl; }
    getline(ifs,line);
    while (line.size()>0u && line.compare(0u,14u,"//Declarations")!=0) { cout << line << endl; getline(ifs,line); }
    cout << endl;


    //Undefine I
    cout << "#ifdef I" << endl << "#undef I" << endl << "#endif" << endl;
    cout << endl;


    //Start main
    cout << endl;
    cout << "int main(int argc, char *argv[])" << endl;
    cout << "{" << endl;
    cout << ind << "using namespace std;" << endl;
    if (tictoc || a_t->count>0 || a_T->count>0)
    {
        cout << ind << "timespec tic, toc;";
        if (a_T->count>0) { cout << " clock_gettime(CLOCK_REALTIME,&tic);"; }
        cout << endl;
    }
    cout << endl;


    //Declarations start
    while (line.compare(0u,14u,"//Declarations")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Declarations" << endl;
    cout << ind << "int ret = 0;" << endl;
    cout << ind << "const string errstr = \": \\033[1;31merror:\\033[0m \";" << endl;
    cout << ind << "const string warstr = \": \\033[1;35mwarning:\\033[0m \";" << endl;
    cout << ind << "const string progstr(__FILE__,string(__FILE__).find_last_of(\"/\")+1,strlen(__FILE__)-string(__FILE__).find_last_of(\"/\")-5);" << endl;
    

    //Get okfmts (only in txt2bin so far)
    //getline(ifs,line); //this line must have okfmts
    //if (line.size()>36u && line.compare(0u,34u,"const valarray<size_t> okfmts = {")==0) { cout << ind << line << endl; getline(ifs,line); }


    //Get oktypes
    getline(ifs,line); //this line must have oktypes
    //cerr << "line.compare = " << line.compare(0u,34u,"const valarray<size_t> oktypes = {") << endl;
    if (line.size()<35u || line.compare(0u,34u,"const valarray<size_t> oktypes = {")!=0) { cerr << progstr+": " << __LINE__ << errstr << "problem with line for oktypes" << endl; return 1; }
    cout << ind << line << endl;
    pos1 = line.find_first_of("{",0) + 1u;
    pos2 = line.find_first_of("}",0) - 1u;
    oktypestr = line.substr(pos1,pos2-pos1+1u);
    T = 1u; for (c=0u; c<oktypestr.size(); ++c) { if (oktypestr.substr(c,1u)==",") { ++T; } }
    t = prevc = 0u;
    for (c=0u; c<oktypestr.size(); ++c)
    {
        if (oktypestr.substr(c,1)=="," || oktypestr.substr(c,1)==" ")
        {
            s2i = stoi(oktypestr.substr(prevc,c-prevc));
            if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
            else { oktypes.push_back(size_t(s2i)); }
            prevc = c + 1u; ++t;
        }
    }
    s2i = stoi(oktypestr.substr(prevc,c-prevc));
    if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
    oktypes.push_back(size_t(s2i));
    for (t=0u; t<T; ++t) { oktypestrs.push_back(typs.at(oktypes[t])); }


    //Declare maps
    //cout << ind << "const unordered_map<size_t,std::streamsize> szs = {{0u,2u},{1u,4u},{2u,8u},{3u,16u},{8u,1u},{9u,1u},{10u,1u},{16u,2u},{17u,2u},{32u,4u},{33u,4u},{64u,8u},{65u,8u},{101u,8u},{102u,16u},{103u,32u}};" << endl;


    //Get I and O
    getline(ifs,line); //this line must have I and O
    if (line.size()<8u || line.find("size_t ")==string::npos) { cerr << progstr+": " << __LINE__ << errstr << "problem with line for I and O" << endl; return 1; }
    pos1 = line.find("=",0) + 2u; pos2 = line.find(",",0) - 1u;
    s2i = stoi(line.substr(pos1,pos2-pos1+1u));
    if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
    I = size_t(s2i);
    pos1 = line.find("O",0) + 4u; pos2 = line.find(";",0) - 1u;
    s2i = stoi(line.substr(pos1,pos2-pos1+1u));
    if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
    O = size_t(s2i);
    if (I>0u || O>0u) { cout << ind << "const size_t "; }
    if (I>0u) { cout << "I = " << I << "u"; }
    if (O>0u) { if (I>0u) { cout << ", "; } cout << "O = " << O << "u"; }
    cout << ";" << endl;


    //Declarations continue
    if (I==0)
    {
        cout << ind << "ofstream ofs1"; for (size_t o=1u; o<O; ++o) { cout << ", ofs" << o+1u; } cout << ";" << endl;
        cout << ind << "int8_t stdo1";
        for (size_t o=1u; o<O; ++o) { cout << ", stdo" << o+1u; }
        for (size_t o=0u; o==0 || o<O; ++o) { cout << ", wo" << o+1u; }
        cout << ";"; cout << endl;
        if (O>0u) { cout << ind << "ioinfo o1"; for (size_t o=1u; o<O; ++o) { cout << ", o" << o+1u; } cout << ";" << endl; }
    }
    else
    {
        cout << ind << "ifstream ifs1";
        for (size_t i=1u; i<I; ++i) { cout << ", ifs" << i+1u; } cout << "; ";
        cout << "ofstream ofs1"; for (size_t o=1u; o<O; ++o) { cout << ", ofs" << o+1u; } cout << ";" << endl;
        cout << ind << "int8_t stdi1";
        for (size_t i=1u; i<I; ++i) { cout << ", stdi" << i+1u; }
        for (size_t o=0u; o==0 || o<O; ++o) { cout << ", stdo" << o+1u; }
        for (size_t o=0u; o==0 || o<O; ++o) { cout << ", wo" << o+1u; }
        cout << ";"; cout << endl;
        cout << ind << "ioinfo i1";
        for (size_t i=1u; i<I; ++i) { cout << ", i" << i+1u; }
        for (size_t o=0u; o<O; ++o) { cout << ", o" << o+1u; }
        cout << ";" << endl;
    }
    getline(ifs,line);
    while (line.size()>0u && line.compare(0u,13u,"//Description")!=0) { cout << ind << line << endl; getline(ifs,line); }
    cout << endl;


    //Description
    while (line.compare(0u,13u,"//Description")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Description" << endl;
    getline(ifs,line);
    while (line.size()>0u && line.compare(0u,1u," ")!=0) { cout << ind << line << endl; getline(ifs,line); }
    cout << endl;


    //Argtable start
    while (line.compare(0u,10u,"//Argtable")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Argtable" << endl;
    cout << ind << "int nerrs;" << endl;


    //Get 1st argtable line and Imin and inames
    if (I>0u)
    {
        getline(ifs,line);
        cout << ind << line << endl;
        pos1 = line.find("<file>",0) + 5u;
        while (line.substr(pos1,1u)!=",") { ++pos1; }
        ++pos1; pos2 = pos1 + 1u;
        while (line.substr(pos2,1u)!=",") { ++pos2; }
        if (line.substr(pos1,pos2-pos1)=="I") { Imin = I; }
        else if (line.substr(pos1,pos2-pos1)=="I-1") { if (I>0u) { Imin = I-1u; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-2") { if (I>1u) { Imin = I-2u; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-3") { if (I>2u) { Imin = I-3u; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-4") { if (I>3u) { Imin = I-4u; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-5") { if (I>4u) { Imin = I-5u; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-6") { if (I>5u) { Imin = I-6u; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else
        {
            s2i = stoi(line.substr(pos1,pos2-pos1));
            if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
            Imin = size_t(s2i);
        }
        pos1 = line.find("input file",0) + 10u;
        while (line.substr(pos1,1)!="(") { ++pos1; }
        pos2 = pos1;
        for (size_t i=0u; i<I; ++i)
        {
            pos1 = pos2 + 1u; pos2 = pos1;
            while (line.substr(pos2,1u)!=")" && line.substr(pos2,1u)!=",") { ++pos2; }
            inames.push_back(line.substr(pos1,pos2-pos1));
        }
    }
    else { Imin = 0; }
    if (Imin>I) { cerr << progstr+": " << __LINE__ << errstr << "Imin cannot be greater than I" << endl; return 1; }


    //Get additional argtable lines and anames
    getline(ifs,line);
    while (line.find("ofile",0)==string::npos)
    {
        cout << ind << line << endl;
        pos1 = line.find("*",0) + 1u;
        pos2 = line.find("=",0) - 1u;
        anames.push_back(line.substr(pos1,pos2-pos1)); ++A;
        getline(ifs,line);
    }


    //Get last argtable line and onames
    //getline(ifs,line);
    cout << ind << line << endl;
    pos1 = line.find("<file>",0) + 5u;
    while (line.substr(pos1,1u)!=",") { ++pos1; }
    ++pos1; pos2 = pos1 + 1u;
    while (line.substr(pos2,1u)!=",") { ++pos2; }
    if (line.substr(pos1,pos2-pos1)=="O") { Omin = O; }
    else if (line.substr(pos1,pos2-pos1)=="O-1") { if (O>0u) { Omin = O-1u; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-2") { if (O>1u) { Omin = O-2u; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-3") { if (O>2u) { Omin = O-3u; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-4") { if (O>3u) { Omin = O-4u; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-5") { if (O>4u) { Omin = O-5u; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-6") { if (O>5u) { Omin = O-6u; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else
    {
        s2i = stoi(line.substr(pos1,pos2-pos1));
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
        Omin = size_t(s2i);
    }
    pos1 = line.find("output file",0) + 10u;
    while (line.substr(pos1,1)!="(") { ++pos1; }
    pos2 = pos1;
    for (size_t o=0u; o<O; ++o)
    {
        pos1 = pos2 + 1u; pos2 = pos1;
        while (line.substr(pos2,1u)!=")" && line.substr(pos2,1u)!=",") { ++pos2; }
        onames.push_back(line.substr(pos1,pos2-pos1));
    }
    if (Omin>O) { cerr << progstr+": " << __LINE__ << errstr << "Omin cannot be greater than O" << endl; return 1; }


    //Finish argtable
    cout << ind << "struct arg_lit *a_help = arg_litn(\"h\",\"help\",0,1,\"display this help and exit\");" << endl;
    cout << ind << "struct arg_end  *a_end = arg_end(5);" << endl;
    cout << ind << "void *argtable[] = {";
    if (I>0u) { cout << "a_fi, "; }
    for (size_t a=0u; a<A; ++a) { cout << anames[a] << ", "; }
    cout << "a_fo, a_help, a_end};" << endl;
    cout << ind << "if (arg_nullcheck(argtable)!=0) { cerr << progstr+\": \" << __LINE__ << errstr << \"problem allocating argtable\" << endl; return 1; }" << endl;
    cout << ind << "nerrs = arg_parse(argc, argv, argtable);" << endl;
    cout << ind << "if (a_help->count>0)" << endl;
    cout << ind << "{" << endl;
    cout << ind+ind << "cout << \"Usage: \" << progstr; arg_print_syntax(stdout, argtable, \"\\n\");" << endl;
    cout << ind+ind << "cout << endl; arg_print_glossary(stdout, argtable, \"  %-25s %s\\n\");" << endl;
    cout << ind+ind << "cout << endl << descr; return 1;" << endl;
    cout << ind << "}" << endl;
    cout << ind << "if (nerrs>0) { arg_print_errors(stderr,a_end,(progstr+\": \"+to_string(__LINE__)+errstr).c_str()); return 1; }" << endl << endl;


    //Check stdin
    if (I>0u)
    {
        cout << endl;
        cout << ind << "//Check stdin" << endl;
        cout << ind << "stdi1 = (a_fi->count==0 || strlen(a_fi->filename[0])==0 || strcmp(a_fi->filename[0],\"-\")==0);" << endl;
        for (size_t i=1u; i<Imin+1; ++i)
        {
            cout << ind << "stdi" << i+1u << " = (a_fi->count<=" << i << " || strlen(a_fi->filename[" << i << "])==0 || strcmp(a_fi->filename[" << i << "],\"-\")==0);" << endl;
        }
        for (size_t i=Imin+1; i<I; ++i)
        {
            cout << ind << "if (a_fi->count>" << i << ") { stdi" << i+1u << " = (strlen(a_fi->filename[" << i << "])==0 || strcmp(a_fi->filename[" << i << "],\"-\")==0); }";
            cout << ind << "else { stdi" << i+1u << " = (!isatty(fileno(stdin)) && a_fi->count==" << i << " && stdi1";
            for (size_t o=1u; o<i; ++o) { cout << "+stdi" << o+1u; } cout << "==0); }" << endl;
        }
        if (I>1u)
        {
            cout << ind << "if (stdi1"; for (size_t i=1u; i<I; ++i) { cout << "+stdi" << i+1u; }
            cout << ">1) { cerr << progstr+\": \" << __LINE__ << errstr << \"can only use stdin for one input\" << endl; return 1; }" << endl;
        }
        cout << ind << "if (stdi1"; for (size_t i=1u; i<I; ++i) { cout << "+stdi" << i+1u; }
        cout << ">0 && isatty(fileno(stdin))) { cerr << progstr+\": \" << __LINE__ << errstr << \"no stdin detected\" << endl; return 1; }" << endl << endl;
    }


    //Check stdout
    cout << endl;
    cout << ind << "//Check stdout" << endl;
    if (O==0)
    {
        cout << ind << "stdo1 = (a_fo->count==0 || strlen(a_fo->filename[0])==0 || strcmp(a_fo->filename[0],\"-\")==0);" << endl;
    }
    else
    {
        cout << ind << "if (a_fo->count>0) { stdo1 = (strlen(a_fo->filename[0])==0 || strcmp(a_fo->filename[0],\"-\")==0); }" << endl;
        cout << ind << "else { stdo1 = (!isatty(fileno(stdout))); }" << endl;
    }
    for (size_t o=1u; o<O; ++o)
    {
        cout << ind << "if (a_fo->count>" << o << ") { stdo" << o+1u << " = (strlen(a_fo->filename[" << o << "])==0 || strcmp(a_fo->filename[" << o << "],\"-\")==0); }" << endl;
        cout << ind << "else { stdo" << o+1u << " = (!isatty(fileno(stdout)) && a_fo->count==" << o << " && stdo1";
        for (size_t i=1u; i<o; ++i) { cout << "+stdo" << i+1u; } cout << "==0); }" << endl;
    }
    if (O>1u)
    {
        cout << ind << "if (stdo1"; for (size_t o=1u; o<O; ++o) { cout << "+stdo" << o+1u; }
        cout << ">1) { cerr << progstr+\": \" << __LINE__ << errstr << \"can only use stdout for one output\" << endl; return 1; }" << endl;
    }
    cout << ind << "wo1 = (stdo1 || a_fo->count>0);";
    for (size_t o=1u; o<O; ++o) { cout << " wo" << o+1u << " = (stdo" << o+1u << " || a_fo->count>" << o << ");"; } cout << endl;
    cout << endl;


    //Open inputs
    if (I>0u)
    {
        cout << endl;
        cout << ind << "//Open input"; if (I>1u) { cout << "s"; } cout << endl;
        for (size_t i=0u; i<Imin+1; ++i)
        {
            cout << ind << "if (stdi" << i+1u << ") { ifs" << i+1u << ".copyfmt(cin); ifs" << i+1u << ".basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs" << i+1u << ".open(a_fi->filename[" << i << "]); }" << endl;
            cout << ind << "if (!ifs" << i+1u << ") { cerr << progstr+\": \" << __LINE__ << errstr << \"problem opening input file";
            if (I>1u) { cout << " " << i+1u; }
            cout << "\" << endl; return 1; }" << endl;
        }
        for (size_t i=Imin+1; i<I; ++i)
        {
            cout << ind << "if (stdi" << i+1u << " || a_fi->count>" << i << ")" << endl;
            cout << ind << "{" << endl;
            cout << ind+ind << "if (stdi" << i+1u << ") { ifs" << i+1u << ".copyfmt(cin); ifs" << i+1u << ".basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs" << i+1u << ".open(a_fi->filename[" << i << "]); }" << endl;
            cout << ind+ind << "if (stdi" << i+1u << " && ifs" << i+1u << ".peek()==EOF) { stdi" << i+1u << " = 0; }" << endl;
            cout << ind+ind << "else if (!ifs" << i+1u << ") { cerr << progstr+\": \" << __LINE__ << errstr << \"problem opening input file " << i+1u << "\" << endl; return 1; }" << endl;
            //cout << ind+ind << "if (!ifs" << i+1u << ") { cerr << progstr+\": \" << __LINE__ << errstr << \"problem opening input file " << i+1u << "\" << endl; return 1; }" << endl;
            cout << ind << "}" << endl;
        }
        cout << endl;
    }


    //Read input headers
    if (I>0u)
    {
        cout << endl;
        cout << ind << "//Read input header"; if (I>1u) { cout << "s"; } cout << endl;
        for (size_t i=0u; i<Imin+1; ++i)
        {
            cout << ind << "if (!read_input_header(ifs" << i+1u << ",i" << i+1u << ")) { cerr << progstr+\": \" << __LINE__ << errstr << \"problem reading header for input file";
            if (I>1u) { cout << " " << i+1u; }
            cout << "\" << endl; return 1; }" << endl;
        }
        for (size_t i=Imin+1; i<I; ++i)
        {
            cout << ind << "if (stdi" << i+1u << " || a_fi->count>" << i << ")" << endl;
            cout << ind << "{" << endl;
            cout << ind+ind << "if (!read_input_header(ifs" << i+1u << ",i" << i+1u << ")) { cerr << progstr+\": \" << __LINE__ << errstr << \"problem reading header for input file" << i+1u << "\" << endl; return 1; }" << endl;
            cout << ind << "}" << endl;
            cout << ind << "else { i" << i+1u << ".F = i1.F; i" << i+1u << ".T = i1.T; i" << i+1u << ".R = i" << i+1u << ".C = i" << i+1u << ".S = i" << i+1u << ".H = 1u; }" << endl;
        }
        cout << ind << "if ((i1.T==oktypes).sum()==0";
        for (size_t i=1u; i<I; ++i) { cout << " || (i" << i+1u << ".T==oktypes).sum()==0"; }
        cout << ")" << endl;
        cout << ind << "{" << endl;
        cout << ind+ind << "cerr << progstr+\": \" << __LINE__ << errstr << \"input data type must be in \" << \"{\";" << endl;
        cout << ind+ind << "for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1u]) ? \"}\" : \",\"); }" << endl;
        cout << ind+ind << "cerr << endl; return 1;" << endl;
        cout << ind << "}" << endl;
        cout << endl;
    }


    //Get options
    while (line.compare(0u,9u,"//Get opt")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Get options" << endl;
    cout << endl;
    getline(ifs,line); 
    while (line.compare(0u,8u,"//Checks")!=0)
    {
        getline(ifs,line);
        if (line.compare(0u,6u,"//Get ")==0)
        {
            while (line.size()>0u)
            {
                cout << ind << line << endl;
                getline(ifs,line);
            }
            cout << endl;
        }
    }


    //Checks
    while (line.compare(0u,8u,"//Checks")!=0) { getline(ifs,line); }
    getline(ifs,line);
    if (line.size()>0u)
    {
        cout << endl;
        cout << ind << "//Checks" << endl;
        while (line.size()>0u) { cout << ind << line << endl; getline(ifs,line); }
        cout << endl;
    }


    //Set output headers
    if (O>0u)
    {
        while (line.compare(0u,14u,"//Set output h")!=0) { getline(ifs,line); }
        cout << endl;
        cout << ind << "//Set output header info"; if (O>1u) { cout << "s"; } cout << endl;
        getline(ifs,line); 
        while (line.size()>0u) { cout << ind << line << endl; getline(ifs,line); }
        cout << endl;
    }


    //Open outputs
    cout << endl;
    cout << ind << "//Open output"; if (O>1u) { cout << "s"; } cout << endl;
    for (size_t o=0u; o==0 || o<O; ++o)
    {
        cout << ind << "if (wo" << o+1u << ")" << endl;
        cout << ind << "{" << endl;
        cout << ind+ind << "if (stdo" << o+1u << ") { ofs" << o+1u << ".copyfmt(cout); ofs" << o+1u << ".basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs" << o+1u << ".open(a_fo->filename[" << o << "]); }" << endl;
        cout << ind+ind << "if (!ofs" << o+1u << ") { cerr << progstr+\": \" << __LINE__ << errstr << \"problem opening output file " << o+1u << "\" << endl; return 1; }" << endl;
        cout << ind << "}" << endl;
    }
    cout << endl;


    //Write output hdrs
    if (O>0 && a_oh->count==0)
    {
        cout << endl;
        cout << ind << "//Write output header"; if (O>1u) { cout << "s"; } cout << endl;
        for (size_t o=0u; o<O; ++o)
        {
            cout << ind << "if (wo" << o+1u << " && !write_output_header(ofs" << o+1u << ",o" << o+1u << ")) { cerr << progstr+\": \" << __LINE__ << errstr << \"problem writing header for output file " << o+1u << "\" << endl; return 1; }" << endl;
        }
        cout << endl;
    }


    //Other prep
    while (line.compare(0u,7u,"//Other")!=0 && line.compare(0u,9u,"//Process")!=0) { getline(ifs,line); }
    if (line.compare(0u,7u,"//Other")==0)
    {
        cout << endl;
        cout << ind << "//Other prep" << endl;
        getline(ifs,line);
        while (line.size()==0u) { cout << endl; getline(ifs,line); }
        while (line.compare(0u,9,"//Process")!=0) { cout << ind << line << endl; getline(ifs,line); }
    }


    //Process start
    while (line.compare(0u,9u,"//Process")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Process" << endl;
    if (a_t->count>0) { cout << ind << "clock_gettime(CLOCK_REALTIME,&tic);" << endl; }
    if (I>0u || O>0u)
    {
        getline(ifs,line);
        cout << ind << line << endl;
        
        //Get t, ttstr and zi
        pos1 = line.find("=",0) + 2; pos2 = line.find(")",0);
        s2i = stoi(line.substr(pos1,pos2-pos1));
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
        t = size_t(s2i);
        if (t!=oktypes[0]) { cerr << progstr+": " << __LINE__ << errstr << "type not member of oktypes" << endl; return 1; }
        if (t<4) { do_float = true; }
        else if (t<100) { do_int = true; }
        else { do_complex = true; }
        pos1 = line.find("(",0) + 1; pos2 = line.find("=",0);
        ttstr = line.substr(pos1,pos2-pos1);
        zi = zros.at(t); oi = ones.at(t);

        //Do float blocks (and direct-sub int and complex blocks)
        t = 0;
        if (do_float)
        {
            tistr = typs.at(oktypes[t]);

            //Write tmpfile and first block
            getline(ifs,line);
            while (line.size()>0u && line.find("else if ("+ttstr+"==")==string::npos)
            {
                fputs((ind+line+"\n").c_str(),tmpf);
                cout << ind << line << endl;
                getline(ifs,line);
            }

            //Check if extra int or complex block included
            if (line.size()>0u)
            {
                if (line.find("else if ("+ttstr+"==8u)")!=string::npos) { do_int = true; }
                else if (line.find("else if ("+ttstr+"==101u)")!=string::npos) { do_complex = true; }
                else { cerr << progstr+": " << __LINE__ << errstr << "int case must be for data type 8, complex case must be for data type 101" << endl; return 1; }
            }
            else { do_int = do_complex = false; }

            //Write float blocks
            ++t;
            while (t<T && (!do_int || int(oktypes[t])<4) && (!do_complex || int(oktypes[t])<100))
            {
                cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << ")" << endl;
                tcstr = typs.at(oktypes[t]);
                zc = zros.at(oktypes[t]); oc = ones.at(oktypes[t]);
                rewind(tmpf);
                while (fgets(buff,256*16,tmpf))
                {
                    line = string(buff);
                    n = 0;
                    while ((n=line.find(tistr,n))!=string::npos)
                    {
                        line.replace(n,tistr.size(),tcstr);
                        n += tcstr.size();
                    }
                    n = 0;
                    while ((n=line.find(zi,n))!=string::npos)
                    {
                        line.replace(n,zi.size(),zc);
                        n += zc.size();
                    }
                    n = 0;
                    while ((n=line.find(oi,n))!=string::npos)
                    {
                        line.replace(n,oi.size(),oc);
                        n += oc.size();
                    }
                    if (a_dz->count>0 && oktypes[t]==2)
                    {
                        if ((n=line.find("blas_is",0))!=string::npos) { line.replace(n,7,"blas_id"); }
                        else if ((n=line.find("blas_s",0))!=string::npos) { line.replace(n,6,"blas_d"); }
                        else if (a_ov->count>0 && (n=line.find("_s(",0))!=string::npos) { line.replace(n,3,"_d("); }
                        else if (a_ov->count>0 && (n=line.find("_s (",0))!=string::npos) { line.replace(n,4,"_d ("); }
                        else if (a_ov->count>0 && (n=line.find("_s_",0))!=string::npos) { line.replace(n,3,"_d_"); }
                    }
                    cout << line;
                }
                ++t;
            }
        }

        //Do int blocks
        if (do_int)
        {
            tistr = typs.at(oktypes[t]);
            if (do_float) { cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << "u)" << endl; }
            else { t = 0; }

            //Write int tmpfile and first block
            getline(ifs,line);
            while (line.size()>0u && line.find("else if ("+ttstr+"==")==string::npos)
            {
                fputs((ind+line+"\n").c_str(),tmpf8);
                cout << ind << line << endl;
                getline(ifs,line);
            }

            //Check if extra complex block included
            if (line.size()>0u)
            {
                if (line.find("else if ("+ttstr+"==101u)")!=string::npos) { do_complex = true; }
                else { cerr << progstr+": " << __LINE__ << errstr << "complex case must be for data type 101" << endl; return 1; }
            }
            else { do_complex = false; }

            //Write int blocks
            ++t;
            while (t<T && (!do_complex || int(oktypes[t])<100))
            {
                cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << "u)" << endl;
                tcstr = typs.at(oktypes[t]);
                rewind(tmpf8);
                while (fgets(buff,256*16,tmpf8))
                {
                    line = string(buff);
                    n = 0;
                    while ((n=line.find(tistr,n))!=string::npos)
                    {
                        if (line.find(tistr+"*>",n)>line.size()) { line.replace(n,tistr.size(),tcstr); }
                        n += tcstr.size();
                    }
                    n = 0;
                    while ((n=line.find(zi,n))!=string::npos)
                    {
                        line.replace(n,zi.size(),zc);
                        n += zc.size();
                    }
                    n = 0;
                    while ((n=line.find(oi,n))!=string::npos)
                    {
                        line.replace(n,oi.size(),oc);
                        n += oc.size();
                    }
                    cout << line;
                }
                ++t;
            }
        }

        //Do complex blocks
        if (do_complex)
        {
            tistr = typs.at(oktypes[t]-100u);
            if (do_float || do_int) { cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << "u)" << endl; }
            else { t = 0; }

            //Write complex tmpfile and first block
            getline(ifs,line);
            while (line.size()>0u)
            {
                fputs((ind+line+"\n").c_str(),tmpf101);
                cout << ind << line << endl;
                getline(ifs,line);
            }

            //Write complex blocks
            ++t;
            while (t<T)
            {
                cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << "u)" << endl;
                tcstr = typs.at(oktypes[t]-100);
                zc = zros.at(oktypes[t]); oc = ones.at(oktypes[t]);
                rewind(tmpf101);
                while (fgets(buff,256*16,tmpf101))
                {
                    line = string(buff);
                    n = 0;
                    while ((n=line.find(tistr,n))!=string::npos)
                    {
                        line.replace(n,tistr.size(),tcstr);
                        n += tcstr.size();
                    }
                    n = 0;
                    while ((n=line.find(zi,n))!=string::npos)
                    {
                        line.replace(n,zi.size(),zc);
                        n += zc.size();
                    }
                    n = 0;
                    while ((n=line.find(oi,n))!=string::npos)
                    {
                        line.replace(n,oi.size(),oc);
                        n += oc.size();
                    }
                    if (a_dz->count>0 && oktypes[t]==102)
                    {
                        if ((n=line.find("blas_csca",0))!=string::npos) { line.replace(n,9,"blas_zsca"); }
                        else if ((n=line.find("blas_css",0))!=string::npos) { line.replace(n,8,"blas_zds"); }
                        else if ((n=line.find("blas_csy",0))!=string::npos) { line.replace(n,8,"blas_zsy"); }
                        else if ((n=line.find("blas_scc",0))!=string::npos) { line.replace(n,8,"blas_dzc"); }
                        else if ((n=line.find("blas_scs",0))!=string::npos) { line.replace(n,8,"blas_dzs"); }
                        else if ((n=line.find("blas_sc",0))!=string::npos) { line.replace(n,7,"blas_dz"); }
                        else if ((n=line.find("blas_cs",0))!=string::npos) { line.replace(n,7,"blas_zd"); }
                        else if ((n=line.find("blas_ic",0))!=string::npos) { line.replace(n,7,"blas_iz"); }
                        else if ((n=line.find("blas_c",0))!=string::npos) { line.replace(n,6,"blas_z"); }
                        else if (a_ov->count>0 && (n=line.find("_c(",0))!=string::npos) { line.replace(n,3,"_z("); }
                        else if (a_ov->count>0 && (n=line.find("_c (",0))!=string::npos) { line.replace(n,4,"_z ("); }
                        else if (a_ov->count>0 && (n=line.find("_c_",0))!=string::npos) { line.replace(n,3,"_z_"); }
                    }
                    cout << line;
                }
                ++t;
            }
        }


        //Write else clause
        cout << ind << "else" << endl;
        cout << ind << "{" << endl;
        cout << ind << ind << "cerr << progstr+\": \" << __LINE__ << errstr << \"data type not supported\" << endl; return 1;" << endl;
        cout << ind << "}" << endl;
    }
    if (a_t->count>0)
    {
        cout << ind << "clock_gettime(CLOCK_REALTIME,&toc);" << endl;
        cout << ind << "cerr << \"elapsed time = \" << (toc.tv_sec-tic.tv_sec)*1e3 + (toc.tv_nsec-tic.tv_nsec)/1e6 << \" ms\" << endl;" << endl;
    }
    cout << ind << endl;


    //Finish
    while (line.compare(0u,8u,"//Finish")!=0) { getline(ifs,line); }
    getline(ifs,line);
    if (line.size()>0u)
    {
        cout << endl;
        cout << ind << "//Finish" << endl;
        while (line.size()>0u) { cout << ind << line << endl; getline(ifs,line); }
        cout << endl;
    }


    //Exit
    cout << endl;
    cout << ind << "//Exit" << endl;
    if (a_T->count>0)
    {
        cout << ind << "clock_gettime(CLOCK_REALTIME,&toc);" << endl;
        cout << ind << "cerr << \"elapsed time = \" << (toc.tv_sec-tic.tv_sec)*1e3 + (toc.tv_nsec-tic.tv_nsec)/1e6 << \" ms\" << endl;" << endl;
    }
    cout << ind << "return ret;" << endl;
    cout << "}" << endl;
    cout << endl;


    //Return
	return ret;
}

