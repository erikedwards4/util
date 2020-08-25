#ifndef CMLI_H_
#define CMLI_H_


class ioinfo
{
    public:
        size_t F = 147u, T = 1u;
        size_t R = 1u, C = 1u, S = 1u, H = 1u;
        bool only_3D() { return (F==65u); }
        bool isrowmajor() { return (F==101u || F==147u); }
        bool iscolmajor() { return (F==1u || F==65u || F==80u || F==102u || F==148u); }
        bool isreal() { return (T<100u); }
        bool iscomplex() { return (T>99u); }
        bool isbool() { return (T==10u); }
        bool isint() { return (T>3u && T<100u); }
        bool isuint() { return (T==9u || T==17u || T==33u || T==65u); }
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
        size_t nonsingleton1() { if (R>1u) { return 0u; } else if (C>1u) { return 1u; } else if (S>1u) { return 2u; } else if (H>1u) { return 3u; } else { return 0u; } }
        size_t N() { return(R*C*S*H); }
        std::streamsize sz()
        {
            if (T==8u || T==9u || T==10u) { return 1; }
            else if (T==0u || T==16u || T==17u) { return 2; }
            else if (T==1u || T==32u || T==33u || T==100u) { return 4; }
            else if (T==2u || T==64u || T==65u || T==101u) { return 8; }
            else if (T==3u || T==102u) { return 16; }
            else if (T==103u) { return 32; }
            else { return 0; }
        }
        std::streamsize nbytes()
        {
            if (T==8u || T==9u || T==10u) { return std::streamsize(R*C*S*H); }
            else if (T==0u || T==16u || T==17u) { return std::streamsize(2u*R*C*S*H); }
            else if (T==1u || T==32u || T==33u || T==100u) { return std::streamsize(4u*R*C*S*H); }
            else if (T==2u || T==64u || T==65u || T==101u) { return std::streamsize(8u*R*C*S*H); }
            else if (T==3u || T==102u) { return std::streamsize(16u*R*C*S*H); }
            else if (T==103u) { return std::streamsize(32u*R*C*S*H); }
            else { return 0; }
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
    const unordered_map<size_t,uint8_t> morder {{0u,1},{1u,1},{65u,1},{101u,0},{102u,1},{147u,0},{148u,1}};
    int pk, s2i;
    
    if (!ifs) { return false; }
    
    try { pk = ifs.peek(); } catch (...) { std::cerr << "cmli read_input_header: peek unsuccessful" << std::endl; return false; }
    if (pk==EOF) { std::cerr << "cmli read_input_header: peek of input finds EOF" << std::endl; return false; }
    //std::cerr << "pk = " << pk << std::endl;
    //try { ifs.read(reinterpret_cast<char*>(Fi),sizeof(size_t)); } catch (...) { return false; }
    //std::cerr << "Fi = " << *Fi << std::endl;

    if (pk==1)  //ArrayFire (.af)
    {
        const unordered_map<char,size_t> typs = {{0,1u},{2,2u},{7,9u},{4,8u},{10,16u},{11,17u},{5,32u},{6,33u},{8,64u},{9,65u},{1,101u},{3,102u}};
        int32_t narrays = 0, keylength;
        char key, dtype;
        int64_t offset, D[4];
        try { ifs.read(reinterpret_cast<char*>(&ii.F),sizeof(size_t)); } catch (...) { return false; }
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
        ii.R = size_t(D[0]); ii.C = size_t(D[1]); ii.S = size_t(D[2]); ii.H = size_t(D[3]);
    }
    else if (pk==65)   //Armadillo (.arma)
    {
        string line;
        size_t pos1, pos2;
        const unordered_map<string,size_t> typs = {{"FN004",1u},{"FN008",2u},{"IS001",8u},{"IU001",9u},{"IS002",16u},{"IU002",17u},{"IS004",32u},{"IU004",33u},{"IS008",64u},{"IU008",65u},{"FC008",101u},{"FC016",102u}};
        try { getline(ifs,line); } catch (...) { return false; }
        if (line.compare(0u,13u,"ARMA_CUB_BIN_")!=0 && line.compare(0u,13u,"ARMA_MAT_BIN_")!=0) { return false; }
        ii.F = size_t(pk);
        pos1 = line.find_last_of("_")+1u; pos2 = line.size();
        try { ii.T = typs.at(line.substr(pos1,pos2-pos1)); } catch (...) { return false; }
        try { getline(ifs,line); } catch (...) { return false; }
        try { s2i = stoi(line,&pos1); } catch (...) { return false; }
        if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
        ii.R = size_t(s2i);
        try { s2i = stoi(line.substr(pos1),&pos2); } catch (...) { return false; }
        if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
        ii.C = size_t(s2i);
        if (pos1+pos2>=line.size()) { ii.S = 1u; }
        else
        {
            try { s2i = stoi(line.substr(pos1+pos2)); } catch (...) { return false; }
            if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
            ii.S = size_t(s2i);
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
        try { ifs.read(reinterpret_cast<char*>(&ii.F),sizeof(size_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.T),sizeof(size_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.R),sizeof(size_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.C),sizeof(size_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.S),sizeof(size_t)); } catch (...) { return false; }
        try { ifs.read(reinterpret_cast<char*>(&ii.H),sizeof(size_t)); } catch (...) { return false; }
    }
    else if (pk==147)   //NumPy (.npy) (will set Fi to 148 if fortran_order found true)
    {
        const unordered_map<string,size_t> fmts = {{"False",147u},{"True",148u},{"false",147u},{"true",148u},{"FALSE",147u},{"TRUE",148u},{"F",147u},{"T",148u},{"f",147u},{"t",148u}};
        const unordered_map<string,size_t> typs = {{"f4",1u},{"f8",2u},{"f16",3u},{"i1",8u},{"u1",9u},{"b1",10u},{"i2",16u},{"u2",16u},{"i4",32u},{"u4",33u},{"i8",64u},{"u8",65u},{"c8",101u},{"c16",102u},{"c32",103u}};
        char mstr[6], vstr[2];
        //int version;
        uint16_t HDR_LEN = 0u;
        string line;
        size_t pos1, pos2, d = 0u, nd;

        try { ifs.read(mstr,6); } catch (...) { std::cerr << "cmli read_input_header: read of numpy magic string unsuccessful" << std::endl; return false; }
        try { ifs.read(vstr,2); } catch (...) { std::cerr << "cmli read_input_header: read of numpy version string unsuccessful" << std::endl; return false; }
        //version = int(vstr[0]);
        try { ifs.read(reinterpret_cast<char*>(&HDR_LEN),sizeof(uint16_t)); } catch (...) { std::cerr << "cmli read_input_header: read of numpy HDR_LEN unsuccessful" << std::endl; return false; }
        //std::cerr << "HDR_LEN = " << HDR_LEN << std::endl;
        try { getline(ifs,line); } catch (...) { std::cerr << "cmli read_input_header: read of numpy HDR line unsuccessful" << std::endl; return false; }
        //std::cerr << "line=" << line << std::endl;
        
        pos1 = line.find("descr");
        if (pos1>=line.size()) { std::cerr << "cmli read_input_header: didn't find 'descr' key in numpy HDR" << std::endl; return false; }
        pos2 = line.find_first_of(":",pos1) + 1u;
        pos1 = line.find_first_of("'",pos2) + 2u; pos2 = line.find_first_of("'",pos1);
        try { ii.T = typs.at(line.substr(pos1,pos2-pos1)); }
        catch (...) { std::cerr << "cmli read_input_header: dtype str not recognized or not supported for numpy" << std::endl; return false; }
        
        pos2 = line.find("fortran_order");
        if (pos2>=line.size()) { std::cerr << "cmli read_input_header: didn't find 'fortran_order' key in numpy HDR" << std::endl; return false; }
        pos1 = line.find_first_of(":",pos2) + 1u;
        while (line.substr(pos1,1u).compare(" ")==0) { pos1++; }
        pos2 = line.find_first_of(",",pos1);
        try { ii.F = fmts.at(line.substr(pos1,pos2-pos1)); }
        catch (...) { std::cerr << "cmli read_input_header: fortran_order str not recognized or not supported for numpy" << std::endl; return false; }
        
        pos1 = line.find("shape");
        if (pos1>=line.size()) { std::cerr << "cmli read_input_header: didn't find 'shape' key in numpy HDR" << std::endl; return false; }
        pos2 = line.find_first_of(":",pos1) + 1u;
        pos1 = line.find_first_of("(",pos2) + 1u; pos2 = line.find_first_of(",)",pos1);
        if (pos2==pos1) { ii.R = ii.C = ii.S = 0u; }
        else
        {
            ii.R = ii.C = ii.S = ii.H = 1u;
            try { s2i = stoi(line.substr(pos1,pos2-pos1)); } catch (...) { return false; }
            if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
            ii.R = size_t(s2i);
            while (pos2==line.find_first_of(",",pos1))
            {
                d++; pos1 = pos2 + 1u;
                pos2 = line.find_first_of(",)",pos1);
                if (line.substr(pos1,pos2-pos1).size()==0u) { s2i = 1u; }
                else
                {
                    try { s2i = stoi(line.substr(pos1,pos2-pos1)); } catch (...) { return false; }
                    if (s2i<0) { std::cerr << "stoi returned negative int" << std::endl; return false; }
                }
                if (d==1u) { ii.C = size_t(s2i); }
                else if (d==2u) { ii.S = size_t(s2i); }
                else if (d==3u) { ii.H = size_t(s2i); }
                else
                {
                    nd = size_t(s2i);
                    if (nd!=1u) { std::cerr << "cmli read_input_header: only 4 dimensions supported" << std::endl; return false; }
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
    
    if (oi.F==0u) //write no-header (raw binary)
    {
    }
    else if (oi.F==1u) //ArrayFire (.af)
    {
        const unordered_map<size_t,char> typs = {{1u,0},{2u,2},{8u,4},{9u,7},{10u,4},{16u,10},{17u,11},{32u,5},{33u,6},{64u,8},{65u,9},{101u,1},{102u,3}};
        char version = 1, key[1] = {0}, dtype;
        int32_t narrays = 1, keylength = 1;
        int64_t offset = 0;
        int64_t D[4] = {int64_t(oi.R),int64_t(oi.C),int64_t(oi.S),int64_t(oi.H)};
        try { dtype = typs.at(oi.T); } catch (...) { std::cerr << "cmli write_output_header: data type not recognized or not supported for arrayfire" << std::endl; return false; }
        try { ofs.write(&version,sizeof(char)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&narrays),sizeof(int32_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&keylength),sizeof(int32_t)); } catch (...) { return false; }
        try { ofs.write(key,keylength); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&offset),sizeof(int64_t)); } catch (...) { return false; }
        try { ofs.write(&dtype,sizeof(char)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&D[0]),4*sizeof(int64_t)); } catch (...) { return false; }
    }
    else if (oi.F==65u) //Armadillo (.arma)
    {
        const unordered_map<size_t,string> hdrs = {{1u,"FN004"},{2u,"FN008"},{8u,"IS001"},{9u,"IU001"},{10u,"IS001"},{16u,"IS002"},{17u,"IU002"},{32u,"IS004"},{33u,"IU004"},{64u,"IS008"},{65u,"IU008"},{101u,"FC008"},{102u,"FC016"}};
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
    else if (oi.F==101u || oi.F==102u)  //CMLI standard header (also for Eigen)
    {
        try { ofs.write(reinterpret_cast<char*>(&oi.F),sizeof(size_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.T),sizeof(size_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.R),sizeof(size_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.C),sizeof(size_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.S),sizeof(size_t)); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&oi.H),sizeof(size_t)); } catch (...) { return false; }
    }
    else if (oi.F==147u || oi.F==148u)  //NumPy (.npy)
    {
        const unordered_map<size_t,string> typs = {{0u,"<f2"},{1u,"<f4"},{2u,"<f8"},{3u,"<f16"},{8u,"|i1"},{9u,"|u1"},{10u,"|b1"},{16u,"<i2"},{16u,"<u2"},{32u,"<i4"},{33u,"<u4"},{64u,"<i8"},{65u,"<u8"},{101u,"<c8"},{102u,"<c16"},{103u,"<c32"}};
        char mstr[6] = {'\x93','N','U','M','P','Y'}, vstr[2] = {'\x01','\x00'};
        uint16_t HDR_LEN;
        string hdrline = "{'descr': '";
        string typ;
        try { typ = typs.at(oi.T); }
        catch (...) { std::cerr << "cmli write_output_header: data type not recognized or not supported for numpy" << std::endl; return false; }
        if (oi.F==147u) { hdrline += typ + "', 'fortran_order': False, 'shape': ("; }
        else { hdrline += typ + "', 'fortran_order': True, 'shape': ("; }
        hdrline += to_string(oi.R) + ", " + to_string(oi.C);
        if (oi.H>1u) { hdrline += ", " + to_string(oi.S) + ", " + to_string(oi.H); }
        else if (oi.S>1u) { hdrline += ", " + to_string(oi.S); }
        hdrline += "), }";
        HDR_LEN = uint16_t(hdrline.size());
        while (hdrline.size()%64u!=53u) { hdrline += " "; HDR_LEN++; }
        hdrline += "\n"; HDR_LEN++;
        //std::cerr << "HDR_LEN = " << HDR_LEN << std::endl;
        try { ofs.write(mstr,6); } catch (...) { return false; }
        try { ofs.write(vstr,2); } catch (...) { return false; }
        try { ofs.write(reinterpret_cast<char*>(&HDR_LEN),sizeof(uint16_t)); } catch (...) { return false; }
        try { ofs.write(hdrline.c_str(),std::streamsize(hdrline.size())); } catch (...) { return false; }
    }
    else
    {
        std::cerr << "cmli write_output_header: output header format not recognized. " << std::endl;
        std::cerr << "current supported formats are: 0 (raw binary), 1 (afire), 65 (arma), 101 (gen row-major), 102 (gen col-major), 147 (npy row-major), 148 (npy col-major)" << std::endl; return false;
    }
    
    if (!ofs) { return false; }
    return true;
}

#endif
