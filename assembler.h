#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "data_type.h"
#include "qtcpu.h"

#include <string>
#include <vector>
#include <cstdlib>

class assembler {
public:
    assembler();
    //assembler(std::string);
    ~assembler();
    
    enum
    {
        LABEL_NUM = 1000, LABEL_LEN = 256,
        COMMD_NUM = 2000,
        PARA_NUM = 50, PARA_LEN = 200
    };
    
    void init();
    
    int load(std::string filename);
    int assemble();
    int assemble_kernel();
    
    int save(dword* commd_set);
    int save(std::string filename);
    
    int get_commd_num();
        
    byte get_static_mem(dword addr);
    int save_static_mem(byte *mem);
    dword get_static_mem_size();

    void print();
    
    void set_CPU_mem(
        dword _kernel = qtCPU::KERNEL_MEM,
        dword _user = qtCPU::USER_MEM,
        dword _static = qtCPU::STATIC_MEM,
        dword _main = qtCPU::MAIN_MEM,
        dword _disp = qtCPU::DISP_MEM
    );
    
private:
    dword CPU_KERNEL_MEM, CPU_USER_MEM, CPU_STATIC_MEM,
          CPU_MAIN_MEM, CPU_DISP_MEM;
    
    typedef struct {
        char na[LABEL_LEN];
        char type;
        dword line;
    } TABLE;

    std::vector<std::string> instru_set;
    
    char instru[PARA_NUM][PARA_LEN];
    TABLE label_out[LABEL_NUM], label_in[LABEL_NUM];
    int lo, li;
    dword list[COMMD_NUM];
    dword list_n;
    
    byte static_mem[qtCPU::MAIN_MEM - qtCPU::STATIC_MEM];
    dword static_mem_ptr;
    
    int strcut(const char buf[], int mode);
    dword regX(char s[]);
    void recheck(int mode);
    dword immed_addr(char s[]);
    dword gen_core(char s[][PARA_LEN]);
    int gen_instru(char s[][PARA_LEN]);
    int gen_data(char s[][PARA_LEN]);
    dword immed(char s[], dword mask, int div);
    dword atom(char *s);
    int is_num(const char *s);
    
    // tools
    dword string_cpy(byte *dest, const char *src);
    std::string trim(const std::string &str);
};

#endif // ASSEMBLER_H
