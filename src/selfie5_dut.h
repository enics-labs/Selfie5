
#ifndef _SELFIE5_DUT_H_PARSED__

//extern unsigned int _instrram_top ; // _instrram_top defined by link.riscv.ld / link.common.ld (Currently not used)//

// #define DUT_EXEC_FROM_DMEM // will execute the test code from data memory over AXI , rather than instr memory TCM

#ifdef PULPENIX2_LITE_DUT
    #define DUT_IMEM_CODE_START_ADDR  (0x8000-(4*INSTR_ARRAY_SIZE))
#else
   #ifdef IMEM_HIGH
     #define DUT_IMEM_CODE_START_ADDR  (MAX_XCORE_IMEM +    0x1 - (4*INSTR_ARRAY_SIZE))
   #else
     #define DUT_IMEM_CODE_START_ADDR  (MIN_XCORE_IMEM + 0x8000 - (4*INSTR_ARRAY_SIZE))
   #endif
#endif

void test_dut(int test_idx, unsigned int test_seed, selfie5_instr_t  instructions_array[]) ;

#define _SELFIE5_DUT_H_PARSED__ 
#endif
