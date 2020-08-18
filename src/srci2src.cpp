//@author Erik Edwards
//@date 2019


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
    size_t i, I, Imin, o, O, Omin, t, T, a, A = 0;
    int s2i;

    const vector<size_t> types = {0,1,2,3,8,9,10,16,17,32,33,64,65,101,102,103};
    const unordered_map<size_t,string> zros = {{1,"0.0f"},{2,"0.0"},{3,"0.0L"},{8,"'\0'"},{9,"'\0'"},{10,"false"},{16,"0"},{17,"0u"},{32,"0"},{33,"0u"},{64,"0l"},{65,"0ul"},{101,"0.0f"},{102,"0.0"},{103,"0.0L"}};
    const unordered_map<size_t,string> ones = {{1,"1.0f"},{2,"1.0"},{3,"1.0L"},{8,"'\1'"},{9,"'\1'"},{10,"true"},{16,"1"},{17,"1u"},{32,"1"},{33,"1u"},{64,"1l"},{65,"1ul"},{101,"1.0f"},{102,"1.0"},{103,"1.0L"}};
    const unordered_map<size_t,string> aftyps = {{1,"f32"},{2,"f64"},{8,"b8"},{9,"u8"},{10,"b8"},{16,"s16"},{17,"u16"},{32,"s32"},{33,"u32"},{64,"s64"},{65,"u64"},{101,"c32"},{102,"c64"}};
    unordered_map<size_t,string> fmts = {{0,"0"},{1,"1"},{65,"65"},{101,"101"},{102,"102"},{147,"147"}};
    unordered_map<size_t,string> typs = {{0,"txt"},{1,"float"},{2,"double"},{3,"long double"},{8,"int8_t"},{9,"uint8_t"},{10,"bool"},{16,"int16_t"},{17,"uint16_t"},{32,"int32_t"},{33,"uint32_t"},{64,"int64_t"},{65,"uint64_t"},{101,"complex<float>"},{102,"complex<double>"},{103,"complex<long double>"}};
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
    cout << "//@date 2019-present" << endl;
    getline(ifs,line);
    while (line.size()>0 && line.compare(0,9,"//Include")!=0) { cout << line << endl; getline(ifs,line); }
    cout << endl;


    //Includes
    while (line.compare(0,9,"//Include")!=0) { getline(ifs,line); }
    cout << endl;
    if (tictoc || a_t->count>0 || a_T->count>0) { cout << "#include <ctime>" << endl; }
    for (i=0; i<includes.size(); i++) { cout << "#include " << includes[i] << endl; }
    getline(ifs,line);
    while (line.size()>0 && line.compare(0,14,"//Declarations")!=0) { cout << line << endl; getline(ifs,line); }
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
    while (line.compare(0,14,"//Declarations")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Declarations" << endl;
    cout << ind << "int ret = 0;" << endl;
    cout << ind << "const string errstr = \": \\033[1;31merror:\\033[0m \";" << endl;
    cout << ind << "const string warstr = \": \\033[1;35mwarning:\\033[0m \";" << endl;
    cout << ind << "const string progstr(__FILE__,string(__FILE__).find_last_of(\"/\")+1,strlen(__FILE__)-string(__FILE__).find_last_of(\"/\")-5);" << endl;
    

    //Get okfmts (only sometimes present)
    getline(ifs,line); //this line must have okfmts
    if (line.size()>36 && line.compare(0,34,"const valarray<uint8_t> okfmts = {")==0) { cout << ind << line << endl; getline(ifs,line); }


    //Get oktypes
    //this line must have oktypes
    if (line.size()<37 || line.compare(0,35,"const valarray<uint8_t> oktypes = {")!=0) { cerr << progstr+": " << __LINE__ << errstr << "problem with line for oktypes" << endl; return 1; }
    cout << ind << line << endl;
    pos1 = line.find_first_of("{",0) + 1;
    pos2 = line.find_first_of("}",0) - 1;
    oktypestr = line.substr(pos1,pos2-pos1+1);
    T = 1; for (c=0; c<oktypestr.size(); c++) { if (oktypestr.substr(c,1)==",") { T++; } }
    t = prevc = 0;
    for (c=0; c<oktypestr.size(); c++)
    {
        if (oktypestr.substr(c,1)=="," || oktypestr.substr(c,1)==" ")
        {
            s2i = stoi(oktypestr.substr(prevc,c-prevc));
            if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
            else { oktypes.push_back(size_t(s2i)); }
            prevc = c + 1; t++;
        }
    }
    s2i = stoi(oktypestr.substr(prevc,c-prevc));
    if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
    oktypes.push_back(size_t(s2i));
    for (t=0; t<T; t++) { oktypestrs.push_back(typs.at(oktypes[t])); }


    //Declare maps
    //cout << ind << "const unordered_map<uint32_t,std::streamsize> szs = {{0,2},{1,4},{2,8},{3,16},{8,1},{9,1},{10,1},{16,2},{17,2},{32,4},{33,4},{64,8},{65,8},{101,8},{102,16},{103,32}};" << endl;


    //Get I and O
    getline(ifs,line); //this line must have I and O
    if (line.size()<8 || line.find("size_t ")==string::npos) { cerr << progstr+": " << __LINE__ << errstr << "problem with line for I and O" << endl; return 1; }
    pos1 = line.find("=",0) + 2; pos2 = line.find(",",0) - 1;
    s2i = stoi(line.substr(pos1,pos2-pos1+1));
    if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
    I = size_t(s2i);
    pos1 = line.find("O",0) + 4; pos2 = line.find(";",0) - 1;
    s2i = stoi(line.substr(pos1,pos2-pos1+1));
    if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
    O = size_t(s2i);
    if (I>0 || O>0) { cout << ind << "const size_t "; }
    if (I>0) { cout << "I = " << I; }
    if (O>0) { if (I>0) { cout << ", "; } cout << "O = " << O; }
    cout << ";" << endl;


    //Declarations continue
    if (I==0)
    {
        cout << ind << "ofstream ofs1"; for (o=1; o<O; o++) { cout << ", ofs" << o+1; } cout << ";" << endl;
        cout << ind << "int8_t stdo1";
        for (o=1; o<O; o++) { cout << ", stdo" << o+1; }
        for (o=0; o==0 || o<O; o++) { cout << ", wo" << o+1; }
        cout << ";"; cout << endl;
        if (O>0) { cout << ind << "ioinfo o1"; for (o=1; o<O; o++) { cout << ", o" << o+1; } cout << ";" << endl; }
    }
    else
    {
        cout << ind << "ifstream ifs1";
        for (i=1; i<I; i++) { cout << ", ifs" << i+1; } cout << "; ";
        cout << "ofstream ofs1"; for (o=1; o<O; o++) { cout << ", ofs" << o+1; } cout << ";" << endl;
        cout << ind << "int8_t stdi1";
        for (i=1; i<I; i++) { cout << ", stdi" << i+1; }
        for (o=0; o==0 || o<O; o++) { cout << ", stdo" << o+1; }
        for (o=0; o==0 || o<O; o++) { cout << ", wo" << o+1; }
        cout << ";"; cout << endl;
        cout << ind << "ioinfo i1";
        for (i=1; i<I; i++) { cout << ", i" << i+1; }
        for (o=0; o<O; o++) { cout << ", o" << o+1; }
        cout << ";" << endl;
    }
    getline(ifs,line);
    while (line.size()>0 && line.compare(0,13,"//Description")!=0) { cout << ind << line << endl; getline(ifs,line); }
    cout << endl;


    //Description
    while (line.compare(0,13,"//Description")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Description" << endl;
    getline(ifs,line);
    while (line.size()>0 && line.compare(0,1," ")!=0) { cout << ind << line << endl; getline(ifs,line); }
    cout << endl;


    //Argtable start
    while (line.compare(0,10,"//Argtable")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Argtable" << endl;
    cout << ind << "int nerrs;" << endl;


    //Get 1st argtable line and Imin and inames
    if (I>0)
    {
        getline(ifs,line);
        cout << ind << line << endl;
        pos1 = line.find("<file>",0) + 5;
        while (line.substr(pos1,1)!=",") { pos1++; }
        pos1++; pos2 = pos1 + 1;
        while (line.substr(pos2,1)!=",") { pos2++; }
        if (line.substr(pos1,pos2-pos1)=="I") { Imin = I; }
        else if (line.substr(pos1,pos2-pos1)=="I-1") { if (I>0) { Imin = I-1; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-2") { if (I>1) { Imin = I-2; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-3") { if (I>2) { Imin = I-3; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-4") { if (I>3) { Imin = I-4; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-5") { if (I>4) { Imin = I-5; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else if (line.substr(pos1,pos2-pos1)=="I-6") { if (I>5) { Imin = I-6; } else { cerr << progstr+": " << __LINE__ << errstr << "Imin expression evals to negative" << endl; return 1; } }
        else
        {
            s2i = stoi(line.substr(pos1,pos2-pos1));
            if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
            Imin = size_t(s2i);
        }
        pos1 = line.find("input file",0) + 10;
        while (line.substr(pos1,1)!="(") { pos1++; }
        pos2 = pos1;
        for (i=0; i<I; i++)
        {
            pos1 = pos2 + 1; pos2 = pos1;
            while (line.substr(pos2,1)!=")" && line.substr(pos2,1)!=",") { pos2++; }
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
        pos1 = line.find("*",0) + 1;
        pos2 = line.find("=",0) - 1;
        anames.push_back(line.substr(pos1,pos2-pos1)); A++;
        getline(ifs,line);
    }


    //Get last argtable line and onames
    //getline(ifs,line);
    cout << ind << line << endl;
    pos1 = line.find("<file>",0) + 5;
    while (line.substr(pos1,1)!=",") { pos1++; }
    pos1++; pos2 = pos1 + 1;
    while (line.substr(pos2,1)!=",") { pos2++; }
    if (line.substr(pos1,pos2-pos1)=="O") { Omin = O; }
    else if (line.substr(pos1,pos2-pos1)=="O-1") { if (O>0) { Omin = O-1; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-2") { if (O>1) { Omin = O-2; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-3") { if (O>2) { Omin = O-3; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-4") { if (O>3) { Omin = O-4; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-5") { if (O>4) { Omin = O-5; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else if (line.substr(pos1,pos2-pos1)=="O-6") { if (O>5) { Omin = O-6; } else { cerr << progstr+": " << __LINE__ << errstr << "Omin expression evals to negative" << endl; return 1; } }
    else
    {
        s2i = stoi(line.substr(pos1,pos2-pos1));
        if (s2i<0) { cerr << progstr+": " << __LINE__ << errstr << "stoi returned negative int" << endl; return 1; }
        Omin = size_t(s2i);
    }
    pos1 = line.find("output file",0) + 10;
    while (line.substr(pos1,1)!="(") { pos1++; }
    pos2 = pos1;
    for (o=0; o<O; o++)
    {
        pos1 = pos2 + 1; pos2 = pos1;
        while (line.substr(pos2,1)!=")" && line.substr(pos2,1)!=",") { pos2++; }
        onames.push_back(line.substr(pos1,pos2-pos1));
    }
    if (Omin>O) { cerr << progstr+": " << __LINE__ << errstr << "Omin cannot be greater than O" << endl; return 1; }


    //Finish argtable
    cout << ind << "struct arg_lit *a_help = arg_litn(\"h\",\"help\",0,1,\"display this help and exit\");" << endl;
    cout << ind << "struct arg_end  *a_end = arg_end(5);" << endl;
    cout << ind << "void *argtable[] = {";
    if (I>0) { cout << "a_fi, "; }
    for (a=0; a<A; a++) { cout << anames[a] << ", "; }
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
    if (I>0)
    {
        cout << endl;
        cout << ind << "//Check stdin" << endl;
        cout << ind << "stdi1 = (a_fi->count==0 || strlen(a_fi->filename[0])==0 || strcmp(a_fi->filename[0],\"-\")==0);" << endl;
        for (i=1; i<Imin+1; i++)
        {
            cout << ind << "stdi" << i+1 << " = (a_fi->count<=" << i << " || strlen(a_fi->filename[" << i << "])==0 || strcmp(a_fi->filename[" << i << "],\"-\")==0);" << endl;
        }
        for (i=Imin+1; i<I; i++)
        {
            cout << ind << "if (a_fi->count>" << i << ") { stdi" << i+1 << " = (strlen(a_fi->filename[" << i << "])==0 || strcmp(a_fi->filename[" << i << "],\"-\")==0); }";
            cout << ind << "else { stdi" << i+1 << " = (!isatty(fileno(stdin)) && a_fi->count==" << i << " && stdi1";
            for (o=1; o<i; o++) { cout << "+stdi" << o+1; } cout << "==0); }" << endl;
        }
        if (I>1)
        {
            cout << ind << "if (stdi1"; for (i=1; i<I; i++) { cout << "+stdi" << i+1; }
            cout << ">1) { cerr << progstr+\": \" << __LINE__ << errstr << \"can only use stdin for one input\" << endl; return 1; }" << endl;
        }
        cout << ind << "if (stdi1"; for (i=1; i<I; i++) { cout << "+stdi" << i+1; }
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
    for (o=1; o<O; o++)
    {
        cout << ind << "if (a_fo->count>" << o << ") { stdo" << o+1 << " = (strlen(a_fo->filename[" << o << "])==0 || strcmp(a_fo->filename[" << o << "],\"-\")==0); }" << endl;
        cout << ind << "else { stdo" << o+1 << " = (!isatty(fileno(stdout)) && a_fo->count==" << o << " && stdo1";
        for (i=1; i<o; i++) { cout << "+stdo" << i+1; } cout << "==0); }" << endl;
    }
    if (O>1)
    {
        cout << ind << "if (stdo1"; for (o=1; o<O; o++) { cout << "+stdo" << o+1; }
        cout << ">1) { cerr << progstr+\": \" << __LINE__ << errstr << \"can only use stdout for one output\" << endl; return 1; }" << endl;
    }
    cout << ind << "wo1 = (stdo1 || a_fo->count>0);";
    for (o=1; o<O; o++) { cout << " wo" << o+1 << " = (stdo" << o+1 << " || a_fo->count>" << o << ");"; } cout << endl;
    cout << endl;


    //Open inputs
    if (I>0)
    {
        cout << endl;
        cout << ind << "//Open input"; if (I>1) { cout << "s"; } cout << endl;
        for (i=0; i<Imin+1; i++)
        {
            cout << ind << "if (stdi" << i+1 << ") { ifs" << i+1 << ".copyfmt(cin); ifs" << i+1 << ".basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs" << i+1 << ".open(a_fi->filename[" << i << "]); }" << endl;
            cout << ind << "if (!ifs" << i+1 << ") { cerr << progstr+\": \" << __LINE__ << errstr << \"problem opening input file";
            if (I>1) { cout << " " << i+1; }
            cout << "\" << endl; return 1; }" << endl;
        }
        for (i=Imin+1; i<I; i++)
        {
            cout << ind << "if (stdi" << i+1 << " || a_fi->count>" << i << ")" << endl;
            cout << ind << "{" << endl;
            cout << ind+ind << "if (stdi" << i+1 << ") { ifs" << i+1 << ".copyfmt(cin); ifs" << i+1 << ".basic_ios<char>::rdbuf(cin.rdbuf()); } else { ifs" << i+1 << ".open(a_fi->filename[" << i << "]); }" << endl;
            cout << ind+ind << "if (stdi" << i+1 << " && ifs" << i+1 << ".peek()==EOF) { stdi" << i+1 << " = 0; }" << endl;
            cout << ind+ind << "else if (!ifs" << i+1 << ") { cerr << progstr+\": \" << __LINE__ << errstr << \"problem opening input file " << i+1 << "\" << endl; return 1; }" << endl;
            //cout << ind+ind << "if (!ifs" << i+1 << ") { cerr << progstr+\": \" << __LINE__ << errstr << \"problem opening input file " << i+1 << "\" << endl; return 1; }" << endl;
            cout << ind << "}" << endl;
        }
        cout << endl;
    }


    //Read input headers
    if (I>0)
    {
        cout << endl;
        cout << ind << "//Read input header"; if (I>1) { cout << "s"; } cout << endl;
        for (i=0; i<Imin+1; i++)
        {
            cout << ind << "if (!read_input_header(ifs" << i+1 << ",i" << i+1 << ")) { cerr << progstr+\": \" << __LINE__ << errstr << \"problem reading header for input file";
            if (I>1) { cout << " " << i+1; }
            cout << "\" << endl; return 1; }" << endl;
        }
        for (i=Imin+1; i<I; i++)
        {
            cout << ind << "if (stdi" << i+1 << " || a_fi->count>" << i << ")" << endl;
            cout << ind << "{" << endl;
            cout << ind+ind << "if (!read_input_header(ifs" << i+1 << ",i" << i+1 << ")) { cerr << progstr+\": \" << __LINE__ << errstr << \"problem reading header for input file" << i+1 << "\" << endl; return 1; }" << endl;
            cout << ind << "}" << endl;
            cout << ind << "else { i" << i+1 << ".F = i1.F; i" << i+1 << ".T = i1.T; i" << i+1 << ".R = i" << i+1 << ".C = i" << i+1 << ".S = i" << i+1 << ".H = 1u; }" << endl;
        }
        cout << ind << "if ((i1.T==oktypes).sum()==0";
        for (i=1; i<I; i++) { cout << " || (i" << i+1 << ".T==oktypes).sum()==0"; }
        cout << ")" << endl;
        cout << ind << "{" << endl;
        cout << ind+ind << "cerr << progstr+\": \" << __LINE__ << errstr << \"input data type must be in \" << \"{\";" << endl;
        cout << ind+ind << "for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1]) ? \"}\" : \",\"); }" << endl;
        cout << ind+ind << "cerr << endl; return 1;" << endl;
        cout << ind << "}" << endl;
        cout << endl;
    }


    //Get options
    while (line.compare(0,9,"//Get opt")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Get options" << endl;
    cout << endl;
    getline(ifs,line); 
    while (line.compare(0,8,"//Checks")!=0)
    {
        getline(ifs,line);
        if (line.compare(0,6,"//Get ")==0)
        {
            while (line.size()>0)
            {
                cout << ind << line << endl;
                getline(ifs,line);
            }
            cout << endl;
        }
    }


    //Checks
    while (line.compare(0,8,"//Checks")!=0) { getline(ifs,line); }
    getline(ifs,line);
    if (line.size()>0)
    {
        cout << endl;
        cout << ind << "//Checks" << endl;
        while (line.size()>0) { cout << ind << line << endl; getline(ifs,line); }
        cout << endl;
    }


    //Set output headers
    if (O>0)
    {
        while (line.compare(0,14,"//Set output h")!=0) { getline(ifs,line); }
        cout << endl;
        cout << ind << "//Set output header info"; if (O>1) { cout << "s"; } cout << endl;
        getline(ifs,line); 
        while (line.size()>0) { cout << ind << line << endl; getline(ifs,line); }
        cout << endl;
    }


    //Open outputs
    cout << endl;
    cout << ind << "//Open output"; if (O>1) { cout << "s"; } cout << endl;
    for (o=0; o==0 || o<O; o++)
    {
        cout << ind << "if (wo" << o+1 << ")" << endl;
        cout << ind << "{" << endl;
        cout << ind+ind << "if (stdo" << o+1 << ") { ofs" << o+1 << ".copyfmt(cout); ofs" << o+1 << ".basic_ios<char>::rdbuf(cout.rdbuf()); } else { ofs" << o+1 << ".open(a_fo->filename[" << o << "]); }" << endl;
        cout << ind+ind << "if (!ofs" << o+1 << ") { cerr << progstr+\": \" << __LINE__ << errstr << \"problem opening output file " << o+1 << "\" << endl; return 1; }" << endl;
        cout << ind << "}" << endl;
    }
    cout << endl;


    //Write output hdrs
    if (O>0 && a_oh->count==0)
    {
        cout << endl;
        cout << ind << "//Write output header"; if (O>1) { cout << "s"; } cout << endl;
        for (o=0; o<O; o++)
        {
            cout << ind << "if (wo" << o+1 << " && !write_output_header(ofs" << o+1 << ",o" << o+1 << ")) { cerr << progstr+\": \" << __LINE__ << errstr << \"problem writing header for output file " << o+1 << "\" << endl; return 1; }" << endl;
        }
        cout << endl;
    }


    //Other prep
    while (line.compare(0,7,"//Other")!=0 && line.compare(0,9,"//Process")!=0) { getline(ifs,line); }
    if (line.compare(0,7,"//Other")==0)
    {
        cout << endl;
        cout << ind << "//Other prep" << endl;
        getline(ifs,line);
        while (line.size()==0) { cout << endl; getline(ifs,line); }
        while (line.compare(0,9,"//Process")!=0) { cout << ind << line << endl; getline(ifs,line); }
    }


    //Process start
    while (line.compare(0,9,"//Process")!=0) { getline(ifs,line); }
    cout << endl;
    cout << ind << "//Process" << endl;
    if (a_t->count>0) { cout << ind << "clock_gettime(CLOCK_REALTIME,&tic);" << endl; }
    if (I>0 || O>0)
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
            while (line.size()>0 && line.find("else if ("+ttstr+"==")==string::npos)
            {
                fputs((ind+line+"\n").c_str(),tmpf);
                cout << ind << line << endl;
                getline(ifs,line);
            }

            //Check if extra int or complex block included
            if (line.size()>0)
            {
                if (line.find("else if ("+ttstr+"==8)")!=string::npos) { do_int = true; }
                else if (line.find("else if ("+ttstr+"==101)")!=string::npos) { do_complex = true; }
                else { cerr << progstr+": " << __LINE__ << errstr << "int case must be for data type 8, complex case must be for data type 101" << endl; return 1; }
            }
            else { do_int = do_complex = false; }

            //Write float blocks
            t++;
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
                t++;
            }
        }

        //Do int blocks
        if (do_int)
        {
            tistr = typs.at(oktypes[t]);
            if (do_float) { cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << ")" << endl; }
            else { t = 0; }

            //Write int tmpfile and first block
            getline(ifs,line);
            while (line.size()>0 && line.find("else if ("+ttstr+"==")==string::npos)
            {
                fputs((ind+line+"\n").c_str(),tmpf8);
                cout << ind << line << endl;
                getline(ifs,line);
            }

            //Check if extra complex block included
            if (line.size()>0)
            {
                if (line.find("else if ("+ttstr+"==101)")!=string::npos) { do_complex = true; }
                else { cerr << progstr+": " << __LINE__ << errstr << "complex case must be for data type 101" << endl; return 1; }
            }
            else { do_complex = false; }

            //Write int blocks
            t++;
            while (t<T && (!do_complex || int(oktypes[t])<100))
            {
                cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << ")" << endl;
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
                t++;
            }
        }

        //Do complex blocks
        if (do_complex)
        {
            tistr = typs.at(oktypes[t]-100);
            if (do_float || do_int) { cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << ")" << endl; }
            else { t = 0; }

            //Write complex tmpfile and first block
            getline(ifs,line);
            while (line.size()>0)
            {
                fputs((ind+line+"\n").c_str(),tmpf101);
                cout << ind << line << endl;
                getline(ifs,line);
            }

            //Write complex blocks
            t++;
            while (t<T)
            {
                cout << ind << "else if (" << ttstr << "==" << int(oktypes[t]) << ")" << endl;
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
                t++;
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
    while (line.compare(0,8,"//Finish")!=0) { getline(ifs,line); }
    getline(ifs,line);
    if (line.size()>0)
    {
        cout << endl;
        cout << ind << "//Finish" << endl;
        while (line.size()>0) { cout << ind << line << endl; getline(ifs,line); }
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

