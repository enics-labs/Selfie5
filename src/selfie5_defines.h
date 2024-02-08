

#ifndef _SELFIE5_DEFINES_H_PARSED__

// Compilation setting

//#define PULPENIX2_LITE_DUT

#define __SELFIE5_DEV__ // Visual development environment , physical DUT absent

//#define __DDP_PULPENIX__  // Old Pulpenix (DDP 2020 version , below defines set in windows invocation script)


//#define _DISABLE_RVSIM__

#ifndef __SELFIE5_DEV__
#define __TEST_DUT__
#endif

// #define __DDP_PULPENIX__
// #define __PULPENIX_BM__
// #define __LEO2_BOARD__
// #define IMEM_HIGH

// #define CHECK_PERFORMANCE 1

//========================================

#ifdef __SELFIE5_DEV__
#include <stdio.h>
#include <stdlib.h>
#endif



#ifdef __LEO2_BOARD__
#define NUM_TESTS_LIMIT  NUM_TEST_ITR // provided by leo2_clx -itr (default is 1)
#else 
#define NUM_TESTS_LIMIT  10  // Number of tests 
#endif

#define MAX_NUM          8  // This parameter determines the maximum literal number the assigned registers
                            // Smaller , means higher probability for branch on  equal to take place
                            // However this number should not exist the applied ISA max immediate value.
                             
#define INSTR_ARRAY_SIZE     32

#define BASE_TESTED_REG      21     // Actual Index of the base tested register - ABI SENSATIVE DO NOT MODIFY
#define MAX_NUM_DUT_REGS      7     // Total number of potential tested registers - ABI SENSATIVE DO NOT MODIFY
#define NUM_TESTED_REGS       4     // Maximum is MAX_NUM_DUT_REGS
#define NUM_TESTED_DMEM_WORDS 4     // Number of tested data-memory words

#define EXEC_INSTR_COUNT_LIMIT (INSTR_ARRAY_SIZE+10) // Assuming each instruction is executes just once, allowing few nore for failure debug assistance.

// #define DUT_EXEC_FROM_DMEM // will execute the test code from data memory over AXI , rather than instr memory TCM

#ifdef __SELFIE5_DEV__
#define MIN_XCORE_IMEM 0
#define dmem_load_store_space 0x10000000 
#endif



#ifdef PULPENIX2_LITE_DUT
    #define DUT_IMEM_CODE_START_ADDR  (0x8000-(4*INSTR_ARRAY_SIZE))
#else
   #ifdef IMEM_HIGH
     #define DUT_IMEM_CODE_START_ADDR  (MAX_XCORE_IMEM +    0x1 - (4*INSTR_ARRAY_SIZE))
   #else
     #define DUT_IMEM_CODE_START_ADDR  (MIN_XCORE_IMEM + 0x8000 - (4*INSTR_ARRAY_SIZE))
   #endif
#endif

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

//#ifdef __LEO2_BOARD__
#define vl_bm_printf(verbosity_level) if (verbosity_level <= VERBOSITY_LEVEL) bm_printf 
//#endif 

#ifndef __PULPENIX_BM__ // Pulpenix Bare-Metal mode / support stand-alone OS equipped run Windows/Linux.
#ifdef __SELFIE5_DEV__
   #define bm_printf printf 
#endif   
#endif



#define INITIAL_SEED 0x2a209b7d



#define _SELFIE5_DEFINES_H_PARSED__
#endif