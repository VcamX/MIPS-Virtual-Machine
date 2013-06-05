#include "qtcpu.h"
#include <iostream>
#include <cstdio>
//#include <cstdlib>
//#include <string>
#include <cstring>

qtCPU::qtCPU(QObject *parent) :
    QObject(parent)
{
    memset(memory, 0, sizeof(memory));
    memset(reg, 0, sizeof(reg));
    memset(t_reg, 0, sizeof(t_reg));

    clr_mem(memory, END_MEM);

    //currsize = 0;
    rst();
}

void qtCPU::clr_mem(byte mem[], dword size)
{
    for (dword i = 0; i < size; i++)
        mem[i] = 0;
}

void qtCPU::rst()
{
    memset(reg, 0, sizeof(reg));
    reg[29] = DISP_MEM;
    reg[30] = reg[29];
    reg[28] = STATIC_MEM;
    reg[REG_PC] = USER_MEM;

    about_to_reg_update();
}

/*
int qtCPU::boot(const dword* commd_set, int size) {
    if (size*4 >= STATIC_MEM - USER_MEM) {
        std::cout << "Memory can't contain the data!\n";
        return 1;
    }

    dword currsize = 0;
    for (int i = 0; i < size; i++) {
        memory[USER_MEM+ currsize++] = (byte)((commd_set[i] >> 24) & 0xFF);
        memory[USER_MEM+ currsize++] = (byte)((commd_set[i] >> 16) & 0xFF);
        memory[USER_MEM+ currsize++] = (byte)((commd_set[i] >> 8) & 0xFF);
        memory[USER_MEM+ currsize++] = (byte)(commd_set[i] & 0xFF);
    }

    //setEP(USER_MEM+CurrSize);

    return 0;
}
*/

int qtCPU::load_static_data(const byte *static_mem, dword size)
{
    if (size > MAIN_MEM - STATIC_MEM) return 1;
    for (dword i = 0; i < size; i++) {
        memory[STATIC_MEM + i] = static_mem[i];
    }
    return 0;
}

int qtCPU::load_mem_commd_data(const dword *commd_set, dword size, const dword adr_st, const dword adr_ed)
{
    if (size*4 > adr_ed - adr_st) return 1;

    dword currsize = 0;
    for (dword i = 0; i < size; i++) {
        memory[adr_st + currsize++] = (byte)((commd_set[i] >> 24) & 0xFF);
        memory[adr_st + currsize++] = (byte)((commd_set[i] >> 16) & 0xFF);
        memory[adr_st + currsize++] = (byte)((commd_set[i] >> 8) & 0xFF);
        memory[adr_st + currsize++] = (byte)(commd_set[i] & 0xFF);
    }
    return 0;
}

void qtCPU::cpy_reg()
{
    for (dword i = 0; i < REGNUM; i++) t_reg[i] = reg[i];
}

void qtCPU::about_to_reg_update()
{
    for (dword i = 0; i < REGNUM; i++)
    {
        if (reg[i] != t_reg[i])
            emit reg_update(i, reg[i]);
    }
    cpy_reg();
}

void qtCPU::pc_increment(int action)
{
    if (action == 0)
    {
        if (reg[REG_PC] < STATIC_MEM)
        {
            execute();

            about_to_reg_update();
            emit exec_result_send(true);
        }
        else
        {
            emit exec_result_send(false);
        }
    }
    else if (action == 1)
    {
    }
}

dword qtCPU::get_ins(const dword pc) {
    dword ir;
    ir = (memory[pc+0] << 24) |
         (memory[pc+1] << 16) |
         (memory[pc+2] << 8) |
         (memory[pc+3]);
    return ir;
}
/*
dword qtCPU::get_ins_counter() {
    return (reg[REG_PC] - USER_MEM) / 4;
}
*/

void qtCPU::set_keyboard_irq(const byte val)
{
    have_irq = true;
    memory[2] = val;
}

int qtCPU::execute() {
    int op, rs, rt, rd, dat, udat, addr, shmt, fun;
    unsigned long long t;
    dword rpc = 0, t_ra = 0, ir;

    if (have_irq && !is_in_irq)
    {
        have_irq = false;
        is_in_irq = true;
        epc = reg[REG_PC];
    }
    else
    {
        ir = get_ins(reg[REG_PC]);

        reg[REG_PC] += 4;
        //printf(" %08X: %08X\n", getIC(), IR);

        op = (ir >> 26) & 0x3F;
        rs = (ir >> 21) & 0x1F;
        rt = (ir >> 16) & 0x1F;
        rd = (ir >> 11) & 0x1F;
        shmt = (ir >> 6) & 0x1F;
        fun = ir & 0x3F;
        dat = (short)(ir & 0xFFFF);
        udat = ir & 0xFFFF;
        addr = (ir & 0x3FFFFFF) << 2;

        switch (op) {
            case 0:
                switch (fun) {
                    case 12:    //syscall
                        t_ra = reg[31];
                        reg[31] = reg[REG_PC];

                        reg[REG_PC] = KERNEL_MEM;
                        break;

                    case 16:    //mfhi
                        reg[rd] = reg[REG_HI];
                        break;

                    case 18:    //mflo
                        reg[rd] = reg[REG_LO];
                        break;

                    case 24:    //mult
                        t = (signed)reg[rs] * (signed)reg[rt];
                        reg[REG_HI] = t >> 32;
                        reg[REG_LO] = t & 0xFFFFFFFFFFFFFFFF;
                        break;

                    case 25:    //multu
                        t = reg[rs] * reg[rt];
                        reg[REG_HI] = t >> 32;
                        reg[REG_LO] = t & 0xFFFFFFFFFFFFFFFF;
                        break;

                    case 26:    //div
                        reg[REG_HI] = (signed)reg[rs] % (signed)reg[rt];
                        reg[REG_LO] = (signed)reg[rs] / (signed)reg[rt];
                        break;

                    case 27:    //divu
                        reg[REG_HI] = reg[rs] % reg[rt];
                        reg[REG_LO] = reg[rs] / reg[rt];
                        break;

                    case 32:    //add
                        reg[rd] = (signed)reg[rs] + (signed)reg[rt];
                        break;

                    case 33:    //addu
                        reg[rd] = reg[rs] + reg[rt];
                        break;

                    case 34:    //sub
                        reg[rd] = (signed)reg[rs] - (signed)reg[rt];
                        break;

                    case 35:    //subu
                        reg[rd] = reg[rs] - reg[rt];
                        break;

                    case 36:    //and
                        reg[rd] = reg[rs] & reg[rt];
                        break;

                    case 37:    //or
                        reg[rd] = reg[rs] | reg[rt];
                        break;

                    case 38:    //xor
                        reg[rd] = reg[rs] ^ reg[rt];
                        break;

                    case 39:    //nor
                        reg[rd] = ~(reg[rs] | reg[rt]);
                        break;

                    case 42:    //slt
                        reg[rd] = (signed)reg[rs] < (signed)reg[rt] ? 1 : 0;
                        break;

                    case 43:    //sltu
                        reg[rd] = reg[rs] < reg[rt] ? 1 : 0;
                        break;

                    case 0:    //sll
                        reg[rd] = reg[rt] << shmt;
                        break;

                    case 2:    //srl
                        reg[rd] = reg[rt] >> shmt;
                        break;

                    case 3:    //sra
                        reg[rd] = (signed)reg[rt] >> shmt;
                        //std::cout << shmt;
                        break;

                    case 4:    //sllv
                        reg[rd] = reg[rt] << reg[rs];
                        break;

                    case 6:    //srlv
                        reg[rd] = reg[rt] >> reg[rs];
                        break;

                    case 8:    //jr
                        reg[REG_PC] = reg[rs];

                        if (rs == 31 && reg[REG_PC] == rpc)
                        {
                            //syscall_flag = false;
                            reg[31] = t_ra;
                        }
                        break;
                }
                break;

            case 1:
                switch (rt) {
                    case 0:     //bltz
                        if (rs < 0)
                            reg[REG_PC] += (dat << 2);
                        break;

                    case 1:     //bgez
                        if (rs >= 0)
                            reg[REG_PC] += (dat << 2);
                        break;

                    case 16:    //bltzal
                        if (rs < 0) {
                            //Reg[31] = PC + 4; //enable delay slot
                            reg[31] = reg[REG_PC]; //disable delay slot
                            reg[REG_PC] += (dat << 2);
                        }
                        break;

                    case 17:    //bgezal
                        if (rs >= 0) {
                            //Reg[31] = PC + 4; //enable delay slot
                            reg[31] = reg[REG_PC]; //disable delay slot
                            reg[REG_PC] += (dat << 2);
                        }
                        break;
                }
                break;

            case 4:     //beq
                if (reg[rs] == reg[rt])
                    reg[REG_PC] += (dat << 2);
                break;

            case 5:     //bne
                if (reg[rs] != reg[rt])
                    reg[REG_PC] += (dat << 2);
                break;

            case 6:     //blez
                if (reg[rs] <= 0)
                    reg[REG_PC] += (dat << 2);
                break;

            case 7:     //bgtz
                if (reg[rs] > 0)
                    reg[REG_PC] += (dat << 2);
                break;

            case 8:     //addi
                reg[rt] = (signed)reg[rs] + dat;
                break;

            case 9:     //addiu
                reg[rt] = reg[rs] + dat;
                //cout << sizeof(short) << endl << dat << endl;
                break;

            case 10:     //slti
                reg[rt] = (signed)reg[rs] < dat ? 1 : 0;
                //cout << sizeof(short) << endl << dat << endl;
                break;

            case 11:     //sltiu
                reg[rt] = reg[rs] < (unsigned)dat ? 1 : 0;
                //cout << sizeof(short) << endl << dat << endl;
                break;

            case 12:     //andi
                reg[rt] = reg[rs] & udat;
                break;

            case 13:     //ori
                reg[rt] = reg[rs] | udat;
                break;

            case 14:     //xori
                reg[rt] = reg[rs] ^ udat;
                break;

            case 15:     //lui
                reg[rt] = dat << 16;
                break;

            case 32:    //lb
                reg[rt] = (signed)memory[reg[rs]+dat+0];
                break;

            case 33:    //lh
                reg[rt] = (signed)(((memory[reg[rs]+dat+0]) << 8) |
                          memory[reg[rs]+dat+1]);
                break;

            case 35:    //lw
                /*
                printf("%X %X %X %X\n",
                       Memory[Reg[rs]+dat+0], Memory[Reg[rs]+dat+1], Memory[Reg[rs]+dat+2], Memory[Reg[rs]+dat+3]);
                */
                reg[rt] = (memory[reg[rs]+dat+0] << 24);
                reg[rt] |= (memory[reg[rs]+dat+1] << 16);
                reg[rt] |= (memory[reg[rs]+dat+2] << 8);
                reg[rt] |= (memory[reg[rs]+dat+3]);
                break;

            case 36:    //lbu
                reg[rt] = memory[reg[rs]+dat+0];
                break;

            case 37:    //lhu
                reg[rt] = ((memory[reg[rs]+dat+0]) << 8) |
                          memory[reg[rs]+dat+1];
                break;

            case 40:    //sb
                memory[reg[rs]+dat+0] = (byte)(reg[rt] & 0xFF);
                /*
                mem_modified_flag = true;
                mem_modified_addr = reg[rs] + dat;
                */
                send_mem_update(reg[rs] + dat);
                break;

            case 41:    //sh
                memory[reg[rs]+dat+0] = (byte)((reg[rt] >> 8) & 0xFF);
                memory[reg[rs]+dat+1] = (byte)(reg[rt] & 0xFF);
                /*
                mem_modified_flag = true;
                mem_modified_addr = reg[rs] + dat;
                */
                send_mem_update(reg[rs] + dat);
                break;

            case 43:    //sw
                memory[reg[rs]+dat+0] = (byte)((reg[rt] >> 24) & 0xFF);
                memory[reg[rs]+dat+1] = (byte)((reg[rt] >> 16) & 0xFF);
                memory[reg[rs]+dat+2] = (byte)((reg[rt] >> 8) & 0xFF);
                memory[reg[rs]+dat+3] = (byte)(reg[rt] & 0xFF);
                /*
                mem_modified_flag = true;
                mem_modified_addr = reg[rs] + dat;
                */
                send_mem_update(reg[rs] + dat);
                break;

            case 2:     //j
                reg[REG_PC] = addr;
                break;

            case 3:     //jal
                //Reg[31] = PC + 4; //enable delay slot
                reg[31] = reg[REG_PC]; //disable delay slot
                reg[REG_PC] = addr;
                break;

            default:
                printf("Instrution Error!\n");
                return 1;
        }
    }
    //while (syscall_flag);

    return 0;
}

void qtCPU::send_mem_update(const dword addr)
{
    emit mem_update(addr, get_ins(addr));
}

const byte* qtCPU::get_mem_ptr(const dword addr_st) {
    return (addr_st < END_MEM) ? memory+addr_st : NULL;
}

const dword* qtCPU::get_reg_ptr()
{
    return reg;
}

/*
bool qtCPU::is_mem_modified() {
    return mem_modified_flag;
}
*/
