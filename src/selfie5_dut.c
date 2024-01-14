


// Code for testing the actual Hardware DUT at an PULENIX/FPGENIX environment
 
// -------------------------------------------------------------------------------------------------------------------------

#include "selfie5_common.h"


unsigned int saved_regs[MAX_NUM_DUT_REGS] ;
unsigned int dut_regs[MAX_NUM_DUT_REGS] ;
unsigned int dmem_load_store_space[NUM_TESTED_DMEM_WORDS] ;
unsigned int dut_code_start_addr ;

#ifdef DUT_EXEC_FROM_DMEM
unsigned int dmem_code_block[INSTR_ARRAY_SIZE] ;
#endif

extern unsigned int  initial_registers_array[]; // This array maintains the randomly initialized content of the processor registers
extern unsigned int  initial_dmem_array[];      // This array maintains the randomly initialized content of the tested data-memory


// -------------------------------------------------------------------------------------------------------------------------

// Save context, Execute DUT code, Restore context

void __attribute__ ((noinline)) dut_save_execute_restore() {

// For now we support only DUT testing of x18-x27 (s2-s11) which by ABI are Callee Saved registers
// x18-x20 are used as 3 assisting pointers
// x21-x27,are the 7 available regs for testing.

  // save assistance pointers reg pointers 
  asm volatile ("addi sp,sp,-4"); 
  asm volatile ("sw x18 ,  0(sp)"); 
  asm volatile ("addi sp,sp,-4");    
  asm volatile ("sw x19 ,  0(sp)");
  asm volatile ("addi sp,sp,-4");    
  asm volatile ("sw x20 ,  0(sp)");
  
  // save return address
  asm volatile ("addi sp,sp,-4"); 
  asm volatile ("sw x1 ,0(sp)");
   
  // set regs-saving array-pointer
  asm volatile ("la x18 , saved_regs");
    
  // save dut regs
  asm volatile ("sw x21 , 0(x18)"); 
  asm volatile ("sw x22 , 4(x18)"); 
  asm volatile ("sw x23 , 8(x18)"); 
  asm volatile ("sw x24 ,12(x18)"); 
  asm volatile ("sw x25 ,16(x18)"); 
  asm volatile ("sw x26 ,20(x18)"); 
  asm volatile ("sw x27 ,24(x18)"); 
                         
  // init dut regs
  asm volatile ("la x19 , initial_registers_array");
  asm volatile ("lw x21 , 21*4(x19)");  
  asm volatile ("lw x22 , 22*4(x19)");  
  asm volatile ("lw x23 , 23*4(x19)");  
  asm volatile ("lw x24 , 24*4(x19)");  
  asm volatile ("lw x25 , 25*4(x19)");  
  asm volatile ("lw x26 , 26*4(x19)");  
  asm volatile ("lw x27 , 27*4(x19)");

  // set DUT execution function pointer
  asm volatile ("la x19 , dut_code_start_addr");
  asm volatile ("lw x19 , 0(x19)");  

  // set load-store pointer
  asm volatile ("la x20 , dmem_load_store_space");
 
 // call execution function
  asm volatile ("jalr x19");
  
  // set dut_regs copy-array pointer
  asm volatile ("la x19 , dut_regs");
       
  // copy registers to dut_regs array    
  asm volatile ("sw x21 ,  0(x19)"); 
  asm volatile ("sw x22 ,  4(x19)"); 
  asm volatile ("sw x23 ,  8(x19)"); 
  asm volatile ("sw x24 , 12(x19)"); 
  asm volatile ("sw x25 , 16(x19)"); 
  asm volatile ("sw x26 , 20(x19)"); 
  asm volatile ("sw x27 , 24(x19)"); 
                         
  // restore regs  
  asm volatile ("lw x21 ,  0(x18)"); 
  asm volatile ("lw x22 ,  4(x18)"); 
  asm volatile ("lw x23 ,  8(x18)"); 
  asm volatile ("lw x24 , 12(x18)"); 
  asm volatile ("lw x25 , 16(x18)");     
  asm volatile ("lw x26 , 20(x18)");     
  asm volatile ("lw x27 , 24(x18)");
                         
  // restore return address
  asm volatile ("lw x1 ,0(sp)");  
  asm volatile ("addi sp,sp,4"); 
      
  // restore assistance pointers reg pointers
  asm volatile ("lw x20 ,  0(sp)"); 
  asm volatile ("addi sp,sp,4");    
  asm volatile ("lw x19 ,  0(sp)"); 
  asm volatile ("addi sp,sp,4");    
  asm volatile ("lw x18 ,  0(sp)"); 
  asm volatile ("addi sp,sp,4");    
}    

//--------------------------------------------------------------------------------------------------------------------------

// Copy instructions to DUT code space
void dut_load_test_code(unsigned int dut_code_start_addr, selfie5_instr_t  instructions_array[] ,int num_dut_test_instr) {
    
  for (int i = 0; i < num_dut_test_instr; i++) {
      
         unsigned int addr =  dut_code_start_addr + i*4 ;   
         *(volatile unsigned int*) addr = instructions_array[i].riscv_code ;
  }
 
  *(volatile unsigned int*)  (dut_code_start_addr+((num_dut_test_instr-1)*4)) = 0x00008067 ; // change last instruction to return
}
//-----------------------------------------------------------------------------------------------------------------------

void init_load_store_dmem_space(volatile unsigned int * dmem_load_store_space, int num_words) {
    
  for (int i = 0; i < num_words; i++) {      
    dmem_load_store_space[i] = initial_dmem_array[i] ; 
  }
}

//-----------------------------------------------------------------------------------------------------------------------

//typedef void void_func_t(void); // General void-in void-out function type

void test_dut(int test_idx, unsigned int test_seed, selfie5_instr_t  instructions_array[]) {

#ifndef DUT_EXEC_FROM_DMEM
  dut_code_start_addr = DUT_IMEM_CODE_START_ADDR ;    
#else
  dut_code_start_addr = (unsigned int) dmem_code_block ;
#endif
 
  dut_load_test_code(dut_code_start_addr , instructions_array, INSTR_ARRAY_SIZE) ;
    
  init_load_store_dmem_space(dmem_load_store_space,NUM_TESTED_DMEM_WORDS) ;
    
  dut_save_execute_restore() ; // Save context, Execute DUT code, Restore context
  
  bool_t check_regs_status = check_regs(dut_regs, BASE_TESTED_REG, BASE_TESTED_REG, NUM_TESTED_REGS);

  bool_t check_dmem_status = check_dmem(dmem_load_store_space);
  
  if ((check_dmem_status==False)||(check_regs_status==False)) {
    vl_bm_printf(0)("\n DUT: Test #%d with seed %08x FAIL on REGS and/or DMEM mismatch, Quitting\n",test_idx,test_seed);
    quit_selfie5() ;
  }  



  // TODO: Check timeout  
}

 
// -------------------------------------------------------------------------------------------------------------------------

