#ifndef DECOMPILER_H
#define DECOMPILER_H

#include "data_type.h"

#include <vector>
#include <string>

class decompiler {
public:
    decompiler();
    ~decompiler();
    
    int load(std::string filename, int mode = 0);
    int load(const dword* mem, dword size, int mode = 0);
    
    std::string get_instru(int order);
    int get_instru_num();
    
    void print();
    int regName(char s[], dword r);
    
private:
    int trans(char s[], dword ir, dword pc);
    std::vector<std::string> instru_set;
};

#endif // DECOMPILER_H
