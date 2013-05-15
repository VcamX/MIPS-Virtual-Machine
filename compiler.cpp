#include "compiler.h"
#include "CPU.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

/* the wrapped part */
compiler::compiler() {
    init();
}

compiler::~compiler() {
    
}

void compiler::init() {
    li = 0;
    lo = 0;
    list_n = 0;
    instru_set.clear();
}

int compiler::load(std::string filename) {
    std::ifstream fin(filename.c_str());
    std::string s;
    
    if (!fin) return 1;
    while (getline(fin, s)) {
        instru_set.push_back(s);
    }
    return 0;
}

int compiler::compile() {
    for (int i = 0; i < instru_set.size(); i++) {
        strcut(instru_set[i].c_str());
        if (gen_instru(instru)) {
            std::cout << "Error in \"" << instru_set[i] << "\"\n";
            return 1;
        }
    }
    
    recheck();
    
    return 0;
}

int compiler::save(std::string filename) {
    FILE* f;
    f = fopen(filename.c_str(), "wb");
    for (int i = 0; i < list_n; i++) {
        dword t = ((list[i] >> 0) & 0xFF) << 24 |
                  ((list[i] >> 8) & 0xFF) << 16 |
                  ((list[i] >> 16) & 0xFF) << 8 |
                  (list[i] >> 24);
        fwrite(&(t), sizeof(dword), 1, f);
    }
    fclose(f);
    return 0;
}

int compiler::save(dword* commd_set) {
    for (int i = 0; i < list_n; i++)
        commd_set[i] = list[i];
    return 0;
}

void compiler::print() {
    printf("\n------- compiler result -------\n");
    for (int i = 0; i < instru_set.size(); i++) {
        std::cout << instru_set[i] << "\n\t\t\t";
        printf("%08X\n", list[i]);
    }
    printf("------- compiler result -------\n");
}

int compiler::get_commd_num() {
    return list_n;
}

int compiler::is_num(const char *s) {
    int len = strlen(s);
    int idx = s[0] == '-' || s[0] == '+';
    if (len > 2 && s[idx] == '0' && (s[idx+1] == 'X' || s[idx+1] == 'x'))
        return 16;
    if (len > 1 && s[idx] == '0')
        return 8;
    for (int i = idx; i < len; i++)
        if (!(s[i] >= '0' && s[i] <= '9'))
            return 0;
    return 10;
}
/*
std::string compiler::get_instru(int order) {
    return instru_set[order];
}
*/
/* the original part */
int compiler::strcut(const char buf[]) {
    int i = 0, m = 0, j;
    while (buf[i]) {
        while (buf[i] == ' ' || buf[i] == 9) i++;
        j = 0;
        while (buf[i] != ' ' && buf[i] != ',' &&
               buf[i] != '(' && buf[i] != ')' &&
               buf[i] != ':' && buf[i] != ';' &&
               buf[i] != '\t' && buf[i] != '\0' &&
               buf[i] != '\n' && buf[i] != 13) {
            instru[m][j++] = (buf[i] >= 'A') && (buf[i] <= 'Z') ?
                             buf[i++] + 32:
                             buf[i++];
        }
        if (!j) break;
        instru[m][j] = '\0';
        if (buf[i] == ':') {
            strcpy(label_in[li].na, instru[m]);
            label_in[li++].line = list_n;
        }
        else
            m++;
        i++;
    }
    return m;
}

dword compiler::regX(char s[]) {
    dword reg;
    switch (s[1]) {
        case 'z':
            reg = 0;
            break;
        case 'v':
            reg = s[2] - '0' + 2;
            break;
        case 'k':
            reg = s[2] - '0' + 62;
            break;
        case 'a':
            if (s[2] == 't')
                reg = 1;
            else
                reg = s[2] - '0' + 4;
            break;
        case 's':
            if (s[2] == 'p')
                reg = 29;
            else
                reg = s[2] - '0' + 16;
            break;
        case 't':
            if (s[2] - '0' >= 8)
                reg = s[2] - '0' + 24;
            else
                reg = s[2] - '0' + 8;
            break;
        case 'g':
            reg = 28;
            break;
        case 'f':
            reg = 30;
            break;
        case 'r':
            reg = 31;
            break;
        default:
            reg = -1;
            break;
    }
    return (reg & 31);
}


dword compiler::atom(char* s) {
    dword u = 0, base = is_num(s), i;
    
    if (!base) return 0;
    i = (s[0] == '-' || s[0] == '+');
    switch (base) {
        case 8: i++; break;
        case 16: i+=2; break;
    }
    for (; s[i] != '\0'; i++){
        if (base == 16 && toupper(s[i]) >= 'A' && toupper(s[i]) <= 'F')
            u = (u*base) + (toupper(s[i])-'A'+10);
        else
            u = (u*base) + (s[i]-'0');
    }
    
    // using ones' complement to calculate two's complement
    if (s[0]=='-')
        u = ~u+1;
    return u;
}

dword compiler::immed(char s[], dword mask, int div) {
    int  p = 0;
    dword r = 0;
    char ch = 0;
    char *s1 = NULL;
    
    /* split */
    while (s[p]) {
        if ((s[p] == '+' && p) ||
            (s[p] == '-' && p) ||
            (s[p] == '*')) {
            ch = s[p];
            s1 = s+p+1;
            s[p] = '\0';
        }
        p++;
    }
    
    /* calc */ 
    switch (ch) {
        case '+': r = atom(s) + atom(s1); break;
        case '-': r = atom(s) - atom(s1); break;
        case '*': r = atom(s) * atom(s1); break;
        default: r = atom(s); break;
    }
    
    r = div ? r >> 2 : r;
    return r & mask;
}

dword compiler::immed_addr(char s[]) {
    dword r;
    
    r = (atom(s) - 4*list_n - 4);
    r >>= 2;
    
    return r & 65535;
}

dword compiler::gen_core(char s[][ARG_LEN]) {
    if (!strcmp(s[0], "add"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 32;

    else if (!strcmp(s[0], "addi"))
        return (8 << 26) |
               (regX(s[2]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[3], 65535, 0));
    
    else if (!strcmp(s[0], "addiu"))
        return (9 << 26) |
               (regX(s[2]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[3], 65535, 0));
               
    else if (!strcmp(s[0], "addu"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 33;
    
    else if (!strcmp(s[0], "and"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 36;
               
    else if (!strcmp(s[0], "andi"))
        return (12 << 26) |
               (regX(s[2]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[3], 65535, 0));

    else if (!strcmp(s[0], "beq")) {
        if (is_num(s[3]))
            return (4 << 26) |
                   (regX(s[1]) << 21) |
                   (regX(s[2]) << 16) |
                   (immed_addr(s[3]));
        else {
            strcpy(label_out[lo].na, s[3]);
            label_out[lo].type = 'I';
            label_out[lo++].line = list_n;
            return (4 << 26) |
                   (regX(s[1]) << 21) |
                   (regX(s[2]) << 16);
        }
    }
    
    else if (!strcmp(s[0], "bgez")) {
        if (is_num(s[2]))
            return (1 << 26) |
                   (regX(s[1]) << 21) |
                   (1 << 16) |
                   (immed_addr(s[2]));
        else {
            strcpy(label_out[lo].na, s[2]);
            label_out[lo].type = 'I';
            label_out[lo++].line = list_n;
            return (1 << 26) |
                   (regX(s[1]) << 21) |
                   (1 << 16);
        }
    }
    
    else if (!strcmp(s[0], "bgezal")) {
        if (is_num(s[2]))
            return (1 << 26) |
                   (regX(s[1]) << 21) |
                   (17 << 16) |
                   (immed_addr(s[2]));
        else {
            strcpy(label_out[lo].na, s[2]);
            label_out[lo].type = 'I';
            label_out[lo++].line = list_n;
            return (1 << 26) |
                   (regX(s[1]) << 21) |
                   (17 << 16);
        }
    }
    
    else if (!strcmp(s[0], "bgtz")) {
        if (is_num(s[2]))
            return (7 << 26) |
                   (regX(s[1]) << 21) |
                   (immed_addr(s[2]));
        else {
            strcpy(label_out[lo].na, s[2]);
            label_out[lo].type = 'I';
            label_out[lo++].line = list_n;
            return (7 << 26) |
                   (regX(s[1]) << 21);
        }
    }
    
    else if (!strcmp(s[0], "blez")) {
        if (is_num(s[2]))
            return (6 << 26) |
                   (regX(s[1]) << 21) |
                   (immed_addr(s[2]));
        else {
            strcpy(label_out[lo].na, s[2]);
            label_out[lo].type = 'I';
            label_out[lo++].line = list_n;
            return (6 << 26) |
                   (regX(s[1]) << 21);
        }
    }
    
    else if (!strcmp(s[0], "bltz")) {
        if (is_num(s[3]))
            return (1 << 26) |
                   (regX(s[1]) << 21) |
                   (immed_addr(s[2]));
        else {
            strcpy(label_out[lo].na, s[2]);
            label_out[lo].type = 'I';
            label_out[lo++].line = list_n;
            return (1 << 26) |
                   (regX(s[1]) << 21);
        }
    }
    
    else if (!strcmp(s[0], "bltzal")) {
        if (is_num(s[2]))
            return (1 << 26) |
                   (regX(s[1]) << 21) |
                   (16 << 16) |
                   (immed_addr(s[2]));
        else {
            strcpy(label_out[lo].na, s[2]);
            label_out[lo].type = 'I';
            label_out[lo++].line = list_n;
            return (1 << 26) |
                   (regX(s[1]) << 21) |
                   (16 << 16);
        }
    }
    
    else if (!strcmp(s[0], "bne")) {
        if (is_num(s[3]))
            return (5 << 26) |
                   (regX(s[1]) << 21) |
                   (regX(s[2]) << 16) |
                   (immed_addr(s[3]));
        else {
            strcpy(label_out[lo].na, s[3]);
            label_out[lo].type = 'I';
            label_out[lo++].line = list_n;
            return (5 << 26) |
                   (regX(s[1]) << 21) |
                   (regX(s[2]) << 16);
        }
    }
    
    else if (!strcmp(s[0], "div"))
        return (regX(s[1]) << 21) |
               (regX(s[2]) << 16) | 26;

    else if (!strcmp(s[0], "divu"))
        return (regX(s[1]) << 21) |
               (regX(s[2]) << 16) | 27;

    else if (!strcmp(s[0], "j")) {
        if (is_num(s[1]))
            return (2 << 26) |
                   (immed(s[1], 67108863, 1));
        else {
            strcpy(label_out[lo].na, s[1]);
            label_out[lo].type = 'J';
            label_out[lo++].line = list_n;
            return (2 << 26);
        }
    }
    
    else if (!strcmp(s[0], "jal")) {
        if (is_num(s[1]))
            return (3 << 26) |
                   (immed(s[1], 67108863, 1));
        else {
            strcpy(label_out[lo].na, s[1]);
            label_out[lo].type = 'J';
            label_out[lo++].line = list_n;
            return (3 << 26);
        }
    }
    
    else if (!strcmp(s[0], "jr"))
        return (regX(s[1]) << 21) | 8;

    else if (!strcmp(s[0], "lb"))
        return (32 << 26) |
               (regX(s[3]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));
               
    else if (!strcmp(s[0], "lbu"))
        return (36 << 26) |
               (regX(s[3]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));
               
    else if (!strcmp(s[0], "lh"))
        return (33 << 26) |
               (regX(s[3]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));
               
    else if (!strcmp(s[0], "lhu"))
        return (37 << 26) |
               (regX(s[3]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));
    
    else if (!strcmp(s[0], "lui"))
        return (15 << 26) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));

    else if (!strcmp(s[0], "lw"))
        return (35 << 26) |
               (regX(s[3]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));
               
    else if (!strcmp(s[0], "mfhi"))
        return (regX(s[1]) << 11) | 16;
    
    else if (!strcmp(s[0], "mflo"))
        return (regX(s[1]) << 11) | 18;
    
    else if (!strcmp(s[0], "mult"))
        return (regX(s[1]) << 21) |
               (regX(s[2]) << 16) | 24;
    
    else if (!strcmp(s[0], "multu"))
        return (regX(s[1]) << 21) |
               (regX(s[2]) << 16) | 25;

    else if (!strcmp(s[0], "noop"))
        return 0;
    
    else if (!strcmp(s[0], "nor"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 39;
               
    else if (!strcmp(s[0], "or"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 37;
               
    else if (!strcmp(s[0], "ori"))
        return (13 << 26) |
               (regX(s[2]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[3], 65535, 0));

    else if (!strcmp(s[0], "sb"))
        return (40 << 26) |
               (regX(s[3]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));
               
    else if (!strcmp(s[0], "sh"))
        return (41 << 26) |
               (regX(s[3]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));
               
    else if (!strcmp(s[0], "sll"))
        return (regX(s[2]) << 16) |
               (regX(s[1]) << 11) |
               (immed(s[3], 31, 0) << 6) | 0;
               
    else if (!strcmp(s[0], "sllv"))
        return (regX(s[3]) << 21) |
               (regX(s[2]) << 16) |
               (regX(s[1]) << 11) | 4;
    
    else if (!strcmp(s[0], "slt"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 42;
               
    else if (!strcmp(s[0], "slti"))
        return (10 << 26) |
               (regX(s[2]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[3], 65535, 0));

    else if (!strcmp(s[0], "sltiu"))
        return (11 << 26) |
               (regX(s[2]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[3], 65535, 0));

    else if (!strcmp(s[0], "sltu"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 43;
               
    else if (!strcmp(s[0], "sra"))
        return (regX(s[2]) << 16) |
               (regX(s[1]) << 11) |
               (immed(s[3], 31, 0) << 6) | 3;

    else if (!strcmp(s[0], "srl"))
        return (regX(s[2]) << 16) |
               (regX(s[1]) << 11) |
               (immed(s[3], 31, 0) << 6) | 2;
           
    else if (!strcmp(s[0], "srlv"))
        return (regX(s[3]) << 21) |
               (regX(s[2]) << 16) |
               (regX(s[1]) << 11) | 6;
    
    else if (!strcmp(s[0], "sub"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 34;
                          
    else if (!strcmp(s[0], "subu"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 35;
        
    else if (!strcmp(s[0], "sw"))
        return (43 << 26) |
               (regX(s[3]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[2], 65535, 0));
               
    else if (!strcmp(s[0], "syscall"))
        return 12;
        
    else if (!strcmp(s[0], "xor"))
        return (regX(s[2]) << 21) |
               (regX(s[3]) << 16) |
               (regX(s[1]) << 11) | 38;
               
    else if (!strcmp(s[0], "xori"))
        return (14 << 26) |
               (regX(s[2]) << 21) |
               (regX(s[1]) << 16) |
               (immed(s[3], 65535, 0));

    else
        return -1;
}

void compiler::recheck() {
    int i, j;
    for (i = 0; i < lo; i++) {
        for (j = 0; j < li; j++) {
            if (!strcmp(label_out[i].na, label_in[j].na)) {
                switch (label_out[i].type) {
                    case 'I':
                        list[label_out[i].line] |=
                            (label_in[j].line - label_out[i].line - 1) & 0xFFFF;
                        break;
                    case 'J':
                        list[label_out[i].line] |=
                            ((CPU::USER_MEM + (label_in[j].line << 2)) >> 2) & 0x3FFFFFF;
                        break;
                    case 'P':
                        list[label_out[i].line] |=
                            ((CPU::USER_MEM + (label_in[j].line << 2)) >> 16) & 0xFFFF;
                        break;
                    case 'p':
                        list[label_out[i].line] |= 
                            (CPU::USER_MEM + (label_in[j].line << 2)) & 0xFFFF;
                        break;
                }
                break;
            }
        }
    }
}

int compiler::gen_instru(char s[][ARG_LEN]) {
    char temp[5][ARG_LEN];
    
    /* Pseudo-type */
    if (!strcmp(s[0], "move")) {
        strcpy(temp[0], "addi");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], s[2]);
        strcpy(temp[3], "0");
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "clear")) {
        strcpy(temp[0], "add");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], "$zero");
        strcpy(temp[3], "$zero");
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "not")) {
        strcpy(temp[0], "not");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], s[2]);
        strcpy(temp[3], "$zero");
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "la")) {        
        strcpy(label_out[lo].na, s[2]);
        label_out[lo].type = 'P';
        label_out[lo++].line = list_n;
        
        strcpy(temp[0], "lui");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], "0");
        list[list_n++] = gen_core(temp);
        
        
        strcpy(label_out[lo].na, s[2]);
        label_out[lo].type = 'p';
        label_out[lo++].line = list_n;
        
        strcpy(temp[0], "ori");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], s[1]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "li")) {
        dword addr = atom(s[2]);
        strcpy(temp[0], "lui");
        strcpy(temp[1], s[1]);
        itoa(addr >> 16, temp[2], 10);
        list[list_n++] = gen_core(temp);
        
        strcpy(temp[0], "ori");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], s[1]);
        itoa(addr & 0xFFFF, temp[3], 10);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "b")) {
        strcpy(temp[0], "beq");
        strcpy(temp[1], "$zero");
        strcpy(temp[2], "$zero");
        strcpy(temp[3], s[1]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "bal")) {
        strcpy(temp[0], "bgezal");
        strcpy(temp[1], "$zero");
        strcpy(temp[2], s[1]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "bgt")) {
        strcpy(temp[0], "slt");
        strcpy(temp[1], "$at");
        strcpy(temp[2], s[2]);
        strcpy(temp[3], s[1]);
        list[list_n++] = gen_core(temp);
        
        strcpy(temp[0], "beq");
        strcpy(temp[1], "$at");
        strcpy(temp[2], "$zero");
        strcpy(temp[3], s[3]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "blt")) {
        strcpy(temp[0], "slt");
        strcpy(temp[1], "$at");
        strcpy(temp[2], s[1]);
        strcpy(temp[3], s[2]);
        
        strcpy(temp[0], "beq");
        strcpy(temp[1], "$at");
        strcpy(temp[2], "$zero");
        strcpy(temp[3], s[3]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "bge")) {
        strcpy(temp[0], "slt");
        strcpy(temp[1], "$at");
        strcpy(temp[2], s[1]);
        strcpy(temp[3], s[2]);
        list[list_n++] = gen_core(temp);
        
        strcpy(temp[0], "bne");
        strcpy(temp[1], "$at");
        strcpy(temp[2], "$zero");
        strcpy(temp[3], s[3]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "ble")) {
        strcpy(temp[0], "slt");
        strcpy(temp[1], "$at");
        strcpy(temp[2], s[2]);
        strcpy(temp[3], s[1]);
        list[list_n++] = gen_core(temp);
        
        strcpy(temp[0], "bne");
        strcpy(temp[1], "$at");
        strcpy(temp[2], "$zero");
        strcpy(temp[3], s[3]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "bgtu")) {
        strcpy(temp[0], "sltu");
        strcpy(temp[1], "$at");
        strcpy(temp[2], s[2]);
        strcpy(temp[3], s[1]);
        list[list_n++] = gen_core(temp);
        
        strcpy(temp[0], "beq");
        strcpy(temp[1], "$at");
        strcpy(temp[2], "$zero");
        strcpy(temp[3], s[3]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "bgtz")) {
        strcpy(temp[0], "slt");
        strcpy(temp[1], "$at");
        strcpy(temp[2], "$zero");
        strcpy(temp[3], s[1]);
        list[list_n++] = gen_core(temp);
        
        strcpy(temp[0], "beq");
        strcpy(temp[1], "$at");
        strcpy(temp[2], "$zero");
        strcpy(temp[3], s[3]);
        list[list_n++] = gen_core(temp);
    }
    
    else if (!strcmp(s[0], "beqz")) {
        strcpy(temp[0], "beq");
        strcpy(temp[1], "$zero");
        strcpy(temp[2], s[1]);
        strcpy(temp[3], s[2]);
        list[list_n++] = gen_core(temp);
    }

    else if (!strcmp(s[0], "rol")) {
        dword t = atom(s[3]) & 0x1F;
        strcpy(temp[0], "srl");
        strcpy(temp[1], "$at");
        strcpy(temp[2], s[2]);
        sprintf(temp[3], "%d", 0x20 - t);
        printf("%X\n", t);
        list[list_n++] = gen_core(temp);

        strcpy(temp[0], "sll");
        strcpy(temp[1], s[2]);
        strcpy(temp[2], s[2]);
        //sprintf(temp[3], "%d", t);
        list[list_n++] = gen_core(temp);

        strcpy(temp[0], "or");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], s[2]);
        strcpy(temp[3], "$at");
        list[list_n++] = gen_core(temp);
    }

    else if (!strcmp(s[0], "push")) {
        strcpy(temp[0], "addi");
        strcpy(temp[1], "$sp");
        strcpy(temp[2], "$sp");
        strcpy(temp[3], "-4");
        list[list_n++] = gen_core(temp);

        strcpy(temp[0], "sw");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], "0");
        strcpy(temp[3], "$sp");
        list[list_n++] = gen_core(temp);
    }

    else if (!strcmp(s[0], "pop")) {
        strcpy(temp[0], "lw");
        strcpy(temp[1], s[1]);
        strcpy(temp[2], "0");
        strcpy(temp[3], "$sp");
        list[list_n++] = gen_core(temp);

        strcpy(temp[0], "addi");
        strcpy(temp[1], "$sp");
        strcpy(temp[2], "$sp");
        strcpy(temp[3], "4");
        list[list_n++] = gen_core(temp);
    }

    else {
        list[list_n++] = gen_core(s);
        if (list[list_n-1] != -1)
            return 0;
        else
            return 1;
    }
    return 0;
}
