/* BASED ON riscv-simulator: A simulator for RISCV
 * Yangcheng Luo	<lyc.eecs@pku.edu.cn>
 * Haoze Wu		<wuhaoze@mprc.pku.edu.cn>
 * Copyright (C) 2014-2015 Microprocessor R&D Center (MPRC), Peking University
  * https://github.com/Oliver-Luo/MPRC-RISCV-simulator/tree/master/src
 */

#include "selfie5_common.h"

#define	rvsim_REGSIZE	32
unsigned int rvsim_reg[rvsim_REGSIZE];

#define	rvsim_IMEM_SIZE	1024 // in bytes
char rvsim_imem[rvsim_IMEM_SIZE];
#define rvsim_IMEM_BASE_ADDR DUT_IMEM_CODE_START_ADDR 

#define	rvsim_DMEM_SIZE	1024 // in bytes
unsigned char rvsim_dmem[rvsim_DMEM_SIZE];

static unsigned int rvsim_decode(unsigned int single_inst);
static unsigned int rvsim_fetch(unsigned int pc);

extern selfie5_instr_t instructions_array[];

extern unsigned int  initial_registers_array[]; // This array maintains the randomly initialized content of the processor registers
extern unsigned int  initial_dmem_array[];      // This array maintains the randomly initialized content of the tested data-memory

void rvsim_null_regs_range(int base_reg , int num_regs){
     bm_printf("\nRVSIM Initial DUT Registers:\n");
    for (int r = base_reg; r < (base_reg+num_regs) ; r++) {
        rvsim_reg[r] = initial_registers_array[r] ;
        bm_printf("rvsim_reg[%d] = %11d = 0x%08x\n",r,rvsim_reg[r],rvsim_reg[r]);      
    }
}

void rvsim_init_dmem_range(int dmem_base_reg, int dmem_base_offset, int num_dmem_entries) {
    
   unsigned char * initial_dmem_array_byte_vec ;
        
   rvsim_reg[dmem_base_reg]  =  rvsim_DMEM_SIZE-(num_dmem_entries*4) ; 
   int dmem_test_base_addr = rvsim_reg[dmem_base_reg] + dmem_base_offset ; 
   
   bm_printf("\nRVSIM Initializing Memory:\n");
   for (int i=0;i<num_dmem_entries;i++) {
        initial_dmem_array_byte_vec = (unsigned char *)&(initial_dmem_array[i]) ;
        int dmem_addr = dmem_test_base_addr + i*4 ;
        rvsim_dmem[dmem_addr]   = initial_dmem_array_byte_vec[0] ;
        rvsim_dmem[dmem_addr+1] = initial_dmem_array_byte_vec[1] ;
        rvsim_dmem[dmem_addr+2] = initial_dmem_array_byte_vec[2] ;
        rvsim_dmem[dmem_addr+3] = initial_dmem_array_byte_vec[3] ;               

        bm_printf("M[%d] = %11d = %08x ; ", i,initial_dmem_array[i],initial_dmem_array[i]);
        bm_printf("rvsim_dmem[%04x]=%02x,[%04x]=%02x,[%04x]=%02x,[%04x]=%02x\n",
        dmem_addr+3, rvsim_dmem[dmem_addr+3],
        dmem_addr+2, rvsim_dmem[dmem_addr+2],
        dmem_addr+1, rvsim_dmem[dmem_addr+1],
        dmem_addr  , rvsim_dmem[dmem_addr]);
   }
}    

unsigned int rvsim_PC  ;

unsigned int inst_count = 0;

bool_t rvsim_exec_prog(unsigned int entry_addr)
{
	unsigned int cmd = 0;
	unsigned int opcode = 0;
    
    rvsim_PC = entry_addr ;

	inst_count = 0;
   
	vl_bm_printf(3)("\n\nRVSIM Content of Execution\n\n");
		
	while(True)
	{
		cmd = rvsim_fetch(rvsim_PC);

		if (cmd == 0) {
            vl_bm_printf(3)("\n\nRVSIM execution terminated on cmd = 0\n\n");
			return True;
		}


		opcode = rvsim_decode(cmd);

		inst_count++;
        
        if (inst_count==EXEC_INSTR_COUNT_LIMIT){
            vl_bm_printf(3)("RVSIM execution instruction counter exceeded EXEC_INSTR_COUNT_LIMIT\n") ;
            return False ;
        }

		rvsim_reg[0] = 0; //Make sure x0 = 0 after every execution.


		switch(opcode)
		{
			case rvsim_LUI:     rvsim_lui(cmd);       break;
			case rvsim_AUIPC:   rvsim_auipc(cmd);     break;
			case rvsim_JAL:     rvsim_jal(cmd);       continue;
			case rvsim_JALR:    rvsim_jalr(cmd);      continue;
			case rvsim_BEQ:     rvsim_beq(cmd);       continue;
			case rvsim_BNE:     rvsim_bne(cmd);       continue;
			case rvsim_BLT:     rvsim_blt(cmd);       continue;
			case rvsim_BGE:     rvsim_bge(cmd);       continue;
			case rvsim_BLTU:    rvsim_bltu(cmd);      continue;
			case rvsim_BGEU:    rvsim_bgeu(cmd);      continue;
			case rvsim_LB:      rvsim_lb(cmd);        break;
			case rvsim_LH:      rvsim_lh(cmd);        break;
			case rvsim_LW:      rvsim_lw(cmd);        break;
			case rvsim_LBU:     rvsim_lbu(cmd);       break;
			case rvsim_LHU:     rvsim_lhu(cmd);       break;
			case rvsim_SB:      rvsim_sb(cmd);        break;
			case rvsim_SH:      rvsim_sh(cmd);        break;
			case rvsim_SW:      rvsim_sw(cmd);        break;
			case rvsim_ADDI:    rvsim_addi(cmd);      break;
			case rvsim_SLTI:    rvsim_slti(cmd);      break;
			case rvsim_SLTIU:   rvsim_sltiu(cmd);     break;
			case rvsim_XORI:    rvsim_xori(cmd);      break;
			case rvsim_ORI:     rvsim_ori(cmd);       break;
			case rvsim_ANDI:    rvsim_andi(cmd);      break;
			case rvsim_SLLI:    rvsim_slli(cmd);      break;
			case rvsim_SRLI:    rvsim_srli(cmd);      break;
			case rvsim_SRAI:    rvsim_srai(cmd);      break;
			case rvsim_ADD:     rvsim_add(cmd);       break;
			case rvsim_SUB:     rvsim_sub(cmd);       break;
			case rvsim_SLL:     rvsim_sll(cmd);       break;
			case rvsim_SLT:     rvsim_slt(cmd);       break;
			case rvsim_SLTU:    rvsim_sltu(cmd);      break;
			case rvsim_XOR:     rvsim_xor(cmd);       break;
			case rvsim_SRL:     rvsim_srl(cmd);       break;
			case rvsim_SRA:     rvsim_sra(cmd);       break;
			case rvsim_OR:      rvsim_or(cmd);        break;
			case rvsim_AND:     rvsim_and(cmd);       break;
			case rvsim_REMU:    rvsim_remu(cmd);      break;
			case rvsim_DIVU:    rvsim_divu(cmd);      break;
			case rvsim_DIV:     rvsim_div(cmd);       break;
			case rvsim_MUL:     rvsim_mul(cmd);       break;
			case rvsim_FENCE:   rvsim_fence(cmd);     break;            
        } // switch

		rvsim_PC += 4;
	} // while

     
}

static unsigned int rvsim_fetch(unsigned int pc) {
    char instr_bytes[4] ;  
    unsigned int * instr_word = (unsigned int *) instr_bytes ;
    unsigned int instr_base_addr = pc - rvsim_IMEM_BASE_ADDR ;
    instr_bytes[0] = rvsim_imem[instr_base_addr] ;
    instr_bytes[1] = rvsim_imem[instr_base_addr+1] ;
    instr_bytes[2] = rvsim_imem[instr_base_addr+2] ;
    instr_bytes[3] = rvsim_imem[instr_base_addr+3] ;    
	return (*instr_word) ; 
    
} // rvsim_exec_prog

static unsigned int rvsim_decode(unsigned int single_inst) {
    
    if (single_inst==0x0ff0000f) return rvsim_FENCE ; // riscv fence code

	unsigned int inst_type = rvsim_ILL ;

	switch (single_inst & 0x7f)
	{
		case 0x37: inst_type = rvsim_LUI; break;
		case 0x17: inst_type = rvsim_AUIPC; break;
		case 0x6f: inst_type = rvsim_JAL; break;
		case 0x67: inst_type = rvsim_JALR; break;
		case 0x63:
		{
			switch ((single_inst & 0x7000) >> 12)
			{
				case 0x0: inst_type = rvsim_BEQ;  break;
				case 0x1: inst_type = rvsim_BNE;  break;
				case 0x4: inst_type = rvsim_BLT;  break;
				case 0x5: inst_type = rvsim_BGE;  break;
				case 0x6: inst_type = rvsim_BLTU; break;
				case 0x7: inst_type = rvsim_BGEU; break;
			}
			break;
		}
		case 0x3:
		{
			switch ((single_inst & 0x7000) >> 12)
			{
				case 0x0: inst_type = rvsim_LB;  break;
				case 0x1: inst_type = rvsim_LH;  break;
				case 0x2: inst_type = rvsim_LW;  break;	
				case 0x4: inst_type = rvsim_LBU; break;
				case 0x5: inst_type = rvsim_LHU; break;
			}
			break;
		}
		case 0x23:
		{
			switch ((single_inst & 0x7000) >> 12)
			{
				case 0x0: inst_type = rvsim_SB; break;
				case 0x1: inst_type = rvsim_SH; break;
				case 0x2: inst_type = rvsim_SW; break;
			}
			break;
		}
		case 0x13:
		{
			switch ((single_inst & 0x7000) >> 12)
			{
				case 0x0: inst_type = rvsim_ADDI;  break;
				case 0x2: inst_type = rvsim_SLTI;  break;
				case 0x3: inst_type = rvsim_SLTIU; break;
				case 0x4: inst_type = rvsim_XORI;  break;
				case 0x6: inst_type = rvsim_ORI;   break;
				case 0x7: inst_type = rvsim_ANDI;  break;
				case 0x1: inst_type = rvsim_SLLI;  break;
				case 0x5: 
				{
					if (!(single_inst & 0x40000000))
						inst_type = rvsim_SRLI;
					else
						inst_type = rvsim_SRAI; 
					break;
				}
				default: inst_type = rvsim_ILL; break;
			}
			break;
		}
		case 0x33:
		{
			switch ((single_inst & 0x7000) >> 12)
			{
				case 0x0:	
				{
					switch( ((single_inst)>>25)&0x7f)
					{
						case 0x0: inst_type = rvsim_ADD;  break;
						case 0x1: inst_type = rvsim_MUL;  break;
						case 0x20: inst_type = rvsim_SUB; break;
						default: inst_type = rvsim_ILL;   break;
					}
					break;
				}
				case 0x1: 
				{
					switch( ((single_inst)>>25)&0x7f)
					{
						case 0x0: inst_type = rvsim_SLL; break;
						default: inst_type = rvsim_ILL;  break;
					}
					break;
				}
				case 0x2: 
				{
					switch( ((single_inst)>>25)&0x7f)
					{	
						case 0x0: inst_type = rvsim_SLT; break;
						default: inst_type = rvsim_ILL;  break;
					}
					break;
				}
				case 0x3: 
				{
					switch( ((single_inst)>>25)&0x7f)
					{
						case 0x0: inst_type = rvsim_SLTU; break;
						default: inst_type = rvsim_ILL;   break;
					}
					break;
				}
				case 0x4: 
				{
					switch( ((single_inst)>>25)&0x7f)
					{
						case 0x0: inst_type = rvsim_XOR; break;
						case 0x1: inst_type = rvsim_DIV; break;
						default: inst_type = rvsim_ILL;  break;
					}
					break;
				}
				case 0x5:
				{
					switch( ((single_inst)>>25)&0x7f)
					{
						case 0x0: inst_type = rvsim_SRL;  break;
						case 0x20: inst_type = rvsim_SRA; break;
						case 0x1: inst_type = rvsim_DIVU; break;
						default: inst_type = rvsim_ILL;   break;
					}
					break;
				}
				case 0x6: 
				{
					switch( ((single_inst)>>25)&0x7f)
					{
						case 0x0: inst_type = rvsim_OR; break;
						default: inst_type = rvsim_ILL; break;
					}
					break;
				}
				case 0x7:
				{
					switch( ((single_inst)>>25)&0x7f)
					{
						case 0x0: inst_type = rvsim_AND;  break;
						case 0x1: inst_type = rvsim_REMU; break;
						default:  inst_type = rvsim_ILL;   break;
					}
					break;
				}
				default: inst_type = rvsim_ILL; break;
			}
			break;	
		}
		default: inst_type = rvsim_ILL; break; 
	}
	
	return inst_type;
}


//-----------------------------------------------------------------------------------

void rvsim_print_reg_range(int base_reg , int num_regs){
	vl_bm_printf(3)("\n\nRVSIM Content of Register:\n\n");  
    for (int r = base_reg; r < (base_reg+num_regs) ; r++)
    {vl_bm_printf(3)("Reg[%2d] = 0x%08x\n", r, rvsim_reg[r]);}
}

//-----------------------------------------------------------------------------------


static void rvsim_print_dmem_range(void)
{
    int dmem_test_base_addr = rvsim_reg[LOAD_STORE_BASE_REG] + LOAD_STORE_BASE_OFFSET ; 
    unsigned int*  rvsim_dmem_words_array = (unsigned int *)(&rvsim_dmem[dmem_test_base_addr]) ; // Overlay words     
    
	vl_bm_printf(3)("\n\nRVSIM Content of Memory:\n\n");
	for(int i = 0; i <  NUM_TESTED_DMEM_WORDS ; i++) {
        vl_bm_printf(3)("M[%d] = 0x%08x\n",i,rvsim_dmem_words_array[i]) ;
    }
}

//-----------------------------------------------------------------------------------

void run_rvsim(int test_idx, unsigned int test_seed) {
 	   unsigned int* rvsim_imem_words_array;
	   rvsim_imem_words_array = (unsigned int *)(rvsim_imem) ; // Overlay words array
       
	   for (int i = 0; i < INSTR_ARRAY_SIZE; i++)
	   	    rvsim_imem_words_array[i] = instructions_array[i].riscv_code ; // Copy instructions to rvsim_imem
        
       rvsim_imem_words_array[INSTR_ARRAY_SIZE-1] = 0 ; // Change last instruction to 0 , to cause RVSIM to terminate.
        
       rvsim_null_regs_range(BASE_TESTED_REG, NUM_TESTED_REGS) ; 
       
       rvsim_init_dmem_range(LOAD_STORE_BASE_REG, LOAD_STORE_BASE_OFFSET, NUM_TESTED_DMEM_WORDS) ; 
       
	   bool_t rvsim_exec_status_ok = rvsim_exec_prog(DUT_IMEM_CODE_START_ADDR);
       if (!rvsim_exec_status_ok) {
           vl_bm_printf(3)("\n Test #%d with seed %08x FAIL on execution timeout, Quitting\n",test_idx,test_seed);
		   quit_selfie5();
       }
	   rvsim_print_reg_range(BASE_TESTED_REG, NUM_TESTED_REGS);
       rvsim_print_dmem_range();

       bool_t check_regs_status = check_regs(rvsim_reg,0,BASE_TESTED_REG,NUM_TESTED_REGS);

       if (!check_regs_status) {
         vl_bm_printf(3)("\n RVSIM: Test #%d with seed %08x FAIL on registers mismatch, Quitting\n",test_idx,test_seed);
       }  

       int  dmem_test_base_addr = rvsim_reg[LOAD_STORE_BASE_REG] + LOAD_STORE_BASE_OFFSET ; 

       bool_t check_dmem_status = check_dmem((unsigned int *)(&rvsim_dmem[dmem_test_base_addr]));
       if (!check_dmem_status) {
         vl_bm_printf(3)("\n RVSIM: Test #%d with seed %08x FAIL on DMEM mismatch, Quitting\n",test_idx,test_seed);         
       }  
       
  
       
       if (!(check_regs_status || check_dmem_status)) quit_selfie5() ;
       
       else vl_bm_printf(3)("\n RVSIM: Test #%d with seed %08x PASSED",test_idx,test_seed);
       
}
