
#ifndef _SELFIE5_MAIN_H_PARSED__

bool_t check_regs(unsigned int * dut_regs, int dut_reg_array_base_idx, int tested_base_reg_idx, int num_tested_regs) ;


bool_t check_dmem(unsigned int * dut_dmem) ;

int quit_selfie5(void) ;

int pos_lfsr_rand_get_next(void) ;


//------------------------------------------------------------------------------------------------

// Used for all stages 

typedef struct selfie5_instr {
    
	selfie5_flow_cmd_t  flow_type;    
	selfie5_gcode_cmd_t gcode_type;
	int offset;
	int RS1; // Index of Source reg 1
	int RS2; // Index of Source reg 2    
	int RD;  // Index of Destination reg

	int MD_entry_idx ;  
       
	unsigned int RS1_exec_val;
	unsigned int RS2_exec_val;
	unsigned int RD_exec_val;  
	bool_t       is_taken;     
    unsigned int riscv_code ;
    
    // Following used mostly for trace printing
    riscv_instr_type_t riscv_instr_type ;
    opc_t              riscv_opc ;
    int                riscv_imm ;    
    funct3_t           riscv_f3 ;
          
}  selfie5_instr_t;


#define _SELFIE5_MAIN_H_PARSED__
#endif