#ifndef MIPSCPU_H
#define MIPSCPU_H
#include "data_type.h"

#include <string>

class CPU{
    public:
        CPU();
        ~CPU();
        
        int boot(std::string filename);
        int boot(const dword *commd_set, int size);
        int load_static_data(const byte *static_mem, dword size);
        int load_mem_data(const dword *mem, dword size, const dword adr_st, const dword adr_ed);

        int execute();
        int execute_single();
        int getCurrSize();
        dword getIR(const dword);
        dword getPC();
        const dword* getReg();
        const byte* getMem(dword addr);
        const byte* getDispMem();

        void show_reg();
        void rst(int mode = 0);
        void setloaded();
        bool isloaded();
        bool is_mem_modified();
        dword get_mem_modified_addr();

        dword getIC();

        void setEP(dword endp);
        dword getEP();

        enum { REGNUM = 34, REG_HI = 32, REG_LO = 33,
               WIDTH = 80, HEIGHT = 25,
               END_MEM = 0x10000, // 64Kb
               KERNEL_MEM = 0, USER_MEM = 0x2000, STATIC_MEM = 0x5000,
               MAIN_MEM = 0x7000, DISP_MEM = END_MEM - WIDTH*HEIGHT };
        
    private:
        dword PC, IR, MDR, endpoint;
        dword Reg[REGNUM];
        int CurrSize;
        byte Memory[END_MEM];

        bool loadedflag;
        bool mem_modified_flag;
        dword mem_modified_addr;
};

#endif // MIPSCPU_H
