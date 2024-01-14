
#ifndef _SELFIE5_RVSIM_H_PARSED__

/* BASED ON riscv-simulator: A simulator for RISCV
 * Yangcheng Luo	<lyc.eecs@pku.edu.cn>
 * Haoze Wu		<wuhaoze@mprc.pku.edu.cn>
 * Copyright (C) 2014-2015 Microprocessor R&D Center (MPRC), Peking University
 * https://github.com/Oliver-Luo/MPRC-RISCV-simulator/tree/master/src
 */

bool_t rvsim_exec_prog(unsigned int entry_addr); // external function prototype
void rvsim_print_reg_range(int base_reg , int num_regs) ;// external function prototype
void rvsim_null_regs_range(int base_reg , int num_regs) ; // external function prototype

#define RVSIM_DEBUG_EXECUTION

void rvsim_lui   (unsigned int inst);
void rvsim_auipc (unsigned int inst);
void rvsim_jal   (unsigned int inst);
void rvsim_jalr  (unsigned int inst);
void rvsim_beq   (unsigned int inst);
void rvsim_bne   (unsigned int inst);
void rvsim_blt   (unsigned int inst);
void rvsim_bge   (unsigned int inst);
void rvsim_bltu  (unsigned int inst);
void rvsim_bgeu  (unsigned int inst);
void rvsim_lb    (unsigned int inst);
void rvsim_lh    (unsigned int inst);
void rvsim_lw    (unsigned int inst);
void rvsim_lbu   (unsigned int inst);
void rvsim_lhu   (unsigned int inst);
void rvsim_sb    (unsigned int inst);
void rvsim_sh    (unsigned int inst);
void rvsim_sw    (unsigned int inst);
void rvsim_addi  (unsigned int inst);
void rvsim_slti  (unsigned int inst);
void rvsim_sltiu (unsigned int inst);
void rvsim_xori  (unsigned int inst);
void rvsim_ori   (unsigned int inst);
void rvsim_andi  (unsigned int inst);
void rvsim_slli  (unsigned int inst);
void rvsim_srli  (unsigned int inst);
void rvsim_srai  (unsigned int inst);
void rvsim_add   (unsigned int inst);
void rvsim_sub   (unsigned int inst);
void rvsim_sll   (unsigned int inst);
void rvsim_slt   (unsigned int inst);
void rvsim_sltu  (unsigned int inst);
void rvsim_xor   (unsigned int inst);
void rvsim_srl   (unsigned int inst);
void rvsim_sra   (unsigned int inst);
void rvsim_or    (unsigned int inst);
void rvsim_and   (unsigned int inst);
void rvsim_remu  (unsigned int inst);
void rvsim_divu  (unsigned int inst);
void rvsim_div   (unsigned int inst);
void rvsim_mul   (unsigned int inst);
void rvsim_fence (unsigned int inst);

typedef enum {
	rvsim_LUI,
	rvsim_AUIPC,
	rvsim_JAL,
	rvsim_JALR,
	rvsim_BEQ,
	rvsim_BNE,
	rvsim_BLT,
	rvsim_BGE,
	rvsim_BLTU,
	rvsim_BGEU,
	rvsim_LB,
	rvsim_LH,
	rvsim_LW,
	rvsim_LBU,
	rvsim_LHU,
	rvsim_SB,
	rvsim_SH,
	rvsim_SW,
	rvsim_ADDI,
	rvsim_SLTI,
	rvsim_SLTIU,
	rvsim_XORI,
	rvsim_ORI,
	rvsim_ANDI,
	rvsim_SLLI,
	rvsim_SRLI,
	rvsim_SRAI,
	rvsim_ADD,
	rvsim_SUB,
	rvsim_SLL,
	rvsim_SLT,
	rvsim_SLTU,
	rvsim_XOR,
	rvsim_SRL,
	rvsim_SRA,
	rvsim_OR,
	rvsim_AND,
	rvsim_DIVU,
	rvsim_FENCE,
	rvsim_FENCE_I,
	rvsim_MUL,
	rvsim_MULH,
	rvsim_MULHSU,
	rvsim_MULHU,
	rvsim_DIV,
	rvsim_REM,
	rvsim_REMU,
	rvsim_ILL  // illegal /non-supported
} rvsim_instr_type_t ;


#define _SELFIE5_RVSIM_H_PARSED__
#endif
