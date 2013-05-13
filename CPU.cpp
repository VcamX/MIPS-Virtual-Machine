#include "CPU.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace std;

//MIPSCPU::MIPSCPU() :REGNUM(32), MAXMEM(100000) {
CPU::CPU() {
    rst();
    memset(Memory, 0, sizeof(Memory));
}

CPU::~CPU() {
}

int CPU::boot(string filename) {
    FILE *file;
    file = fopen(filename.c_str(), "rb");
    if (!file) {
        cout << "File Error!\n";
        fclose(file);
        return 1;
    }
    else {
        cout << "\nFile " << filename << " is loading...\n";
    }
    while (fread(&(Memory[USER_MEM+CurrSize]), sizeof(byte), 1, file)) {
        //printf("%02X", CurrSize, Memory[CurrSize]);
        CurrSize++;
        /*
        if (!(CurrSize % 4)) {
            code = (Memory[CurrSize-4] << 24) | (Memory[CurrSize-3] << 16) |
                   (Memory[CurrSize-2] << 8) | Memory[CurrSize-1];
            printf("%08X\n", code);
        }
        */
        if (USER_MEM+CurrSize >= MAIN_MEM) {
            cout << "Out of Memory!\n";
            fclose(file);
            return 1;
        }
    }
    cout << "Loading is done!\n";
    fclose(file);

    setEP(USER_MEM+CurrSize);

    return 0;
}

int CPU::boot(const dword* commd_set, int size) {
    for (int i = 0; i < size; i++) {
        if (USER_MEM+CurrSize >= MAIN_MEM) {
            cout << "Out of Memory!\n";
            return 1;
        }
        Memory[USER_MEM+CurrSize++] = (byte)((commd_set[i] >> 24) & 0xFF);
        Memory[USER_MEM+CurrSize++] = (byte)((commd_set[i] >> 16) & 0xFF);
        Memory[USER_MEM+CurrSize++] = (byte)((commd_set[i] >> 8) & 0xFF);
        Memory[USER_MEM+CurrSize++] = (byte)(commd_set[i] & 0xFF);
    }

    setEP(USER_MEM+CurrSize);

    return 0;
}

dword CPU::getIR(const dword PC) {
    dword ir;
    ir = (Memory[PC+0] << 24) |
         (Memory[PC+1] << 16) |
         (Memory[PC+2] << 8) |
         (Memory[PC+3]);
    return ir;
}

dword CPU::getPC() {
    return PC;
}

dword CPU::getIC() {
    return (PC - USER_MEM) / 4;
}

void CPU::setEP(dword endp) {
    printf("End point: %08X\n", endp);
    if (USER_MEM <= endp && endp <= MAIN_MEM)
        endpoint = endp;
    endpoint = (endpoint - USER_MEM) / 4 * 4 + USER_MEM;
}

dword CPU::getEP() {
    return endpoint;
}

dword CPU::get_mem_modified_addr() {
    return mem_modified_addr;
}

int CPU::execute_single() {
    int op, rs, rt, rd, dat, udat, adr, shmt, fun;
    unsigned long long t;
    IR = getIR(PC);

    printf(" %03d: %08X\n", getIC(), IR);

    if (PC == endpoint) {
        return 2;
    }

    mem_modified_flag = false;
    
    PC += 4;
    op = (IR >> 26) & 0x3F;
    rs = (IR >> 21) & 0x1F;
    rt = (IR >> 16) & 0x1F;
    rd = (IR >> 11) & 0x1F;
    shmt = (IR >> 6) & 0x1F;
    fun = IR & 0x3F;
    dat = (short)(IR & 0xFFFF);
    udat = IR & 0xFFFF;
    adr = (IR & 0x3FFFFFF) << 2;
    
    switch (op) {
        case 0:
            switch (fun) {
                case 16:    //mfhi
                    Reg[rd] = Reg[REG_HI];
                    break;

                case 18:    //mflo
                    Reg[rd] = Reg[REG_LO];
                    break;

                case 24:    //mult
                    t = (signed)Reg[rs] * (signed)Reg[rt];
                    Reg[REG_HI] = t >> 32;
                    Reg[REG_LO] = t & 0xFFFFFFFFFFFFFFFF;
                    break;

                case 25:    //multu
                    t = Reg[rs] * Reg[rt];
                    Reg[REG_HI] = t >> 32;
                    Reg[REG_LO] = t & 0xFFFFFFFFFFFFFFFF;
                    break;

                case 26:    //div
                    Reg[REG_HI] = (signed)Reg[rs] % (signed)Reg[rt];
                    Reg[REG_LO] = (signed)Reg[rs] / (signed)Reg[rt];
                    break;

                case 27:    //divu
                    Reg[REG_HI] = Reg[rs] % Reg[rt];
                    Reg[REG_LO] = Reg[rs] / Reg[rt];
                    break;

                case 32:    //add
                    Reg[rd] = (signed)Reg[rs] + (signed)Reg[rt];
                    break;

                case 33:    //addu
                    Reg[rd] = Reg[rs] + Reg[rt];
                    break;

                case 34:    //sub
                    Reg[rd] = (signed)Reg[rs] - (signed)Reg[rt];
                    break;

                case 35:    //subu
                    Reg[rd] = Reg[rs] - Reg[rt];
                    break;

                case 36:    //and
                    Reg[rd] = Reg[rs] & Reg[rt];
                    break;

                case 37:    //or
                    Reg[rd] = Reg[rs] | Reg[rt];
                    break;

                case 38:    //xor
                    Reg[rd] = Reg[rs] ^ Reg[rt];
                    break;

                case 39:    //nor
                    Reg[rd] = ~(Reg[rs] & Reg[rt]);
                    break;

                case 42:    //slt
                    Reg[rd] = (signed)Reg[rs] < (signed)Reg[rt] ? 1 : 0;
                    break;

                case 43:    //sltu
                    Reg[rd] = Reg[rs] < Reg[rt] ? 1 : 0;
                    break;

                case 0:    //sll
                    Reg[rd] = Reg[rt] << shmt;
                    break;

                case 2:    //srl
                    Reg[rd] = Reg[rt] >> shmt;
                    break;

                case 3:    //sra
                    Reg[rd] = (signed)Reg[rt] >> shmt;
                    cout << shmt;
                    break;

                case 4:    //sllv
                    Reg[rd] = Reg[rt] << Reg[rs];
                    break;

                case 6:    //srlv
                    Reg[rd] = Reg[rt] >> Reg[rs];
                    break;

                case 8:    //jr
                    PC = Reg[rs];
                    break;
            }
            break;

        case 1:
            switch (rt) {
                case 0:     //bltz
                    if (rs < 0)
                        PC += (dat << 2);
                    break;

                case 1:     //bgez
                    if (rs >= 0)
                        PC += (dat << 2);
                    break;

                case 16:    //bltzal
                    if (rs < 0) {
                        Reg[31] = PC + 4;
                        PC += (dat << 2);
                    }
                    break;

                case 17:    //bgezal
                    if (rs >= 0) {
                        Reg[31] = PC + 4;
                        PC += (dat << 2);
                    }
                    break;
            }
            break;

        case 4:     //beq
            if (Reg[rs] == Reg[rt])
                PC += (dat << 2);
            break;

        case 5:     //bne
            if (Reg[rs] != Reg[rt])
                PC += (dat << 2);
            break;

        case 6:     //blez
            if (Reg[rs] <= 0)
                PC += (dat << 2);
            break;

        case 7:     //bgtz
            if (Reg[rs] > 0)
                PC += (dat << 2);
            break;

        case 8:     //addi
            Reg[rt] = (signed)Reg[rs] + dat;
            break;

        case 9:     //addiu
            Reg[rt] = Reg[rs] + dat;
            //cout << sizeof(short) << endl << dat << endl;
            break;

        case 10:     //slti
            Reg[rt] = (signed)Reg[rs] < dat ? 1 : 0;
            //cout << sizeof(short) << endl << dat << endl;
            break;
            
        case 11:     //sltiu
            Reg[rt] = Reg[rs] < (unsigned)dat ? 1 : 0;
            //cout << sizeof(short) << endl << dat << endl;
            break;
            
        case 12:     //andi
            Reg[rt] = Reg[rs] & udat;
            break;

        case 13:     //ori
            Reg[rt] = Reg[rs] | udat;
            break;

        case 14:     //xori
            Reg[rt] = Reg[rs] ^ udat;
            break;

        case 15:     //lui
            Reg[rt] = dat << 16;
            break;

        case 32:    //lb
            Reg[rt] = (signed)Memory[Reg[rs]+dat+0];
            break;

        case 33:    //lh
            Reg[rt] = (signed)(((Memory[Reg[rs]+dat+0]) << 8) |
                      Memory[Reg[rs]+dat+1]);
            break;

        case 35:    //lw
            /*
            printf("%X %X %X %X\n",
                   Memory[Reg[rs]+dat+0], Memory[Reg[rs]+dat+1], Memory[Reg[rs]+dat+2], Memory[Reg[rs]+dat+3]);
            */
            Reg[rt] = (Memory[Reg[rs]+dat+0] << 24);
            Reg[rt] |= (Memory[Reg[rs]+dat+1] << 16);
            Reg[rt] |= (Memory[Reg[rs]+dat+2] << 8);
            Reg[rt] |= (Memory[Reg[rs]+dat+3]);
            break;

        case 36:    //lbu
            Reg[rt] = Memory[Reg[rs]+dat+0];
            break;

        case 37:    //lhu
            Reg[rt] = ((Memory[Reg[rs]+dat+0]) << 8) |
                      Memory[Reg[rs]+dat+1];
            break;

        case 40:    //sb
            Memory[Reg[rs]+dat+0] = (byte)(Reg[rt] & 0xFF);
            mem_modified_flag = true;
            mem_modified_addr = Reg[rs] + dat;
            break;

        case 41:    //sh
            Memory[Reg[rs]+dat+0] = (byte)((Reg[rt] >> 8) & 0xFF);
            Memory[Reg[rs]+dat+1] = (byte)(Reg[rt] & 0xFF);
            mem_modified_flag = true;
            mem_modified_addr = Reg[rs] + dat;
            break;

        case 43:    //sw
            Memory[Reg[rs]+dat+0] = (byte)((Reg[rt] >> 24) & 0xFF);
            Memory[Reg[rs]+dat+1] = (byte)((Reg[rt] >> 16) & 0xFF);
            Memory[Reg[rs]+dat+2] = (byte)((Reg[rt] >> 8) & 0xFF);
            Memory[Reg[rs]+dat+3] = (byte)(Reg[rt] & 0xFF);
            mem_modified_flag = true;
            mem_modified_addr = Reg[rs] + dat;
            break;

        case 2:     //j
            PC = adr;
            break;

        case 3:     //jal
            Reg[31] = PC + 4;
            //Reg[31] = PC;
            PC = adr;
            break;
        
        default:
            printf("Instrution Error!\n");
            return 1;
            break;            
    }
    return 0;
}

int CPU::execute() {
    //int op, rs, rt, rd, dat, adr, shmt, fun;
    //int cnt = 0;
    
    cout << "\nStart executing...\n";
    for (;;) {
        switch (execute_single()) {
            case 1:
                return 1;
            case 2:
                break;
        }
        /*
        //getchar();
        IR = (Memory[PC+0] << 24) |
             (Memory[PC+1] << 16) |
             (Memory[PC+2] << 8) |
             (Memory[PC+3]);
        
        if (!IR) {
            //show_reg();
            break;
        }
        else
            printf("  %d: %08X\n", ++cnt, IR);
        
        PC += 4;
        op = (IR >> 26) & 0x3F;
        rs = (IR >> 21) & 0x1F;
        rt = (IR >> 16) & 0x1F;
        rd = (IR >> 11) & 0x1F;
        shmt = (IR >> 6) & 0x1F;
        fun = IR & 0x3F;
        dat = (short)(IR & 0xFF);
        adr = (IR & 0x3FFFFFFF) << 2;
        
        switch (op) {
            case 0:
                switch (fun) {
                    case 32:    //add
                        Reg[rd] = (signed)Reg[rs] + (signed)Reg[rt];
                        break;
                    case 33:    //addu
                        Reg[rd] = Reg[rs] + Reg[rt];
                        break;
                    case 34:    //sub
                        Reg[rd] = (signed)Reg[rs] - (signed)Reg[rt];
                        break;
                    case 35:    //subu
                        Reg[rd] = Reg[rs] - Reg[rt];
                        break;
                    case 36:    //and
                        Reg[rd] = Reg[rs] & Reg[rt];
                        break;
                    case 37:    //or
                        Reg[rd] = Reg[rs] | Reg[rt];
                        break;
                    case 38:    //xor
                        Reg[rd] = Reg[rs] ^ Reg[rt];
                        break;
                    case 39:    //nor
                        Reg[rd] = ~(Reg[rs] & Reg[rt]);
                        break;
                    case 42:    //slt
                        Reg[rd] = (signed)Reg[rs] < (signed)Reg[rt] ? 1 : 0;
                        break;
                    case 43:    //sltu
                        Reg[rd] = Reg[rs] < Reg[rt] ? 1 : 0;
                        break;
                    case 0:    //sll
                        Reg[rd] = Reg[rs] << Reg[rt];
                        break;
                    case 2:    //srl
                        Reg[rd] = Reg[rs] >> Reg[rt];
                        break;
                    case 3:    //sra
                        Reg[rd] = (signed)Reg[rs] >> Reg[rt];
                        break;
                    case 8:    //jr
                        PC = Reg[rs];
                        break;
                }
                break;
            case 4:     //beq
                if (Reg[rs] == Reg[rt])
                    PC += (dat << 2);
                break;
            case 5:     //bne
                if (Reg[rs] != Reg[rt])
                    PC += (dat << 2);
                break;
            case 8:     //addi
                Reg[rt] = (signed)Reg[rs] + dat;
                break;
            case 12:     //andi
                Reg[rt] = Reg[rs] & dat;
                break;
            case 13:     //ori
                Reg[rt] = Reg[rs] | dat;
                break;
            case 14:     //xori
                Reg[rt] = Reg[rs] ^ dat;
                break;
            case 32:    //lb
                Reg[rt] = Memory[Reg[rs]+dat+0];
                break;
            case 33:    //lh
                Reg[rt] = ((Memory[Reg[rs]+dat+0]) << 8) |
                          Memory[Reg[rs]+dat+1];
                break;
            case 35:    //lw
                Reg[rt] = ((Memory[Reg[rs]+dat+0]) << 24) |
                          ((Memory[Reg[rs]+dat+1]) << 16) |
                          ((Memory[Reg[rs]+dat+2]) << 8) |
                          (Memory[Reg[rs]+dat+3]);
                break;
            case 40:    //sb
                Memory[Reg[rs]+dat+0] = (byte)(Reg[rt] & 0xFF);
                break;
            case 41:    //sh
                Memory[Reg[rs]+dat+0] = (byte)((Reg[rt] >> 8) & 0xFF);
                Memory[Reg[rs]+dat+1] = (byte)(Reg[rt] & 0xFF);
                break;
            case 43:    //sw
                Memory[Reg[rs]+dat+0] = (byte)((Reg[rt] >> 24) & 0xFF);
                Memory[Reg[rs]+dat+1] = (byte)((Reg[rt] >> 16) & 0xFF);
                Memory[Reg[rs]+dat+2] = (byte)((Reg[rt] >> 8) & 0xFF);
                Memory[Reg[rs]+dat+3] = (byte)(Reg[rt] & 0xFF);
                break;
            case 2:     //j
                PC = PC & 0xFFFFFFFF | adr;
                break;
            case 3:     //jal
                Reg[31] = PC + 4;
                PC = adr;
                break;
            
            default:
                printf("Instrution Error!\n");
                return 1;
        }*/
    }
    cout << "End of executing...\n";
    return 0;
}

void CPU::rst(int mode) {
    memset(Reg, 0, sizeof(Reg));
    Reg[29] = DISP_MEM - 1;

    /* clear screen */
    for (int i = END_MEM - WIDTH*HEIGHT; i < END_MEM; i++) {
        Memory[i] = 0;
    }

    if (mode) {
        for (dword i = USER_MEM; i < DISP_MEM; i++) Memory[i] = 0;
    }

    CurrSize = 0;
    PC = USER_MEM;
    IR = 0;
    loadedflag = false;
}

void CPU::show_reg() {
    cout << "\n--- Reg Content ---\n";
    for (int i = 0; i < REGNUM; i++)
        printf("  %2d: %08X\n", i, Reg[i]); 
    cout << "-------------------\n";
}

void CPU::setloaded() {
    loadedflag = true;
}

bool CPU::isloaded() {
    return loadedflag;
}

int CPU::getCurrSize() {
    return CurrSize;
}

const byte *CPU::getMem(dword addr) {
    return (addr < END_MEM) ? Memory+addr : NULL;
}

const dword* CPU::getReg() {
    return Reg;
}

const byte* CPU::getDispMem() {
    return Memory + END_MEM - WIDTH*HEIGHT;
}

bool CPU::is_mem_modified() {
    return mem_modified_flag;
}
