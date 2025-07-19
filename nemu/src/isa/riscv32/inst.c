/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include "macro.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)      // 获取寄存器i的值，原函数在reg.c中定义
#define Mr   vaddr_read  // 内存读函数，原函数在memory/vaddr.c中定义
#define Mw   vaddr_write // 内存写函数，原函数在memory/vaddr.c中定义

enum {
    TYPE_I,
    TYPE_U,
    TYPE_S,
    TYPE_N, // none

    TYPE_J,
    TYPE_B, // branch
    TYPE_R,
};

// 获取rs1和rs2寄存器的值
#define src1R()                                                                                                        \
    do {                                                                                                               \
        *src1 = R(rs1);                                                                                                \
    } while (0)
#define src2R()                                                                                                        \
    do {                                                                                                               \
        *src2 = R(rs2);                                                                                                \
    } while (0)
// 立即数或加载指令
#define immI()                                                                                                         \
    do {                                                                                                               \
        *imm = SEXT(BITS(i, 31, 20), 12);                                                                              \
    } while (0)
// 上半部立即数指令
#define immU()                                                                                                         \
    do {                                                                                                               \
        *imm = SEXT(BITS(i, 31, 12), 20) << 12;                                                                        \
    } while (0)
// 存储指令
#define immS()                                                                                                         \
    do {                                                                                                               \
        *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7);                                                       \
    } while (0);

#define immJ()                                                                                                         \
    do {                                                                                                               \
        *imm = SEXT(                                                                                                   \
            (BITS(i, 31, 31) << 20) | (BITS(i, 19, 12) << 12) | (BITS(i, 20, 20) << 11) | (BITS(i, 30, 21) << 1), 21); \
    } while (0);

#define immB()                                                                                                         \
    do {                                                                                                               \
        *imm = SEXT((BITS(i, 31, 31) << 12) | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1),  \
                    13);                                                                                               \
    } while (0);

// 解码指令并获取操作数
// s: Decode结构体指针, rd: 目标寄存器, src1: 源寄存器1的值, src2: 源寄存器2的值, imm: 立即数, type: 指令类型
// type可以是TYPE_I, TYPE_U, TYPE_S, TYPE_N等，表示不同的指令类型
static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
    uint32_t i = s->isa.inst;
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    *rd = BITS(i, 11, 7);
    switch (type) {
        // I型指令: 立即数指令
        case TYPE_I:
            src1R(); // 获取rs1寄存器的值
            immI();
            break;
        // U型指令: 上半部立即数指令
        case TYPE_U: immU(); break;
        // S型指令: 存储指令
        case TYPE_S:
            src1R(); // 获取rs1寄存器的值
            src2R(); // 获取rs2寄存器的值
            immS();
            break;
        case TYPE_J:
            immJ(); // 使用正确的J型立即数处理
            break;
        case TYPE_B:
            src1R(); // 获取rs1寄存器的值
            src2R(); // 获取rs2寄存器的值
            immB();  // 使用正确的B型立即数处理
            break;
        case TYPE_R:
            src1R();
            src2R();
            break;
        case TYPE_N: break;
        default: panic("unsupported type = %d", type);
    }
}

// 译码并执行指令
static int decode_exec(Decode *s) {
    s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst) // 获取当前指令
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)                                                           \
    {                                                                                                                  \
        int rd = 0;                                                                                                    \
        word_t src1 = 0, src2 = 0, imm = 0;                                                                            \
        decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type));                                               \
        __VA_ARGS__;                                                                                                   \
    }

    // 指令模式匹配
    // INSTPAT(模式字符串, 指令名称, 指令类型, 指令执行操作);
    /*
    * 0表示相应的位只能匹配0
    * 1表示相应的位只能匹配1
    * ?表示相应的位可以匹配0或1
    * 空格是分隔符, 只用于提升模式字符串的可读性, 不参与匹配
    */
    INSTPAT_START();

    // R型指令
    INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub, R, R(rd) = src1 - src2);                    // sub: rd = rs1 - rs2
    INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R, R(rd) = src1 + src2);                    // add: rd = rs1 + rs2
    INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and, R, R(rd) = src1 & src2);                    // and: rd = rs1 & rs2
    INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, R, R(rd) = src1 < src2 ? 1 : 0);           // sltu: rd = (rs1 < rs2) (unsigned)
    INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor, R, R(rd) = src1 ^ src2);                    // xor: rd = rs1 ^ src2
    INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or, R, R(rd) = src1 | src2);                     // or: rd = rs1 | src2
    INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul, R, R(rd) = src1 * src2);                    // mul: rd = rs1 * rs2 (lower 32 bits)
    INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh, R, R(rd) = ((int64_t)(sword_t)src1 * (int64_t)(sword_t)src2) >> 32); // mulh: rd = (rs1 * rs2) >> 32 (signed)
    INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div, R, R(rd) = (sword_t)src1 / (sword_t)src2);  // div: rd = rs1 / rs2 (signed)
    INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu, R, R(rd) = src1 / src2);                   // divu: rd = rs1 / rs2 (unsigned)
    // INSTPAT("0000001 ????? ????? 101 ????? 01110 11", divuw, R, R(rd) = SEXT(src1 / src2, 32));        // divuw: rd = rs1 / rs2 (unsigned, 32-bit op) [RV64I]
    // INSTPAT("0000001 ????? ????? 100 ????? 01110 11", divw, R,
    //         R(rd) = SEXT((sword_t)src1 / (sword_t)src2, 32)); // divw: rd = rs1 / rs2 (signed, 32-bit op) [RV64I]
    INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu, R, R(rd) = ((int64_t)(sword_t)src1 * (uint64_t)src2) >> 32);        // mulhsu: rd = (rs1 * rs2) >> 32 (signed * unsigned)
    INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu, R, R(rd) = ((uint64_t)src1 * (uint64_t)src2) >> 32);                 // mulhu: rd = (rs1 * rs2) >> 32 (unsigned)
    // INSTPAT("0000001 ????? ????? 000 ????? 01110 11", mulw, R, R(rd) = SEXT(src1 * src2, 32));        // mulw: rd = rs1 * rs2 (lower 32 bits, 32-bit op) [RV64I]
    INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem, R, R(rd) = (sword_t)src1 % (sword_t)src2); // rem: rd = rs1 % rs2 (signed)
    INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu, R, R(rd) = src1 % src2);                  // remu: rd = rs1 % rs2 (unsigned)
    // INSTPAT("0000001 ????? ????? 111 ????? 01110 11", remuw, R, R(rd) = SEXT(src1 % src2, 32));       // remuw: rd = rs1 % rsc2 (unsigned, 32-bit op) [RV64I]
    // INSTPAT("0000001 ????? ????? 110 ????? 01110 11", remw, R,
    //         R(rd) = SEXT((sword_t)src1 % (sword_t)src2, 32)); // remw: rd = rs1 % rs2 (signed, 32-bit op) [RV64I]
    INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, R, R(rd) = (sword_t)src1 < (sword_t)src2 ? 1 : 0); // slt: rd = (rs1 < rs2) (signed)
    INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra, R, R(rd) = (sword_t)src1 >> (src2 & 0x1f));          // sra: rd = rs1 >> rs2 (arithmetic)
    INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl, R, R(rd) = src1 >> (src2 & 0x1f));          // srl: rd = rs1 >> rs2 (logical)
    INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll, R, R(rd) = src1 << (src2 & 0x1f));             // sll: rd = rs1 << rs2

    // I型指令
    INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu, I, R(rd) = Mr(src1 + imm, 1));             // lbu: rd = mem[rs1+imm] (byte, unsigned)
    INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I, R(rd) = src1 + imm);                   // addi: rd = rs1 + imm
    // INSTPAT("??????? ????? ????? 000 ????? 00110 11", addiw, I, R(rd) = SEXT(src1 + imm, 32));        // addiw: rd = rs1 + imm (32-bit op) [RV64I]
    // INSTPAT("0000000 ????? ????? 000 ????? 01110 11", addw, I, R(rd) = SEXT(src1 + src2, 32));       // addw: rd = rs1 + rs2 (32-bit op) [RV64I]
    INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi, I, R(rd) = src1 & imm);                  // andi: rd = rs1 & imm
    INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr, I, R(rd) = s->dnpc, s->dnpc = (src1 + imm) & ~1); // jalr: rd=pc+4, pc=rs1+imm
    INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I, R(rd) = Mr(src1 + imm, 4));                  // lw: rd = mem[rs1+imm] (word)
    INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu, I, R(rd) = src1 < imm ? 1 : 0);              // sltiu: rd = (rs1 < imm) (unsigned)
    INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli, I, R(rd) = src1 >> (imm & 0x1f));             // srli: rd = rs1 >> imm (logical)
    INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli, I, R(rd) = src1 << (imm & 0x1f));             // slli: rd = rs1 << imm
    INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai, I, R(rd) = (sword_t)src1 >> (imm & 0x1f));    // srai: rd = rs1 >> imm (arithmetic)
    INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb, I, R(rd) = SEXT(Mr(src1 + imm, 1), 8));         // lb: rd = mem[rs1+imm] (byte, signed)
    INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu, I, R(rd) = Mr(src1 + imm, 2));       // lhu: rd = mem[rs1+imm] (half, unsigned)
    // INSTPAT("??????? ????? ????? 110 ????? 00000 11", lwu, I, R(rd) = Mr(src1 + imm, 4));                 // lwu: rd = mem[rs1+imm] (word, unsigned) [RV64I]
    INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori, I, R(rd) = src1 | imm);                        // ori: rd = rs1 | imm
    // INSTPAT("0000000 ????? ????? 101 ????? 00110 11", slliw, I, R(rd) = SEXT(src1 << (src2 & 0x1f), 32)); // slliw: rd = rs1 << shamt (32-bit op) [RV64I]
    // INSTPAT("0000000 ????? ????? 001 ????? 01110 11", sllw, I, R(rd) = SEXT(src1 << (src2 & 0x1f), 32));  // sllw: rd = rs1 << rs2 (32-bit op) [RV64I]
    INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti, I,
            R(rd) = (sword_t)src1 < (sword_t)imm ? 1 : 0); // slti: rd = (rs1 < imm) (signed)
    INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori, I, R(rd) = src1 ^ imm);                   // xori: rd = rs1 ^ imm
    INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh, I, R(rd) = SEXT(Mr(src1 + imm, 2), 16)); // lh: rd = mem[rs1+imm] (half, signed)

    // S型指令
    INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb, S, Mw(src1 + imm, 1, src2)); // sb: mem[rs1+imm] = rs2 (byte)
    INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw, S, Mw(src1 + imm, 4, src2)); // sw: mem[rs1+imm] = rs2 (word)
    // INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd, S, Mw(src1 + imm, 8, src2)); // sd: mem[rs1+imm] = rs2 (double word) [RV64I]
    INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh, S, Mw(src1 + imm, 2, src2)); // sh: mem[rs1+imm] = rs2 (half)

    // B型指令
    INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq, B, if (src1 == src2) s->dnpc = s->pc + imm);    // beq: if (rs1 == rs2) pc += imm
    INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne, B, if (src1 != src2) s->dnpc = s->pc + imm);    // bne: if (rs1 != rs2) pc += imm
    INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt, B, if ((sword_t)src1 < (sword_t)src2) s->dnpc = s->pc + imm); // blt: if (rs1 < rs2) pc += imm (signed)
    INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu, B, if (src1 >= src2) s->dnpc = s->pc + imm);   // bgeu: if (rs1 >= rs2) pc += imm (unsigned)
    INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge, B,
            if ((sword_t)src1 >= (sword_t)src2) s->dnpc = s->pc + imm); // bge: if (rs1 >= rs2) pc += imm (signed)
    INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu, B, if (src1 < src2) s->dnpc = s->pc + imm);    // bltu: if (rs1 < rs2) pc += imm (unsigned)

    // U型指令
    INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U, R(rd) = s->pc + imm); // auipc: rd = pc + imm
    INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui, U, R(rd) = imm);           // lui: rd = imm

    // J型指令
    INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal, J, R(rd) = s->dnpc, s->dnpc = s->pc + imm); // jal: rd = pc+4, pc += imm

    // N型指令
    INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N, NEMUTRAP(s->pc, R(10))); // ebreak: environment break
    INSTPAT_END();

    R(0) = 0; // reset $zero to 0

    return 0;
}

int isa_exec_once(Decode *s) {
    s->isa.inst = inst_fetch(&s->snpc, 4); // 取指,获取指令并更新s->snpc指向下一条指令的地址
    return decode_exec(s); // 译码并执行指令同时更新dnpc
}
