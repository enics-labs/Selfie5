
#ifndef _SELFIE5_MAIN_H_PARSED__

// Compilation setting

//#define PULPENIX2_LITE_DUT

//#define __SELFIE5_VS__ // Visual Stuio development environment

//#define __DDP_PULPENIX__  // Old Pulpenix (DDP 2020 version , below defines set in windows invocation script)


#define _DISABLE_RVSIM__
#define __TEST_DUT__

#define __DDP_PULPENIX__
#define __PULPENIX_BM__
#define __LEO2_BOARD__
#define IMEM_HIGH

#define CHECK_PERFORMANCE 1

//========================================

#define NUM_TESTS_LIMIT  NUM_TEST_ITR // provided by leo2_clx -itr (default is 1)

#define MAX_NUM          8  // This parameter determines the maximum literal number the assigned registers
                            // Smaller , means higher probability for branch on  equal to take place
                            // However this number should not exist the applied ISA max immediate value.
                             
#define INSTR_ARRAY_SIZE     32

#define BASE_TESTED_REG      21     // Actual Index of the base tested register - ABI SENSATIVE DO NOT MODIFY
#define MAX_NUM_DUT_REGS      7     // Total number of potential tested registers - ABI SENSATIVE DO NOT MODIFY
#define NUM_TESTED_REGS       4     // Maximum is MAX_NUM_DUT_REGS
#define NUM_TESTED_DMEM_WORDS 4     // Number of tested data-memory words

#define EXEC_INSTR_COUNT_LIMIT (INSTR_ARRAY_SIZE+10) // Assuming each instruction is executes just once, allowing few nore for failure debug assistance.

#define INITIAL_SEED 0x2a209b7d

bool_t check_regs(unsigned int * dut_regs, int dut_reg_array_base_idx, int tested_base_reg_idx, int num_tested_regs) ;


bool_t check_dmem(unsigned int * dut_dmem) ;

int quit_selfie5(void) ;

int pos_lfsr_rand_get_next(void) ;

#if GP_DEF == NA
 #define VERBOSITY_LEVEL 3    // Higher VERBOSITY_LEVEL means more prints
#else
 #define VERBOSITY_LEVEL GP_DEF  
#endif


#ifdef __LEO2_BOARD__
#define LIVE_SIGN_PRINT_COUNT 100000
#else
#define LIVE_SIGN_PRINT_COUNT 10      // Print a life sign once every number of tests
#endif

#define vl_bm_printf(verbosity_level) if (verbosity_level <= VERBOSITY_LEVEL) bm_printf 

#ifndef __PULPENIX_BM__ // Pulpenix Bare-Metal mode / support stand-alone OS equipped run Windows/Linux.
   #define bm_printf printf 
#endif

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