//@author Erik Edwards
//@date 2019


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <argtable2.h>
#include <cctype>
#include <valarray>
#include <unordered_map>
#include <vector>
#include <complex>
#include <ctime>
#include "/home/erik/codee/cmli/cmli.hpp"


int main(int argc, char *argv[])
{
	using namespace std;
    //timespec tic, toc; clock_gettime(CLOCK_REALTIME,&tic);
	
	
	//Declarations
	int ret = 0;
	const string errstr = ": \033[1;31merror:\033[0m ";
	//const string warstr = ": \033[1;32mwarning:\033[0m ";
    const string progstr(__FILE__,string(__FILE__).find_last_of("/")+1,strlen(__FILE__)-string(__FILE__).find_last_of("/")-5);
	const valarray<uint8_t> oktypes = {1,2};
    const unordered_map<uint32_t,size_t> szs = {{0,2},{1,4},{2,8},{3,16},{8,1},{9,1},{10,1},{16,2},{17,2},{32,4},{33,4},{64,8},{65,8},{101,8},{102,16},{103,32}};
	const size_t I = 1, O = 1;
	ifstream ifs1; ofstream ofs1;
    int8_t stdi1, stdo1;
    ioinfo i1, o1;
	uint32_t r, c, D, N, cnt;
    char tmp;

	
	//Declarations for input file 1
	vector<float> X1;
	

	//Description
    string descr;
	descr += "Converts the Kaldi ark file X to a cmli-format binary file.\n";
	descr += "If no inputs, then assumes that input is piped in from stdin.\n";
    descr += "\n";
    descr += "Use -d (--dim) to specify the number of features to output [default=all].\n";
	descr += "This will be the number of columns in the output.\n";
    descr += "\n";
    descr += "Use -n (--N) to specify the number of frames to output [default=all].\n";
	descr += "This will be the number of rows in the output.\n";
    descr += "\n";
	descr += "Use -t (--type) to specify the output data type [default=2].\n";
	descr += "Here (for Kaldi purposes), this must be 1 (single) or 2 (double).\n";
	descr += "The Kaldi input itself is always single.\n";
    descr += "\n";
    descr += "Use -f (--fmt) to give the output file format.\n";
    descr += "This can be 1 (arrayfire), 65 (arma), 101 (cmli row major),\n";
    descr += "102 (cmli col major), or 147 (npy), with default from input.\n";
	descr += "\n";
	descr += "Examples:\n";
	descr += "$ kaldi2bin X.ark -o X.bin \n";
	descr += "$ kaldi2bin X.ark > X.bin \n";
	descr += "$ cat X.ark | kaldi2bin > X.bin \n";
    descr += "$ kaldi2bin X.ark -d5 > X.bin \n";


    //Argtable
	int nerrs;
	struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file");
    struct arg_int  *a_dim = arg_intn("d","dim","<uint>",0,1,"num features to output [default=all]");
    struct arg_int    *a_N = arg_intn("n","N","<uint>",0,1,"num frames to output [default=all]");
	struct arg_int *a_otyp = arg_intn("t","type","<uint>",0,1,"output data type [default=2]");
    struct arg_int *a_ofmt = arg_intn("f","fmt","<uint>",0,1,"output file format [default=102]");
	struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file");
	struct arg_lit *a_help = arg_litn("h","help",0,1,"display this help and exit");
	struct arg_end  *a_end = arg_end(5);
	void *argtable[] = {a_fi, a_dim, a_N, a_otyp, a_ofmt, a_fo, a_help, a_end};
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
    stdi1 = (a_fi->count<=0 || strlen(a_fi->filename[0])==0 || strcmp(a_fi->filename[0],"-")==0);
    if (stdi1>0 && isatty(fileno(stdin))) { cerr << progstr+": " << __LINE__ << errstr << "no stdin detected" << endl; return 1; }


    //Check stdout
    stdo1 = (a_fo->count==0 || strlen(a_fo->filename[0])==0 || strcmp(a_fo->filename[0],"-")==0);


    //Open input
    if (stdi1==0) { ifs1.open(a_fi->filename[0]); } else { ifs1.copyfmt(cin); ifs1.basic_ios<char>::rdbuf(cin.rdbuf()); }
    if (!ifs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input file" << endl; return 1; }


    //Get options


    //Get o1.F
	if (a_ofmt->count==0) { o1.F = 102; }
	else if (a_ofmt->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be nonnegative" << endl; return 1; }
    else if (a_ofmt->ival[0]>255) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be < 256" << endl; return 1; }
	else { o1.F = uint8_t(a_ofmt->ival[0]); }


    //Get o1.T
	if (a_otyp->count==0) { o1.T = 2; }
	else if (a_otyp->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "output data type must be positive int" << endl; return 1; }
	else if (a_otyp->ival[0]>103) { cerr << progstr+": " << __LINE__ << errstr << "output data type must be <= 103" << endl; return 1; }
	else { o1.T = uint8_t(a_otyp->ival[0]); }
	if ((o1.T==oktypes).sum()==0)
    {
        cerr << progstr+": " << __LINE__ << errstr << "input data type must be in " << "{";
        for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1]) ? "}" : ","); }
        cerr << endl; return 1;
    }


    //Get dim
	if (a_dim->count==0) { D = 0; }
	else if (a_dim->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "dim must be a non-negative int" << endl; return 1; }
	else { D = uint32_t(a_dim->ival[0]); }


    //Get N
	if (a_N->count==0) { N = 0; }
	else if (a_N->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "N must be a non-negative int" << endl; return 1; }
	else { N = uint32_t(a_N->ival[0]); }


    //Find BFM string
    cnt = 0u;
    while (cnt<5u && ifs1 && !ifs1.eof())
    {
        try { ifs1.read(&tmp,1); }
	    catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem finding BFM in ark input" << endl; return 1; }
        if (tmp==66) { cnt=1u; }
        else if (tmp==70 && cnt==1u ) { cnt=2u; }
        else if (tmp==77 && cnt==2u ) { cnt=3u; }
        else if (tmp==32 && cnt==3u ) { cnt=4u; }
        else if (tmp==4 && cnt==4u) { cnt=5u; }
        else { cnt=0u; }
    }
    if (!ifs1 || ifs1.eof()) { cerr << progstr+": " << __LINE__ << errstr << "didn't find BFM in ark input" << endl; return 1; }


    //Read R
    try { ifs1.read(reinterpret_cast<char*>(&o1.R),4); }
	catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading R (num frames)" << endl; return 1; }
    if (N>o1.R) { cerr << progstr+": " << __LINE__ << errstr << "N cannot be greater than R" << endl; return 1; }
    else if (N==0) { N = o1.R; }


    //Char btwn R and C
    try { ifs1.read(&tmp,1); }
	catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input char between R and C" << endl; return 1; }


    //Read C
    try { ifs1.read(reinterpret_cast<char*>(&o1.C),4); }
	catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading C (feat D)" << endl; return 1; }
    if (D>o1.C) { cerr << progstr+": " << __LINE__ << errstr << "dim cannot be greater than C" << endl; return 1; }
    else if (D==0) { D = o1.C; }
    if (!ifs1 || ifs1.eof()) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input ark file header" << endl; return 1; }


    //Set data sizes
	o1.S = o1.H = 1u;
	
	
	//Open output
	if (stdo1==0) { ofs1.open(a_fo->filename[0], ofstream::binary); }
	else { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); }
	if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file" << endl; return 1; }
	
	
	//Write output header
	if (!write_output_header(ofs1,o1)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file" << endl; return 1; }


    //Read input and write output
    if (o1.T==1)
    {
        valarray<float> Xr(0.0f,o1.C);
        for (r=0u; r<o1.R; r++)
        {
            try { ifs1.read(reinterpret_cast<char*>(&Xr[0]),i1.sz()*i1.C); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file at row " << r << endl; return 1; }
            if (r<N)
            {
                try { ofs1.write(reinterpret_cast<char*>(&Xr[0]),o1.sz()*D); }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file data at row " << r << endl; return 1; }
            }
        }
    }
    else if (o1.T==2)
    {
        valarray<float> Xr(0.0f,o1.C); valarray<double> Yr(0.0,D);
        for (r=0u; r<o1.R; r++)
        {
            try { ifs1.read(reinterpret_cast<char*>(&Xr[0]),i1.sz()*i1.C); }
            catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file at row " << r << endl; return 1; }
            if (r<N)
            {
                for (c=0u; c<D; c++) { Yr[c] = double(Xr[c]); }
                try { ofs1.write(reinterpret_cast<char*>(&Yr[0]),o1.sz()*D); }
                catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing output file data at row " << r << endl; return 1; }
            }
        }
    }
    else
	{
		cerr << progstr+": " << __LINE__ << errstr << "output data type not recognized" << endl; return 1;
	}


    //Return
    //clock_gettime(CLOCK_REALTIME,&toc); cerr << "elapsed time = " << (toc.tv_sec-tic.tv_sec)*1e3 + (toc.tv_nsec-tic.tv_nsec)/1e6 << " ms" << endl;
	return ret;
}

