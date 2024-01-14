
#ifndef _SELFIE5_COMMON_H_PARSED__

#include <leo_2_init.h>

typedef enum {False, True} bool_t;
#define MAX_STR_LEN          80     // printing strings length


#include "selfie5_flow.h" 
#include "selfie5_gcode.h"
#include "selfie5_riscv.h" 
#include "selfie5_main.h" 
#include "selfie5_dut.h" 
#include "selfie5_rvsim.h"
#include "selfie5_trace.h"

#ifdef __TEST_DUT__
#include "selfie5_dut.h"
#endif

#define _SELFIE5_COMMON_H_PARSED__
#endif

