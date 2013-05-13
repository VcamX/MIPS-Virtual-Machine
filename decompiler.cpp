#include "decompiler.h"
#include "CPU.h"

#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* the wrapped part */
decompiler::decompiler() {
    instru_set.clear();
}

decompiler::~decompiler() {
    
}

int decompiler::load(std::string filename, int mode) {
    FILE* f;
    byte mem[5];
    char s[50];
    dword cnt = 0, pc = CPU::USER_MEM, code;
    std::string str;
    
    if (mode == 1) printf("\n----- decompiler test mode -----\n");
    f = fopen(filename.c_str(), "rb");
    while (fread(&(mem[cnt++]), sizeof(byte), 1, f)) {
        pc++;
        if (!(cnt % 4)) {
            code = (mem[cnt-4] << 24) | (mem[cnt-3] << 16) |
                   (mem[cnt-2] << 8) | mem[cnt-1];
            trans(s, code, pc - 4);
            if (mode == 1) printf("  %s\n", s);
            str.assign(s);
            instru_set.push_back(str);
            cnt = 0;
        }
    }
    fclose(f);
    if (mode == 1) printf("----- decompiler test mode -----\n");
    return 0;
}

int decompiler::load(const dword* mem, dword size, int mode) {
    char s[50];
    std::string str;
    
    if (mode == 1) printf("\n----- decompiler test mode -----\n");
    for (dword i = 0; i < size; i++) {
        trans(s, mem[i], i*4 + CPU::USER_MEM);
        if (mode == 1) printf("  %s\n", s);
        str.assign(s);
        instru_set.push_back(str);
    }
    if (mode == 1) printf("----- decompiler test mode -----\n");
    return 0;
}

std::string decompiler::get_instru(int order) {
    return instru_set[order];
}

int decompiler::get_instru_num() {
    return instru_set.size();
}

void decompiler::print() {
    printf("\n------- decompiler result -------\n");
    for (int i = 0; i < instru_set.size(); i++) {
        std::cout << instru_set[i] << std::endl;
    }
    printf("------- decompiler result -------\n");
}

/* the original part */
int decompiler::regName(char s[], dword r) {
    if (r >= 8 && r <= 15)
        sprintf(s, "$t%d", r-8);
    else if (r >= 16 && r <= 23)
        sprintf(s, "$s%d", r-16);
    else switch (r) {
        case 0: strcpy(s, "$zero"); break;
        case 1: strcpy(s, "$at"); break;
        case 2: strcpy(s, "$v0"); break;
        case 3: strcpy(s, "$v1"); break;
        case 4: strcpy(s, "$a0"); break;
        case 5: strcpy(s, "$a1"); break;
        case 6: strcpy(s, "$a2"); break;
        case 7: strcpy(s, "$a3"); break;
        case 24: strcpy(s, "$t8"); break;
        case 25: strcpy(s, "$t9"); break;
        case 26: strcpy(s, "$k0"); break;
        case 27: strcpy(s, "$k1"); break;
        case 28: strcpy(s, "$gp"); break;
        case 29: strcpy(s, "$sp"); break;
        case 30: strcpy(s, "$fp"); break;
        case 31: strcpy(s, "$ra"); break;
        case 32: strcpy(s, "$HI"); break;
        case 33: strcpy(s, "$LO"); break;
    }
    return 1;
}

int decompiler::trans(char s[], dword ir, dword pc) {
    char crs[10];
    char crt[10];
    char crd[10];
    dword op, rs, rt, rd, shmt, func, addr;
    short data;
    
    op = ir >> 26;
    rs = (ir >> 21) & 31;
    regName(crs, rs);
    rt = (ir >> 16) & 31;
    regName(crt, rt);
    rd = (ir >> 11) & 31;
    regName(crd, rd);
    shmt = (ir >> 6) & 31;
    func = ir & 63;
    data = (short)ir & 0xFFFF;
    addr = (pc & 0xF0000000) | ((ir & 0x3FFFFFF) << 2);
    
    switch (op) {
        case 0:
            switch (func) {
                case 12:
                    sprintf(s, "syscall");
                    break;
                    
                case 16:
                    sprintf(s, "mfhi %s", crd);
                    break;
                
                case 18:
                    sprintf(s, "mflo %s", crd);
                    break;
                
                case 24:
                    sprintf(s, "mult %s, %s", crs, crt);
                    break;
                
                case 25:
                    sprintf(s, "multu %s, %s", crs, crt);
                    break;
                
                case 26:
                    sprintf(s, "div %s, %s", crs, crt);
                    break;
                
                case 27:
                    sprintf(s, "divu %s, %s", crs, crt);
                    break;
                
                case 32:
                    sprintf(s, "add %s, %s, %s", crd, crs, crt);
                    break;
                
                case 33:
                    sprintf(s, "addu %s, %s, %s", crd, crs, crt);
                    break;
                
                case 34:
                    sprintf(s, "sub %s, %s, %s", crd, crs, crt);
                    break;
                
                case 35:
                    sprintf(s, "subu %s, %s, %s", crd, crs, crt);
                    break;
                
                case 36:
                    sprintf(s, "and %s, %s, %s", crd, crs, crt);
                    break;
                
                case 37:
                    sprintf(s, "or %s, %s, %s", crd, crs, crt);
                    break;
                
                case 38:
                    sprintf(s, "xor %s, %s, %s", crd, crs, crt);
                    break;
                
                case 39:
                    sprintf(s, "nor %s, %s, %s", crd, crs, crt);
                    break;
                    
                case 42:
                    sprintf(s, "slt %s, %s, %s", crd, crs, crt);
                    break;
                    
                case 43:
                    sprintf(s, "sltu %s, %s, %s", crd, crs, crt);
                    break;
                    
                case 0:
                    sprintf(s, "sll %s, %s, %d", crd, crt, shmt);
                    break;
                    
                case 2:
                    sprintf(s, "srl %s, %s, %d", crd, crt, shmt);
                    break;
                    
                case 3:
                    sprintf(s, "sra %s, %s, %d", crd, crt, shmt);
                    break;
                    
                case 4:
                    sprintf(s, "sllv %s, %s, %s", crd, crt, crs);
                    break;
                    
                case 6:
                    sprintf(s, "srlv %s, %s, %s", crd, crt, crs);
                    break;
                    
                case 8:
                    sprintf(s, "jr %s", crs);
                    break;
            }
            break;
        
        case 1:
            switch (rt) {
                case 0:
                    sprintf(s, "bltz %s, %06X", crs, pc + 4 + data*4);
                    break;
                case 1:
                    sprintf(s, "bgez %s, %06X", crs, pc + 4 + data*4);
                    break;
                case 16:
                    sprintf(s, "bltzal %s, %06X", crs, pc + 4 + data*4);
                    break;
                case 17:
                    sprintf(s, "bgezal %s, %06X", crs, pc + 4 + data*4);
                    break;
            }
            break;
            
        case 4:
            sprintf(s, "beq %s, %s, %06X", crs, crt, pc + 4 + data*4);
            break;
            
        case 5:
            /*
            if (data & 0x8000) {
                data = ~data + 1;
                sprintf(s, "bne %s, %s, %06X", crs, crt, pc + 4 - data*4);
            }
            else
                sprintf(s, "bne %s, %s, %06X", crs, crt, pc + 4 + data*4);
            */
            sprintf(s, "bne %s, %s, %06X", crs, crt, pc + 4 + data*4);
            break;
            
        case 6:
            sprintf(s, "blez %s, %06X", crs, pc + 4 + data*4);
            break;
            
        case 7:
            sprintf(s, "bgtz %s, %06X", crs, pc + 4 + data*4);
            break;
            
        case 8:
            sprintf(s, "addi %s, %s, %d", crt, crs, data);
            break;
            
        case 9:
            sprintf(s, "addiu %s, %s, %d", crt, crs, data);
            break;
            
        case 10:
            sprintf(s, "slti %s, %s, %d", crt, crs, data);
            break;
            
        case 11:
            sprintf(s, "sltiu %s, %s, %d", crt, crs, data);
            break;
            
        case 12:
            sprintf(s, "andi %s, %s, %d", crt, crs, data);
            break;
            
        case 13:
            sprintf(s, "ori %s, %s, %d", crt, crs, data);
            break;
            
        case 14:
            sprintf(s, "xori %s, %s, %d", crt, crs, data);
            break;
            
        case 15:
            sprintf(s, "lui %s, %d", crt, data);
            break;
            
        case 32:
            sprintf(s, "lb %s, %d(%s)", crt, data, crs);
            break;
            
        case 33:
            sprintf(s, "lh %s, %d(%s)", crt, data, crs);
            break;
                        
        case 35:
            sprintf(s, "lw %s, %d(%s)", crt, data, crs);
            break;
                        
        case 36:
            sprintf(s, "lbu %s, %d(%s)", crt, data, crs);
            break;
            
        case 37:
            sprintf(s, "lhu %s, %d(%s)", crt, data, crs);
            break;
        
        case 40:
            sprintf(s, "sb %s, %d(%s)", crt, data, crs);
            break;
                        
        case 41:
            sprintf(s, "sh %s, %d(%s)", crt, data, crs);
            break;
                        
        case 43:
            sprintf(s, "sw %s, %d(%s)", crt, data, crs);
            break;
            
        case 2:
            sprintf(s, "j %06X", addr);
            break;
            
        case 3:
            sprintf(s, "jal %06X", addr);
            break;
            
        default:
            return 1;
    }
    
    return 0;
}
