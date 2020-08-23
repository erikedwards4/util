//@author Erik Edwards
//@date 2018-present
//@license BSD 3-clause


#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <argtable2.h>
#include <cctype>
#include <cfloat>
#include <cstdint>
#include <valarray>
#include <unordered_map>
#include <vector>
#include <complex>
#include <ctime>
#include "../util/cmli.hpp"


int main(int argc, char *argv[])
{
	using namespace std;
    //timespec tic, toc; clock_gettime(CLOCK_REALTIME,&tic);
	
	
	//Declarations
	int ret = 0;
	const string errstr = ": \033[1;31merror:\033[0m ";
	//const string warstr = ": \033[1;32mwarning:\033[0m ";
    const string progstr(__FILE__,string(__FILE__).find_last_of("/")+1,strlen(__FILE__)-string(__FILE__).find_last_of("/")-5);
	const valarray<size_t> okfmts = {0u,1u,65u,101u,102u,147u,148u};
	const valarray<size_t> oktypes = {1u,2u,3u,8u,9u,16u,17u,32u,33u,64u,65u,101u,102u,103u};
	//const unordered_map<size_t,std::streamsize> szs = {{0,2},{1,4},{2,8},{3,16},{8,1},{9,1},{10,1},{16,2},{17,2},{32,4},{33,4},{64,8},{65,8},{101,8},{102,16},{103,32}};
	const size_t I = 1u, O = 1u;
	ifstream ifs1; ofstream ofs1;
    int8_t stdi1, stdo1;
	ioinfo o1;
	size_t r, c, s, h;
	size_t p1, p2, cma;
	int64_t s2i; long double s2ld, s2ldi;

	
	//Declarations for input file
	string field1, line1;
	istringstream iss1;
	vector<string> strvec;
	vector<float> X1; vector<double> X2; vector<long double> X3;
	vector<complex<float>> X101; vector<complex<double>> X102; vector<complex<long double>> X103;
	vector<int8_t> X8; vector<uint8_t> X9; vector<int16_t> X16; vector<uint16_t> X17;
	vector<int32_t> X32; vector<uint32_t> X33; vector<int64_t> X64; vector<uint64_t> X65;
	

	//Description
    string descr;
	descr += "Converts the ascii text file X to a cmli-format binary file.\n";
	descr += "If no inputs, then assumes that input is piped in from stdin.\n";
	descr += "\n";
	descr += "The output file format is CMLI column-major (fmt=102) by default.\n";
	descr += "\n";
	descr += "The output data type can be specified using -t (--type).\n";
	descr += "\n";
	descr += "Examples:\n";
	descr += "$ txt2bin X.txt -o X.bin \n";
	descr += "$ txt2bin X.txt > X.bin \n";
	descr += "$ echo \"1.1 -2.2 3.3\" | txt2bin > X.bin \n";
	descr += "$ echo \"1.1 -2.2 3.3\" | txt2bin -t1 > X.bin \n";
	descr += "$ echo \"(1.1,0) (1,-2.2) (3,3)\" | txt2bin -t101 > X.bin \n";
	
	
	//Argtable
	int nerrs;
	struct arg_file  *a_fi = arg_filen(nullptr,nullptr,"<file>",I-1,I,"input file");
	struct arg_int *a_otyp = arg_intn("t","type","<uint>",0,1,"output data type [default=1]");
	struct arg_int *a_ofmt = arg_intn("f","fmt","<uint>",0,1,"output file format [default=147]");
	struct arg_file  *a_fo = arg_filen("o","ofile","<file>",0,O,"output file");
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
    stdi1 = (a_fi->count<=0 || strlen(a_fi->filename[0])==0 || strcmp(a_fi->filename[0],"-")==0);
    if (stdi1>0 && isatty(fileno(stdin))) { cerr << progstr+": " << __LINE__ << errstr << "no stdin detected" << endl; return 1; }


    //Check stdout
    stdo1 = (a_fo->count<=0 || strlen(a_fo->filename[0])==0 || strcmp(a_fo->filename[0],"-")==0);


    //Open input
    if (stdi1==0) { ifs1.open(a_fi->filename[0]); } else { ifs1.copyfmt(cin); ifs1.basic_ios<char>::rdbuf(cin.rdbuf()); }
    if (!ifs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening input file" << endl; return 1; }


	//Get o1.F
	if (a_ofmt->count==0) { o1.F = 147u; }
	else if (a_ofmt->ival[0]<0) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be nonnegative" << endl; return 1; }
	else if (a_ofmt->ival[0]>255) { cerr << progstr+": " << __LINE__ << errstr << "output file format must be < 256" << endl; return 1; }
	else { o1.F = size_t(a_ofmt->ival[0]); }
	if ((o1.F==okfmts).sum()==0)
	{
		cerr << progstr+": " << __LINE__ << errstr << "output file format must be in " << "{";
		for (auto o : okfmts) { cerr << int(o) << ((o==okfmts[okfmts.size()-1]) ? "}" : ","); }
		cerr << endl; return 1;
	}
	
	
	//Get o1.T
	if (a_otyp->count==0) { o1.T = 1u; }
	else if (a_otyp->ival[0]<1) { cerr << progstr+": " << __LINE__ << errstr << "output data type must be positive" << endl; return 1; }
	else if (a_otyp->ival[0]>103) { cerr << progstr+": " << __LINE__ << errstr << "output data type must be <= 103" << endl; return 1; }
	else { o1.T = size_t(a_otyp->ival[0]); }
	if ((o1.T==oktypes).sum()==0)
    {
        cerr << progstr+": " << __LINE__ << errstr << "input data type must be in " << "{";
        for (auto o : oktypes) { cerr << int(o) << ((o==oktypes[oktypes.size()-1]) ? "}" : ","); }
        cerr << endl; return 1;
    }
	
	
	//Read 1st line of input
	o1.R = o1.C = 0u;
	if (getline(ifs1,line1))
	{
		if (!ifs1) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file at row " << o1.R << endl; return 1; }
		try { iss1.str(line1); }
		catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file at row " << o1.R << endl; return 1; }
		
		switch (o1.T)
		{
			case 1:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2ld = stold(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					if (s2ld<-static_cast<long double>(FLT_MAX) || s2ld>static_cast<long double>(FLT_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 1 (float) at row " << o1.R << endl; return 1; }
					X1.push_back(float(s2ld));
				}
				break;
			case 2:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2ld = stold(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					if (s2ld<-static_cast<long double>(DBL_MAX) || s2ld>static_cast<long double>(DBL_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 2 (double) at row " << o1.R << endl; return 1; }
					X2.push_back(double(s2ld));
				}
				break;
			case 3:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2ld = stold(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					X3.push_back(s2ld);
				}
				break;
			case 8:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<INT8_MIN || s2i>INT8_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 8 (int8_t) at row " << o1.R << endl; return 1; }
					X8.push_back(int8_t(s2i));
				}
				break;
			case 9:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<0 || s2i>UINT8_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 9 (uint8_t) at row " << o1.R << endl; return 1; }
					X9.push_back(uint8_t(s2i));
				}
				break;
			case 16:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<INT16_MIN || s2i>INT16_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 16 (int16_t) at row " << o1.R << endl; return 1; }
					X16.push_back(int16_t(s2i));
				}
				break;
			case 17:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<0 || s2i>UINT16_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 17 (uint16_t) at row " << o1.R << endl; return 1; }
					X17.push_back(uint16_t(s2i));
				}
				break;
			case 32:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<INT32_MIN || s2i>INT32_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 32 (int32_t) at row " << o1.R << endl; return 1; }
					X32.push_back(int32_t(s2i));
				}
				break;
			case 33:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<0 || s2i>UINT32_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 33 (uint32_t) at row " << o1.R << endl; return 1; }
					X33.push_back(uint32_t(s2i));
				}
				break;
			case 64:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { X64.push_back(stoll(field1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
				}
				break;
			case 65:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					try { X65.push_back(stoull(field1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
				}
				break;
			case 101:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					p1 = field1.find("("); p2 = field1.rfind(")"); cma = field1.find(",");
					if (p1>field1.length() || p2>field1.length() || cma>field1.length())
					{
						cerr << progstr+": " << __LINE__ << errstr << "complex data must use (real,imag) style" << endl; return 1; 
					}
					try { s2ld = stold(field1.substr(p1+1,cma-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					try { s2ldi = stold(field1.substr(cma+1,p2-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					if (s2ld<-static_cast<long double>(FLT_MAX) || s2ld>static_cast<long double>(FLT_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 1 (float) at row " << o1.R << endl; return 1; }
					if (s2ldi<-static_cast<long double>(FLT_MAX) || s2ldi>static_cast<long double>(FLT_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 1 (float) at row " << o1.R << endl; return 1; }
					X101.push_back({float(s2ld),float(s2ldi)});
				}
				break;
			case 102:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					p1 = field1.find("("); p2 = field1.rfind(")"); cma = field1.find(",");
					if (p1>field1.length() || p2>field1.length() || cma>field1.length())
					{
						cerr << progstr+": " << __LINE__ << errstr << "complex data must use (real,imag) style" << endl; return 1; 
					}
					try { s2ld = stold(field1.substr(p1+1,cma-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					try { s2ldi = stold(field1.substr(cma+1,p2-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					if (s2ld<-static_cast<long double>(DBL_MAX) || s2ld>static_cast<long double>(DBL_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 2 (double) at row " << o1.R << endl; return 1; }
					if (s2ldi<-static_cast<long double>(DBL_MAX) || s2ldi>static_cast<long double>(DBL_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 2 (double) at row " << o1.R << endl; return 1; }
					X102.push_back({double(s2ld),double(s2ldi)});
				}
				break;
			case 103:
				while (!iss1.eof())
				{
					iss1 >> field1; ++o1.C;
					p1 = field1.find("("); p2 = field1.rfind(")"); cma = field1.find(",");
					if (p1>field1.length() || p2>field1.length() || cma>field1.length())
					{
						cerr << progstr+": " << __LINE__ << errstr << "complex data must use (real,imag) style" << endl; return 1; 
					}
					try { s2ld = stold(field1.substr(p1+1,cma-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					try { s2ldi = stold(field1.substr(cma+1,p2-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					X103.push_back({s2ld,s2ldi});
				}
				break;
			default:
				cerr << progstr+": " << __LINE__ << errstr << "requested output data type (" << int(o1.T) << ") not recognized" << endl; return 1;
		}
		//if (!iss1) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file at row " << o1.R << endl; return 1; }
		o1.R++;
	}
	
	
	//Read input
	while (getline(ifs1, line1))
	{
		if (!ifs1) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file at row " << o1.R << endl; return 1; }
		
		iss1.clear(); iss1.str(line1);
		if (!iss1) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file at row " << o1.R << endl; return 1; }
		
		c = 0u;
		switch (o1.T)
		{
			case 1:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2ld = stold(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					if (s2ld<-static_cast<long double>(FLT_MAX) || s2ld>static_cast<long double>(FLT_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 1 (float) at row " << o1.R << endl; return 1; }
					X1.push_back(float(s2ld));
				}
				break;
			case 2:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2ld = stold(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					if (s2ld<-static_cast<long double>(DBL_MAX) || s2ld>static_cast<long double>(DBL_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 2 (double) at row " << o1.R << endl; return 1; }
					X2.push_back(double(s2ld));
				}
				break;
			case 3:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2ld = stold(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					X3.push_back(s2ld);
				}
				break;
			case 8:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<INT8_MIN || s2i>INT8_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 8 (int8_t) at row " << o1.R << endl; return 1; }
					X8.push_back(int8_t(s2i));
				}
				break;
			case 9:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<0 || s2i>UINT8_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 9 (uint8_t) at row " << o1.R << endl; return 1; }
					X9.push_back(uint8_t(s2i));
				}
				break;
			case 16:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<INT16_MIN || s2i>INT16_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 16 (int16_t) at row " << o1.R << endl; return 1; }
					X16.push_back(int16_t(s2i));
				}
				break;
			case 17:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<0 || s2i>UINT16_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 17 (uint16_t) at row " << o1.R << endl; return 1; }
					X17.push_back(uint16_t(s2i));
				}
				break;
			case 32:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<INT32_MIN || s2i>INT32_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 32 (int32_t) at row " << o1.R << endl; return 1; }
					X32.push_back(int32_t(s2i));
				}
				break;
			case 33:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { s2i = stoll(field1); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
					if (s2i<0 || s2i>UINT32_MAX) { cerr << progstr+": " << __LINE__ << errstr << "int is out of range for data type 33 (uint32_t) at row " << o1.R << endl; return 1; }
					X33.push_back(uint32_t(s2i));
					//try { X33.push_back(stoul(field1)); }
					//catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
				}
				break;
			case 64:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { X64.push_back(stoll(field1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
				}
				break;
			case 65:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					try { X65.push_back(stoull(field1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to int at row " << o1.R << endl; return 1; }
				}
				break;
			case 101:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					p1 = field1.find("("); p2 = field1.rfind(")"); cma = field1.find(",");
					if (p1>field1.length() || p2>field1.length() || cma>field1.length())
					{
						cerr << progstr+": " << __LINE__ << errstr << "complex data must use (real,imag) style" << endl; return 1; 
					}
					try { s2ld = stold(field1.substr(p1+1,cma-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					try { s2ldi = stold(field1.substr(cma+1,p2-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					if (s2ld<-static_cast<long double>(FLT_MAX) || s2ld>static_cast<long double>(FLT_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 1 (float) at row " << o1.R << endl; return 1; }
					if (s2ldi<-static_cast<long double>(FLT_MAX) || s2ldi>static_cast<long double>(FLT_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 1 (float) at row " << o1.R << endl; return 1; }
					X101.push_back({float(s2ld),float(s2ldi)});
				}
				break;
			case 102:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					p1 = field1.find("("); p2 = field1.rfind(")"); cma = field1.find(",");
					if (p1>field1.length() || p2>field1.length() || cma>field1.length())
					{
						cerr << progstr+": " << __LINE__ << errstr << "complex data must use (real,imag) style" << endl; return 1; 
					}
					try { s2ld = stold(field1.substr(p1+1,cma-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					try { s2ldi = stold(field1.substr(cma+1,p2-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					if (s2ld<-static_cast<long double>(DBL_MAX) || s2ld>static_cast<long double>(DBL_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 2 (double) at row " << o1.R << endl; return 1; }
					if (s2ldi<-static_cast<long double>(DBL_MAX) || s2ldi>static_cast<long double>(DBL_MAX)) { cerr << progstr+": " << __LINE__ << errstr << "num is out of range for data type 2 (double) at row " << o1.R << endl; return 1; }
					X102.push_back({double(s2ld),double(s2ldi)});
				}
				break;
			case 103:
				while (!iss1.eof())
				{
					iss1 >> field1; ++c;
					p1 = field1.find("("); p2 = field1.rfind(")"); cma = field1.find(",");
					if (p1>field1.length() || p2>field1.length() || cma>field1.length())
					{
						cerr << progstr+": " << __LINE__ << errstr << "complex data must use (real,imag) style" << endl; return 1; 
					}
					try { s2ld = stold(field1.substr(p1+1,cma-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					try { s2ldi = stold(field1.substr(cma+1,p2-1)); }
					catch (...) { cerr << progstr+": " << __LINE__ << errstr << "could not convert str to num at row " << o1.R << endl; return 1; }
					X103.push_back({s2ld,s2ldi});
				}
				break;
			default:
				cerr << progstr+": " << __LINE__ << errstr << "requested data type (" << o1.T << ") not recognized" << endl; return 1;
		}
		//if (!iss1) { cerr << progstr+": " << __LINE__ << errstr << "problem reading input file at row " << o1.R << endl; return 1; }
		if (c!=o1.C) { cerr << progstr+": " << __LINE__ << errstr << "problem with input file (ncol differs across rows)" << endl; return 1; }
		
		o1.R++;
	}


	//Set data sizes
	o1.S = o1.H = 1u;
	
	
	//Open output
	if (stdo1==0) { ofs1.open(a_fo->filename[0], ofstream::binary); }
	else { ofs1.copyfmt(cout); ofs1.basic_ios<char>::rdbuf(cout.rdbuf()); }
	if (!ofs1) { cerr << progstr+": " << __LINE__ << errstr << "problem opening output file" << endl; return 1; }
	
	
	//Write output header
	if (!write_output_header(ofs1,o1)) { cerr << progstr+": " << __LINE__ << errstr << "problem writing header for output file" << endl; return 1; }


	//Other prep
	
	
	//Write output data
	switch (o1.T)
	{
		case 1:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X1[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X1[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 2:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X2[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X2[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 3:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X3[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X3[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 8:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X8[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X8[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 9:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X9[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X9[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 16:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X16[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X16[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 17:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X17[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X17[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 32:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X32[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X32[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 33:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X33[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X33[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 64:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X64[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X64[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 65:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X65[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X65[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 101:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X101[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X101[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 102:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X102[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X102[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		case 103:
			if (o1.iscolmajor())
			{
				for (h=0u; h<o1.H; h++)
				{
					for (s=0u; s<o1.S; s++)
					{
						for (c=0u; c<o1.C; c++)
						{
							for (r=0u; r<o1.R; r++)
							{
								try { ofs1.write(reinterpret_cast<char*>(&X103[h+s*o1.H+c*o1.S*o1.H+r*o1.C*o1.S*o1.H]),o1.sz()); }
								catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
							}
						}
					}
				}
			}
			else
			{
				try { ofs1.write(reinterpret_cast<char*>(&X103[0]),o1.nbytes()); }
				catch (...) { cerr << progstr+": " << __LINE__ << errstr << "problem writing data for output file" << endl; return 1; }
			}
			break;
		default:
			cerr << progstr+": " << __LINE__ << errstr << "requested data type (" << int(o1.T) << ") not recognized" << endl; return 1;
	}
	
	
	//Return
    //clock_gettime(CLOCK_REALTIME,&toc); cerr << "elapsed time = " << (toc.tv_sec-tic.tv_sec)*1e3 + (toc.tv_nsec-tic.tv_nsec)/1e6 << " ms" << endl;
	return ret;
}

