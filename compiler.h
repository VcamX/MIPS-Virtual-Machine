#ifndef COMPILER_H
#define COMPILER_H

#include "data_type.h"

#include <string>
#include <vector>
#include <cstdlib>

class compiler {
public:
    compiler();
    compiler(std::string, std::string);
    compiler(std::string, dword* commd_set);
    ~compiler();
    
    enum { LABEL_NUM = 100, LABEL_LEN = 50, COMMD_NUM = 100, PARA_NUM = 10, ARG_LEN = 100 };
    
    int load(std::string filename);
    int compile();
    
    int save(dword* commd_set);
    int get_commd_num();
    int save(std::string filename);
    
    //std::string get_instru(int order);
    
    void print();

private:
    typedef struct {
        char na[LABEL_LEN];
        char type;
        dword line;
    } TABLE;

    void init();
    std::vector<std::string> instru_set;
    
    char instru[PARA_NUM][ARG_LEN];
    TABLE label_out[LABEL_NUM], label_in[LABEL_NUM];
    int lo, li;
    dword list[COMMD_NUM];
    int list_n;
    
    int strcut(const char buf[]);
    dword regX(char s[]);
    void recheck();
    dword immed_addr(char s[]);
    dword gen_core(char s[][ARG_LEN]);
    int gen_instru(char s[][ARG_LEN]);
    dword immed(char s[], dword mask, int div);
    dword atom(char *s);
    int is_num(const char *s);
};

#endif // COMPILER_H
