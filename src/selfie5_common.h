
#ifndef _SELFIE5_COMMON_H_PARSED__

#ifdef __LEO2_BOARD__
#include <leo_2_init.h>
#endif 

#ifndef __SELFIE5_DEV__
extern unsigned int dmem_load_store_space[] ;
#endif

#include "selfie5_defines.h"
#include "selfie5_flow.h" 
#include "selfie5_gcode.h"
#include "selfie5_riscv.h" 
#include "selfie5_main.h" 
#include "selfie5_rvsim.h"
#include "selfie5_trace.h"

#ifdef __TEST_DUT__
#include "selfie5_dut.h"
#endif

#define _SELFIE5_COMMON_H_PARSED__
#endif

