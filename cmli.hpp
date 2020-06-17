#ifndef CMLI_H_
#define CMLI_H_


class ioinfo
{
    public:
        uint8_t F = 102, T = 2;
        uint32_t R = 1u, C = 1u, S = 1u, H = 1u;
        bool only_3D() { return (F==65); }
        bool isrowmajor() { return (F==101 || F==147); }
        bool iscolmajor() { return (F==1 || F==65 || F==80 || F==102 || F==148); }
        bool isreal() { return (T<100); }
        bool iscomplex() { return (T>99); }
        bool isbool() { return (T==10); }
        bool isint() { return (T>3 && T<100); }
        bool isuint() { return (T==9 || T==17 || T==33 || T==65); }
        bool isempty() { return (R==0u || C==0u || S==0u || H==0u); }
        bool isscalar() { return (R==1u && C==1u && S==1u && H==1u); }
        bool iscolvec() { return (C==1u && S==1u && H==1u); }
        bool isrowvec() { return (R==1u && S==1u && H==1u); }
        bool isslicevec() { return (R==1u && C==1u && H==1u); }
        bool ishyperslicevec() { return (R==1u && C==1u && S==1u); }
        bool isvec() { return ((R==1u && C==1u && S==1u) || (R==1u && C==1u && H==1u) || (R==1u && S==1u && H==1u) || (C==1u && S==1u && H==1u)); }
        //bool isvec() { return ((R==1u || C==1u) && S==1u && H==1u); }
        bool ismat() { return (S==1u && H==1u); }
        bool iscube() { return (H==1u); }
        bool issquare() { return (R==C); }
        uint32_t N() { return(R*C*S*H); }
        uint32_t sz()
        {
            if (T==8 || T==9 || T==10) { return 1u; }
            else if (T==0 || T==16 || T==17) { return 2u; }
            else if (T==1 || T==32 || T==33 || T==100) { return 4u; }
            else if (T==2 || T==64 || T==65 || T==101) { return 8u; }
            else if (T==3 || T==102) { return 16u; }
            else if (T==103) { return 32u; }
            else { return 0u; }
        }
        uint32_t nbytes()
        {
            if (T==8 || T==9 || T==10) { return R*C*S*H; }
            else if (T==0 || T==16 || T==17) { return 2u*R*C*S*H; }
            else if (T==1 || T==32 || T==33 || T==100) { return 4u*R*C*S*H; }
            else if (T==2 || T==64 || T==65 || T==101) { return 8u*R*C*S*H; }
            else if (T==3 || T==102) { return 16u*R*C*S*H; }
            else if (T==103) { return 32u*R*C*S*H; }
            else { return 0u; }
        }
};


inline bool same_size(ioinfo &i1, ioinfo &i2) { return (i1.R==i2.R && i1.C==i2.C && i1.S==i2.S && i1.H==i2.H); }


inline bool same_major(ioinfo &i1, ioinfo &i2) { return (i1.iscolmajor()==i2.iscolmajor() && i1.isrowmajor()==i2.isrowmajor()); }


inline bool major_compat(ioinfo &i1, ioinfo &i2) { return (same_major(i1,i2) || i1.isvec() || i2.isvec()); }

inline bool major_compat(ioinfo &i1, ioinfo &i2, ioinfo &i3) { return (major_compat(i1,i2) && major_compat(i1,i3) && major_compat(i2,i3)); }

inline bool major_compat(ioinfo &i1, ioinfo &i2, ioinfo &i3, ioinfo &i4)
{
    return (major_compat(i1,i2) && major_compat(i1,i3) && major_compat(i1,i4) && major_compat(i2,i3) && major_compat(i2,i4) && major_compat(i3,i4));
}

inline bool major_compat(ioinfo &i1, ioinfo &i2, ioinfo &i3, ioinfo &i4, ioinfo &i5)
{
    return (major_compat(i1,i2) && major_compat(i1,i3) && major_compat(i1,i4) && major_compat(i1,i5) && major_compat(i2,i3) && major_compat(i2,i4) && major_compat(i2,i5) && major_compat(i3,i4) && major_compat(i3,i5) && major_compat(i4,i5));
}


inline bool matmul_compat(ioinfo &i1, ioinfo &i2) { return (i1.C==i2.R); }


inline bool bcast_compat(ioinfo &i1, ioinfo &i2)
{
    return ((i1.R==1u || i2.R==1u || i1.R==i2.R) && (i1.C==1u || i2.C==1u || i1.C==i2.C) && (i1.S==1u || i2.S==1u || i1.S==i2.S) && (i1.H==1u || i2.H==1u || i1.H==i2.H));
}


//inline void swap_row_col_major(float *X1, ioinfo &i1)


inline bool read_input_header(std::ifstream &ifs, ioinfo &ii)
{
    using namespace std;
    const unordered_map<uint8_t,uint8_t> morder {{0,1},{1,1},{65,1},{101,0},{102,1},{147,0},{148,1}};
    int pk, s2i;
    
    if (!ifs) { return false; }
    
    try { pk = ifs.peek(); } catch (...) { std::cerr << "cmli read_input_header: peek unsuccessful" << std::endl; return false; }
    if (pk==EOF) { std::cerr << "cmli read_input_header: peek of input finds EOF" << std::endl; return false; }
    //std::cerr << "pk = " << pk << std::endl;
    //try { ifs.read(reinterpret_cast<char*>(Fi),sizeof(uint8_t)); } catch (...) { return false; }
    //std::cerr << "Fi = " << *Fi << std::endl;

    if (pk==1)  //ArrayFire (.af)
    {
        const unordered_map<char,uint8_t> typs = {{0,1},{2,2},{7,9},{4,8},{10,16},{11,17},{5,32},{6,33},{8,64},{9,65},{1,101},{3,102}};
        int32_t narrays = 0, keylength;
        char key, dtype;
        int64_t offset, D[4];
        try { ifs.read(reinterpret_cast<char*>(&ii.F),sizeof(uint8_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&narrays),sizeof(int32_t)); } catch (...) { return false; }
        if (narrays!=1) { std::cerr << "cmli read_input_header: num arrays must be 1 for arrayfire" << std::endl; return false; }
        try { ifs.read(reinterpret_cast<char*>(&keylength),sizeof(int32_t)); } catch (...) { return false; }
        try { ifs.read(&key,keylength); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&offset),sizeof(int64_t)); } catch (...) { return false; }
        try { ifs.read(&dtype,sizeof(char)); } catch (...) { return false; }
        try { ii.T = typs.at(dtype); } catch (...) { std::cerr << "cmli read_input_header: data type not supported for arrayfire" << std::endl; return false; }
        //std::cerr << "dtype = " << (int)dtype << std::endl; std::cerr << "*Ti = " << (int)*Ti << std::endl;
        try { ifs.read(reinterpret_cast<char*>(&D[0]),4*sizeof(int64_t)); } catch (...) { return false; }
        if (D[0]<0 || D[0]>4294967295) { std::cerr << "cmli read_input_header: nrows must be in [0 4294967295]" << std::endl; return false; }
        if (D[1]<0 || D[1]>4294967295) { std::cerr << "cmli read_input_header: ncols must be in [0 4294967295]" << std::endl; return false; }
        if (D[2]<0 || D[2]>4294967295) { std::cerr << "cmli read_input_header: nslices must be in [0 4294967295]" << std::endl; return false; }
        if (D[3]<0 || D[3]>4294967295) { std::cerr << "cmli read_input_header: nhyperslices must be in [0 4294967295]" << std::endl; return false; }
        //if (D[3]!=1) { std::cerr << "4D tensors not supported for arrayfire" << std::endl; return false; }
        ii.R = uint32_t(D[0]); ii.C = uint32_t(D[1]); ii.S = uint32_t(D[2]); ii.H = uint32_t(D[3]);
    }
    else if (pk==65)   //Armadillo (.arma)
    {
        string line;
        size_t pos1, pos2;
        const unordered_map<string,uint8_t> typs = {{"FN004",1},{"FN008",2},{"IS001",8},{"IU001",9},{"IS002",16},{"IU002",17},{"IS004",32},{"IU004",33},{"IS008",64},{"IU008",65},{"FC008",101},{"FC016",102}};
        try { getline(ifs,line); } catch (...) { return false; }
        if (line.compare(0,13,"ARMA_CUB_BIN_")!=0 && line.compare(0,13,"ARMA_MAT_BIN_")!=0) { return false; }
        ii.F = uint8_t(pk);
        pos1 = line.find_last_of("_")+1; pos2 = line.size();
        try { ii.T = typs.at(line.substr(pos1,pos2-pos1)); } catch (...) { return false; }
        try { getline(ifs,line); } catch (...) { return false; }
        try { s2i = stoi(line,&pos1); } catch (...) { return false; }
        if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
        ii.R = uint32_t(s2i);
        try { s2i = stoi(line.substr(pos1),&pos2); } catch (...) { return false; }
        if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
        ii.C = uint32_t(s2i);
        if (pos1+pos2>=line.size()) { ii.S = 1u; }
        else
        {
            try { s2i = stoi(line.substr(pos1+pos2)); } catch (...) { return false; }
            if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
            ii.S = uint32_t(s2i);
        }
        ii.H = 1u;
    }
    else if (pk==80)   //PyTorch
    {
        string line;
        // while (ifs)
        // {
        //     try { getline(ifs,line); } catch (...) { return false; }
        //     std::cerr << "line = " << line << std::endl;
        // }
        std::cerr << "PyTorch file format not supported (use convert util)" << std::endl; return false;
    }
    else if (pk==101 || pk==102)  //CMLI standard header (also for Eigen)
    {
        try { ifs.read(reinterpret_cast<char*>(&ii.F),sizeof(uint8_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.T),sizeof(uint8_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.R),sizeof(uint32_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.C),sizeof(uint32_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.S),sizeof(uint32_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.H),sizeof(uint32_t)); } catch (...) { return false; }
    }
    else if (pk==147)   //NumPy (.npy) (will set Fi to 148 if fortran_order found true)
    {
        const unordered_map<string,uint8_t> fmts = {{"False",147},{"True",148},{"false",147},{"true",148},{"FALSE",147},{"TRUE",148},{"F",147},{"T",148},{"f",147},{"t",148}};
        const unordered_map<string,uint8_t> typs = {{"f4",1},{"f8",2},{"f16",3},{"i1",8},{"u1",9},{"b1",10},{"i2",16},{"u2",16},{"i4",32},{"u4",33},{"i8",64},{"u8",65},{"c8",101},{"c16",102},{"c32",103}};
        char mstr[6], vstr[2];
        //int version;
        uint16_t HDR_LEN = 0;
        string line;
        size_t pos1, pos2, d = 0, nd;

        try { ifs.read(mstr,6); } catch (...) { std::cerr << "cmli read_input_header: read of numpy magic string unsuccessful" << std::endl; return false; }
        try { ifs.read(vstr,2); } catch (...) { std::cerr << "cmli read_input_header: read of numpy version string unsuccessful" << std::endl; return false; }
        //version = int(vstr[0]);
        try { ifs.read(reinterpret_cast<char*>(&HDR_LEN),sizeof(uint16_t)); } catch (...) { std::cerr << "cmli read_input_header: read of numpy HDR_LEN unsuccessful" << std::endl; return false; }
        //std::cerr << "HDR_LEN = " << HDR_LEN << std::endl;
        try { getline(ifs,line); } catch (...) { std::cerr << "cmli read_input_header: read of numpy HDR line unsuccessful" << std::endl; return false; }
        //std::cerr << "line=" << line << std::endl;
        
        pos1 = line.find("descr");
        if (pos1>=line.size()) { std::cerr << "cmli read_input_header: didn't find 'descr' key in numpy HDR" << std::endl; return false; }
        pos2 = line.find_first_of(":",pos1) + 1;
        pos1 = line.find_first_of("'",pos2) + 2; pos2 = line.find_first_of("'",pos1);
        try { ii.T = typs.at(line.substr(pos1,pos2-pos1)); }
        catch (...) { std::cerr << "cmli read_input_header: dtype str not recognized or not supported for numpy" << std::endl; return false; }
        
        pos2 = line.find("fortran_order");
        if (pos2>=line.size()) { std::cerr << "cmli read_input_header: didn't find 'fortran_order' key in numpy HDR" << std::endl; return false; }
        pos1 = line.find_first_of(":",pos2) + 1;
        while (line.substr(pos1,1).compare(" ")==0) { pos1++; }
        pos2 = line.find_first_of(",",pos1);
        try { ii.F = fmts.at(line.substr(pos1,pos2-pos1)); }
        catch (...) { std::cerr << "cmli read_input_header: fortran_order str not recognized or not supported for numpy" << std::endl; return false; }
        
        pos1 = line.find("shape");
        if (pos1>=line.size()) { std::cerr << "cmli read_input_header: didn't find 'shape' key in numpy HDR" << std::endl; return false; }
        pos2 = line.find_first_of(":",pos1) + 1;
        pos1 = line.find_first_of("(",pos2) + 1; pos2 = line.find_first_of(",)",pos1);
        if (pos2==pos1) { ii.R = ii.C = ii.S = 0u; }
        else
        {
            ii.R = ii.C = ii.S = ii.H = 1u;
            try { s2i = stoi(line.substr(pos1,pos2-pos1)); } catch (...) { return false; }
            if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
            ii.R = uint32_t(s2i);
            while (pos2==line.find_first_of(",",pos1))
            {
                d++; pos1 = pos2 + 1;
                pos2 = line.find_first_of(",)",pos1);
                try { s2i = stoi(line.substr(pos1,pos2-pos1)); } catch (...) { return false; }
                if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
                if (d==1) { ii.C = uint32_t(s2i); }
                else if (d==2) { ii.S = uint32_t(s2i); }
                else if (d==3) { ii.H = uint32_t(s2i); }
                else
                {
                    nd = uint32_t(s2i);
                    if (nd!=1) { std::cerr << "cmli read_input_header: only 4 dimensions supported" << std::endl; return false; }
                }
            }
        }
    }
    else
    {   
        std::cerr << "cmli read_input_header: input header format not recognized" << std::endl; return false;
    }
    
    if (!ifs) { return false; }
    return true;
}


inline bool write_output_header(std::ofstream &ofs, ioinfo &oi)
{
    using namespace std;
    
    if (!ofs) { return false; }
    
    if (oi.F==0) //write no-header (raw binary)
    {
    }
    else if (oi.F==1) //ArrayFire (.af)
    {
        const unordered_map<uint8_t,char> typs = {{1,0},{2,2},{8,4},{9,7},{10,4},{16,10},{17,11},{32,5},{33,6},{64,8},{65,9},{101,1},{102,3}};
        char version = 1, key[1] = {0}, dtype;
        int32_t narrays = 1, keylength = 1;
        int64_t offset = 0;
        int64_t D[4] = {oi.R,oi.C,oi.S,oi.H};
        try { dtype = typs.at(oi.T); } catch (...) { std::cerr << "cmli write_output_header: data type not recognized or not supported for arrayfire" << std::endl; return false; }
        try { ofs.write(&version,sizeof(char)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&narrays),sizeof(int32_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&keylength),sizeof(int32_t)); } catch (...) { return false; }
        try { ofs.write(key,keylength); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&offset),sizeof(int64_t)); } catch (...) { return false; }
        try { ofs.write(&dtype,sizeof(char)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&D[0]),4*sizeof(int64_t)); } catch (...) { return false; }
    }
    else if (oi.F==65) //Armadillo (.arma)
    {
        const unordered_map<uint32_t,string> hdrs = {{1,"FN004"},{2,"FN008"},{8,"IS001"},{9,"IU001"},{10,"IS001"},{16,"IS002"},{17,"IU002"},{32,"IS004"},{33,"IU004"},{64,"IS008"},{65,"IU008"},{101,"FC008"},{102,"FC016"}};
        if (oi.H>1u) { std::cerr << "cmli write_output_header: arma output format does not support 4D" << std::endl; return false; }
        if (oi.S>1u)
        {
            try { ofs << "ARMA_CUB_BIN_" << hdrs.at(oi.T) << std::endl << oi.R << " " << oi.C << " " << oi.S << std::endl; }
            catch (...) { return false; }
        }
        else
        {
            try { ofs << "ARMA_MAT_BIN_" << hdrs.at(oi.T) << std::endl << oi.R << " " << oi.C << std::endl; }
            catch (...) { return false; }
        }
    }
    else if (oi.F==101 || oi.F==102)  //CMLI standard header (also for Eigen)
    {
        try { ofs.write(reinterpret_cast<char*>(&oi.F),sizeof(uint8_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.T),sizeof(uint8_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.R),sizeof(uint32_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.C),sizeof(uint32_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.S),sizeof(uint32_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.H),sizeof(uint32_t)); } catch (...) { return false; }
    }
    else if (oi.F==147 || oi.F==148)  //NumPy (.npy)
    {
        const unordered_map<uint8_t,string> typs = {{0,"<f2"},{1,"<f4"},{2,"<f8"},{3,"<f16"},{8,"|i1"},{9,"|u1"},{10,"|b1"},{16,"<i2"},{16,"<u2"},{32,"<i4"},{33,"<u4"},{64,"<i8"},{65,"<u8"},{101,"<c8"},{102,"<c16"},{103,"<c32"}};
        char mstr[6] = {'\x93','N','U','M','P','Y'}, vstr[2] = {'\x01','\x00'};
        uint16_t HDR_LEN;
        string hdrline = "{'descr': '";
        string typ;
        try { typ = typs.at(oi.T); }
        catch (...) { std::cerr << "cmli write_output_header: data type not recognized or not supported for numpy" << std::endl; return false; }
        if (oi.F==147) { hdrline += typ + "', 'fortran_order': False, 'shape': ("; }
        else { hdrline += typ + "', 'fortran_order': True, 'shape': ("; }
        hdrline += to_string(oi.R) + "," + to_string(oi.C);
        if (oi.H>1u) { hdrline += "," + to_string(oi.S) +"," + to_string(oi.H); }
        else if (oi.S>1u) { hdrline += "," + to_string(oi.S); }
        hdrline += "), }";
        HDR_LEN = uint16_t(hdrline.size());
        while (hdrline.size()%64!=54) { hdrline += " "; HDR_LEN++; }
        //std::cerr << "HDR_LEN = " << HDR_LEN << std::endl;
        hdrline += "\n";
        try { ofs.write(mstr,6); } catch (...) { return false; }
        try { ofs.write(vstr,2); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&HDR_LEN),sizeof(uint16_t)); } catch (...) { return false; }
        try { ofs.write(hdrline.c_str(),std::streamsize(hdrline.size())); } catch (...) { return false; }
    }
    else
    {
        std::cerr << "cmli write_output_header: output header format not recognized. " << std::endl;
        std::cerr << "current supported formats are: 0 (raw binary), 1 (af), 65 (arma), 101 (std row-major), 102 (std col-major), 147 (npy row-major), 148 (npy col-major)" << std::endl; return false;
    }
    
    if (!ofs) { return false; }
    return true;
}

#endif

