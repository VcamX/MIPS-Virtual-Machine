#ifndef DEASSEMBLER_H
#define DEASSEMBLER_H

#include "data_type.h"

#include <vector>
#include <string>

class deassembler {
public:
    deassembler();
    ~deassembler();
    
    int load(std::string filename, dword init_pc, int mode = 0);
    int load(const dword* mem, dword size, dword init_pc, int mode = 0);
    
    std::string get_instru(int order);
    int get_instru_num();
    
    void print();
    int regName(char s[], dword r);
    
private:
    int trans(char s[], dword ir, dword pc);
    std::vector<std::string> instru_set;
};

#endif // DEASSEMBLER_H
