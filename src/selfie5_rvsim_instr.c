/* BASED ON 
 * riscv-simulator: A simulator for RISCV
 * Yangcheng Luo	<lyc.eecs@pku.edu.cn>
 * Haoze Wu		<wuhaoze@mprc.pku.edu.cn>
 * Copyright (C) 2014-2015 Microprocessor R&D Center (MPRC), Peking University
 * https://github.com/Oliver-Luo/MPRC-RISCV-simulator/tree/master/src
 */

//--------------------------------------------------------------------------


#ifndef __LEO2_BOARD__
#include <stdio.h>
#include <stdlib.h>
#endif


#include "selfie5_common.h"

//--------------------------------------------------------------------------

extern unsigned int rvsim_reg[];
extern unsigned int rvsim_PC;
extern unsigned char rvsim_imem[];
extern unsigned char rvsim_dmem[];

//extern FILE *debug_out_file ; 

extern unsigned int inst_count ;

#define Rd(inst) ((inst >> 7) & 0x1f)
#define Rs1(inst) ((inst >> 15) & 0x1f)
#define Rs2(inst) ((inst >> 20) & 0x1f)


static int sign_ext(int imm, unsigned int width)
{
	int sign_bit = imm >> (width - 1);
	if (sign_bit)
	{
		imm = imm | (0xffffffff << width);
	}

	return imm;
}

static unsigned int unsign_add_sign(unsigned int unsign, int sign)
{
	if (sign >= 0)
		return unsign + sign;
	else
		return unsign - (unsigned int)(~sign + 1);
}

void rvsim_lui(unsigned int inst)
{
        unsigned int rd = Rd(inst);
        unsigned int imm = (inst & 0xfffff000);

        rvsim_reg[rd] = imm;

#ifdef RVSIM_DEBUG_EXECUTION
       vl_bm_printf(3)("PC: %08x, inst: LUI, rd: %d(0x%x), imm: 0x%x\n", rvsim_PC, rd, rvsim_reg[rd], imm);
#endif
}

void rvsim_auipc(unsigned int inst)
{
        unsigned int rd = Rd(inst);
        unsigned int imm = (inst & 0xfffff000);

        rvsim_reg[rd] = rvsim_PC + imm;

#ifdef RVSIM_DEBUG_EXECUTION
       vl_bm_printf(3)("PC: %08x, inst: AUIPC, rd: %d(0x%x), imm: 0x%x\n", rvsim_PC, rd, rvsim_reg[rd], imm);
#endif
}

void rvsim_jal(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	int imm = ((inst & 0x80000000) >> 12) | ((inst & 0x7fe00000) >> 21)
		| ((inst & 0x00100000) >> 10) | ((inst & 0x000ff000) >> 1);
	imm = sign_ext(imm, 20);
	unsigned int tmp_pc = rvsim_PC;
	unsigned int target = unsign_add_sign(rvsim_PC, imm * 2);

	rvsim_reg[rd] = rvsim_PC + 4;
	rvsim_PC = target;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, New PC: %08x, inst: JAL, rd: %d(0x%x), imm: 0x%x\n", tmp_pc, rvsim_PC, rd,rvsim_reg[rd],imm);
#endif
}
//?? why why set the least-significant bit not the two?
void rvsim_jalr(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int imm = sign_ext((inst >> 20), 12);
	int target = ((int)rvsim_reg[rs1] + imm) & 0xfffffffe;
	unsigned int tmp_pc = rvsim_PC;

	rvsim_reg[rd] = rvsim_PC + 4;
	rvsim_PC = target;

#ifdef RVSIM_DEBUG_EXECUTION
                vl_bm_printf(3)("PC: %08x, New PC: %08x, inst: JALR, rd: %d(0x%x), rs1: %2d(%08x), imm: 0x%x\n", tmp_pc, rvsim_PC, rd,rvsim_reg[rd],rs1,rvsim_reg[rs1],imm);
#endif
}

void rvsim_beq(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = ((inst & 0x80000000) >> 20) | ((inst & 0x7e000000) >> 21)
		| ((inst & 0x00000f00) >> 8) | ((inst & 0x00000080) << 3);
	unsigned int tmp_pc = rvsim_PC;

	imm = sign_ext(imm, 12);

	if((int)rvsim_reg[rs1] == (int)rvsim_reg[rs2])
		rvsim_PC = unsign_add_sign(rvsim_PC, imm * 2);
	else
		rvsim_PC += 4;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, New PC: %08x, inst: BEQ, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x\n", tmp_pc, rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2], imm);
#endif
}

void rvsim_bne(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = ((inst & 0x80000000) >> 20) | ((inst & 0x7e000000) >> 21)
		| ((inst & 0x00000f00) >> 8) | ((inst & 0x00000080) << 3);
	unsigned int tmp_pc = rvsim_PC;

	imm = sign_ext(imm, 12);

	if((int)rvsim_reg[rs1] != (int)rvsim_reg[rs2])
		rvsim_PC = unsign_add_sign(rvsim_PC, imm * 2);
	else
		rvsim_PC += 4;

#ifdef RVSIM_DEBUG_EXECUTION
   vl_bm_printf(3)("PC: %08x, New PC: %08x, inst: BNE, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x\n", tmp_pc, rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2], imm);
#endif
}

void rvsim_blt(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = ((inst & 0x80000000) >> 20) | ((inst & 0x7e000000) >> 21)
		| ((inst & 0x00000f00) >> 8) | ((inst & 0x00000080) << 3);
	unsigned int tmp_pc = rvsim_PC;

	imm = sign_ext(imm, 12);

	if((int)rvsim_reg[rs1] < (int)rvsim_reg[rs2])
		rvsim_PC = unsign_add_sign(rvsim_PC, imm * 2);
	else
		rvsim_PC += 4;

#ifdef RVSIM_DEBUG_EXECUTION
                vl_bm_printf(3)("PC: %08x, New PC: %08x, inst: BLT, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x\n", tmp_pc, rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2],imm);
#endif
}

void rvsim_bge(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = ((inst & 0x80000000) >> 20) | ((inst & 0x7e000000) >> 21)
		| ((inst & 0x00000f00) >> 8) | ((inst & 0x00000080) << 3);
	unsigned int tmp_pc = rvsim_PC;

	imm = sign_ext(imm, 12);

	if((int)rvsim_reg[rs1] >= (int)rvsim_reg[rs2])
		rvsim_PC = unsign_add_sign(rvsim_PC, imm * 2);
	else
		rvsim_PC += 4;

#ifdef RVSIM_DEBUG_EXECUTION
                vl_bm_printf(3)("PC: %08x, New PC: %08x, inst: BGE, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x\n", tmp_pc, rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2], imm);
#endif
}

void rvsim_bltu(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = ((inst & 0x80000000) >> 20) | ((inst & 0x7e000000) >> 21)
		| ((inst & 0x00000f00) >> 8) | ((inst & 0x00000080) << 3);
	unsigned int tmp_pc = rvsim_PC;

	imm = sign_ext(imm, 12);

	if((unsigned int)rvsim_reg[rs1] < (unsigned int)rvsim_reg[rs2])
		rvsim_PC = unsign_add_sign(rvsim_PC, imm * 2);
	else
		rvsim_PC += 4;

#ifdef RVSIM_DEBUG_EXECUTION
                vl_bm_printf(3)("PC: %08x, New PC: %08x, inst: BLTU, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x\n", tmp_pc, rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2], imm);
#endif
}

void rvsim_bgeu(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = ((inst & 0x80000000) >> 20) | ((inst & 0x7e000000) >> 21)
		| ((inst & 0x00000f00) >> 8) | ((inst & 0x00000080) << 3);
	unsigned int tmp_pc = rvsim_PC;

	imm = sign_ext(imm, 12);

	if((unsigned int)rvsim_reg[rs1] >= (unsigned int)rvsim_reg[rs2])
		rvsim_PC = unsign_add_sign(rvsim_PC, imm * 2);
	else
		rvsim_PC += 4;

#ifdef RVSIM_DEBUG_EXECUTION
                vl_bm_printf(3)("PC: %08x, New PC: %08x, inst: BGEU, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x\n", tmp_pc, rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2], imm);
#endif
}

void rvsim_lb(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int imm = sign_ext((inst >> 20), 12);
	unsigned int addr = unsign_add_sign(rvsim_reg[rs1], imm);

	rvsim_reg[rd] = sign_ext(rvsim_dmem[addr], 8);

#ifdef RVSIM_DEBUG_EXECUTION
                vl_bm_printf(3)("PC: %08x, inst: LB, rd: %d(0x%x), rs1: %2d(%08x), imm %d\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], imm);
#endif	
}

void rvsim_lh(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int imm = sign_ext((inst >> 20), 12);
	unsigned int addr = unsign_add_sign(rvsim_reg[rs1], imm);
	unsigned int value = 0;

	value = rvsim_dmem[addr];
	value = value | rvsim_dmem[++addr] << 8;

	rvsim_reg[rd] = sign_ext(value, 16);

#ifdef RVSIM_DEBUG_EXECUTION
                vl_bm_printf(3)("PC: %08x, inst: LH, rd: %d(0x%x), rs1: %2d(%08x), imm %d\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], imm);
#endif	
}

void rvsim_lw(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int imm = sign_ext((inst >> 20), 12);
	unsigned int addr = unsign_add_sign(rvsim_reg[rs1], imm);
	unsigned int value = 0;
	int i = 0;
	
  	for (i = 0; i < 4; i++) value = value | (rvsim_dmem[addr++] << (i * 8));
	
	rvsim_reg[rd] = value;

#ifdef RVSIM_DEBUG_EXECUTION
                vl_bm_printf(3)("PC: %08x, inst: LW, rd: %d(0x%x), rs1: %2d(%08x), imm 0x%x ; <-mem[0x%x]\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], imm, addr-4);
#endif	



}

void rvsim_lbu(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int imm = sign_ext((inst >> 20), 12);
	unsigned int addr = unsign_add_sign(rvsim_reg[rs1], imm);

	rvsim_reg[rd] = rvsim_dmem[addr];

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: LBU, rd: %d(0x%x), rs1: %2d(%08x), imm %d\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], imm);
#endif	
}

void rvsim_lhu(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int imm = sign_ext((inst >> 20), 12);
	unsigned int addr = unsign_add_sign(rvsim_reg[rs1], imm);
	unsigned int value = 0;

	value = rvsim_dmem[addr];
	value = value | (rvsim_dmem[++addr]) << 8;

	rvsim_reg[rd] = value;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: LHU, rd: %d(0x%x), rs1: %2d(%08x), imm %d\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], imm);
#endif	
}

void rvsim_sb(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = sign_ext((((inst >> 20) & 0x00000fe0) | ((inst >> 7) & 0x0000001f)), 12);
	unsigned int addr = unsign_add_sign(rvsim_reg[rs1], imm);
	char value = rvsim_reg[rs2] & 0x000000ff;

	rvsim_dmem[addr]=value ;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SB, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x\n", rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2], imm);
#endif
}
// whz ?? something wrong with the memeory.if <ea0, ffffffff>,and we access addr. e9f
void rvsim_sh(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = sign_ext((((inst >> 20) & 0x00000fe0) | ((inst >> 7) & 0x0000001f)), 12);
	unsigned int addr = unsign_add_sign(rvsim_reg[rs1], imm);
	int value = rvsim_reg[rs2] & 0x0000ffff;

    rvsim_dmem[addr]   =  value & 0xff;
    rvsim_dmem[++addr] = (value & 0xff00) >> 8;
	

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SH, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x\n", rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2], imm);
#endif
}

void rvsim_sw(unsigned int inst)
{
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	int imm = sign_ext((((inst >> 20) & 0x00000fe0) | ((inst >> 7) & 0x0000001f)), 12);
	unsigned int addr = unsign_add_sign(rvsim_reg[rs1], imm);
	int value = rvsim_reg[rs2];
	int i = 0;

	for (i = 0; i < 4; i++)
	{
		rvsim_dmem[addr] = (rvsim_reg[rs2] & (0xff << (i * 8))) >> (i * 8);
		addr++;       
	}

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SW, rs1: %2d(%08x), rs2: %2d(%08x), imm: 0x%x ; ->mem[0x%x]\n", rvsim_PC, rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2], imm,addr-4);
#endif
}

void rvsim_addi(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs = Rs1(inst);
	int imm = sign_ext((inst >> 20), 12);

	rvsim_reg[rd] = rvsim_reg[rs] + imm;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: ADDI, rd: %d(0x%x), rs: %d(0x%x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs,rvsim_reg[rs], imm);
#endif
}

void rvsim_slti(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);

	int cmp1 = (int) (rvsim_reg[rs1]);
	int cmp2 = sign_ext((inst >> 20), 12);

	if (cmp1 < cmp2)
		rvsim_reg[rd] = 1;
	else
		rvsim_reg[rd] = 0;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SLTI, rd: %d(0x%x), rs1: %2d(%08x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], cmp2);
#endif
}

/*some confusion with the spec whz??*/
void rvsim_sltiu(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);

	int cmp1 = (unsigned int) (rvsim_reg[rs1]);
	int cmp2 = (unsigned int) sign_ext((inst >> 20), 12);

	if (cmp1 < cmp2)
		rvsim_reg[rd] = 1;
	else
		rvsim_reg[rd] = 0;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SLTIU, rd: %d(0x%x), rs1: %2d(%08x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], cmp2);
#endif
}

void rvsim_xori(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int	imm = sign_ext((inst >> 20), 12);

	rvsim_reg[rd] = rvsim_reg[rs1] ^ imm;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: XORI, rd: %d(0x%x), rs1: %2d(%08x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], imm);
#endif
}

void rvsim_ori(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int	imm = sign_ext((inst >> 20), 12);

	rvsim_reg[rd] = rvsim_reg[rs1] | imm;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: ORI, rd: %d(0x%x), rs1: %2d(%08x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], imm);
#endif
}

void rvsim_andi(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	int	imm = sign_ext((inst >> 20), 12);

	rvsim_reg[rd] = rvsim_reg[rs1] & imm;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: ANDI, rd: %d(0x%x), rs1: %2d(%08x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], imm);
#endif
}

void rvsim_slli(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int shift_width = (inst >> 20) & 0x1f;

	rvsim_reg[rd] = rvsim_reg[rs1] << shift_width;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SLLI, rd: %d(0x%x), rs1: %2d(%08x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], shift_width);
#endif
}

void rvsim_srli(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int shift_width = (inst >> 20) & 0x1f;

	rvsim_reg[rd] = rvsim_reg[rs1] >> shift_width;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SRLI, rd: %d(0x%x), rs1: %2d(%08x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], shift_width);
#endif
}

void rvsim_srai(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int shift_width = (inst >> 20) & 0x1f;

	rvsim_reg[rd] = rvsim_reg[rs1] >> shift_width;
	rvsim_reg[rd] = sign_ext(rvsim_reg[rd], 32 - shift_width);

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SRAI, rd: %d(0x%x), rs1: %2d(%08x) , imm: 0x%x\n", rvsim_PC, rd,rvsim_reg[rd], rs1,rvsim_reg[rs1], shift_width);
#endif
}

void rvsim_add(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);

	rvsim_reg[rd] = rvsim_reg[rs1] + rvsim_reg[rs2];

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: ADD, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_sub(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	
	rvsim_reg[rd] = rvsim_reg[rs1] - rvsim_reg[rs2];

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SUB, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_sll(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	unsigned int shift_width = rvsim_reg[rs2] & 0x1f;

	rvsim_reg[rd] = rvsim_reg[rs1] << shift_width;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SLL, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_slt(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);

	int cmp1 = (int) (rvsim_reg[rs1]);
	int cmp2 = (int) (rvsim_reg[rs2]);

	if (cmp1 < cmp2)
		rvsim_reg[rd] = 1;
	else
		rvsim_reg[rd] = 0;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SLT, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_sltu(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);

	unsigned int cmp1 = (unsigned int) (rvsim_reg[rs1]);
	unsigned int cmp2 = (unsigned int) (rvsim_reg[rs2]);

	if (cmp1 < cmp2)
		rvsim_reg[rd] = 1;
	else
		rvsim_reg[rd] = 0;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SLTU, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_xor(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	
	rvsim_reg[rd] = rvsim_reg[rs1] ^ rvsim_reg[rs2];


#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: XOR, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

/* logical right shift */
void rvsim_srl(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	unsigned int shift_width = rvsim_reg[rs2] & 0x1f;

	rvsim_reg[rd] = rvsim_reg[rs1] >> shift_width;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SRL, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

/* arithmetic right shift */
void rvsim_sra(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	unsigned int shift_width = rvsim_reg[rs2] & 0x1f;

	rvsim_reg[rd] = rvsim_reg[rs1] >> shift_width;
	rvsim_reg[rd] = sign_ext(rvsim_reg[rd], 32 - shift_width);

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: SRA, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_or(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);

	rvsim_reg[rd] = rvsim_reg[rs1] | rvsim_reg[rs2];


#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: OR, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_and(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);

	rvsim_reg[rd] = rvsim_reg[rs1] & rvsim_reg[rs2];

#define RVSIM_DEBUG_EXECUTION

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: AND, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_remu(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);

	rvsim_reg[rd] = rvsim_reg[rs1] % rvsim_reg[rs2];

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: REMU, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif	

}

void rvsim_div(unsigned int inst)
{

	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);

	int value_rs1 = rvsim_reg[rs1];
	int value_rs2 = rvsim_reg[rs2];
	int value_rd = value_rs1 / value_rs2;

	rvsim_reg[rd] = value_rd;

#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: DIV, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_divu(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);

	rvsim_reg[rd] = rvsim_reg[rs1] / rvsim_reg[rs2];
#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: DIVU, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_mul(unsigned int inst)
{
	unsigned int rd = Rd(inst);
	unsigned int rs1 = Rs1(inst);
	unsigned int rs2 = Rs2(inst);
	unsigned long long int value = (unsigned long long int)rvsim_reg[rs1] * (unsigned long long int)rvsim_reg[rs2];
	rvsim_reg[rd] = value & 0xffffffff;
#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: MUL, rd: %d(0x%x), rs1: %2d(%08x) , rs2: %2d(%08x)\n", rvsim_PC, rd,rvsim_reg[rd] ,rs1,rvsim_reg[rs1], rs2,rvsim_reg[rs2]);
#endif
}

void rvsim_fence (unsigned int inst) {
#ifdef RVSIM_DEBUG_EXECUTION
	vl_bm_printf(3)("PC: %08x, inst: FENCE\n", rvsim_PC);
#endif
}



