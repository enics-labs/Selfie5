

#include "selfie5_common.h" 


#ifdef __TEST_DUT__
  #include "selfie5_dut.h"
  #ifdef DUT_EXEC_FROM_DMEM
    extern unsigned int dmem_code_block[] ;
  #endif
#endif


#ifdef INTERRUPTS
#include <int.h>
#include <timer.h>
#define EVENT_ENABLE         (EVENT_UNIT_BASE_ADDR+0x0)
#define EVENT_CLEAR_PENDING	 (EVENT_UNIT_BASE_ADDR+0xc)
volatile unsigned int timer_cnt = 0;
void ISR_TA_CMP (void) { 
    //set next timer
    timer_set_cmp(get_time_long()+(1000));

    //clear event
    wr32(EVENT_CLEAR_PENDING, 1<<7); 

    //increment counter
    timer_cnt++; 
} 
#endif

//----------------------------------------------------------------------------------------------------------------------

extern void print_instruction_code_gcode (void);
extern void print_instruction_code_flow  (void);
extern void print_gcode_execution_trace  (void);
extern void print_flow_execution_trace   (void);
extern void create_gcode                 (void);
extern void gen_riscv_code               (void);

//----------------------------------------------------------------------------------------------------------------------

//  LFSR pseudo random number generator 


unsigned int lfsr_reg = INITIAL_SEED ;  // Global LFSR register , set to default seed.

// Set to seed value for the pseudo random series

void lfsr_rand_set_seed(unsigned int seed) {
    lfsr_reg = seed; // Simply load ther global lfsr register
}

// Assistance function for extracting a cerain bit value out of an unsined int
char get_bit(unsigned int x, unsigned int idx) {return (x >> idx) & 1 ; } // shift to least significant bit and mask out

// Get the next pseudo random value in the series

unsigned int lfsr_rand_get_next(void) {
    unsigned int x = lfsr_reg;
    unsigned int y = (x << 1) | (get_bit(x, 31) ^ get_bit(x, 29) ^ get_bit(x, 25) ^ get_bit(x, 24));
    y = y & 0xffffffff; // keep it 32 bits.
    lfsr_reg = y;
    return x;
}

int pos_lfsr_rand_get_next(void) {
   unsigned int rand = lfsr_rand_get_next() ;
   if (((int)(rand)) < 0) return -((int)(rand)) ;
   else return (int)(rand) ;
}

//====================================================================================================================


selfie5_instr_t instructions_array[INSTR_ARRAY_SIZE];        // This array represents the instructions
char     is_address_available[INSTR_ARRAY_SIZE];             // This array holds a list of all addresses yet to be determined

unsigned int    registers_array[32];                         // This array maintains the reference content of the processor registers
unsigned int    dmem_array[NUM_TESTED_DMEM_WORDS];           // This array maintains the reference content of the tested data-memory

volatile unsigned int initial_registers_array[32];                // This array maintains the randomly initialized content of the processor registers
volatile unsigned int initial_dmem_array[NUM_TESTED_DMEM_WORDS];  // This array maintains the randomly initialized content of the tested data-memory

extern bool_t rvsim_exec_prog(unsigned int entry_addr) ;

// NOTICE , flow_type_str must be in EXACT order as in selfie5_flow_cmd_t !!!
char flow_types_str[NUM_FLOW_CMD][MAX_STR_LEN] = {"GOTO","STEP","FLOW_FINISH"}; // Just for prints
// NOTICE , gcode_types_str must be in EXACT order as in selfie5_gcode_cmd_t !!!


//-----------------------------------------------------------------------------------------------------------------------

void init_selfie5(unsigned int test_seed) {


	vl_bm_printf(4)("Initializing Selfi5 data structures with seed %08x\n\n",test_seed);
    
    lfsr_rand_set_seed(test_seed) ; // Initialize random number seed.
    
	for (int i = 0; i < INSTR_ARRAY_SIZE ; i++) is_address_available[i] = True; // True indicates address is still available			  


	for (int i = BASE_TESTED_REG ; i < (BASE_TESTED_REG+NUM_TESTED_REGS) ; i++)  {
          initial_registers_array[i] = lfsr_rand_get_next() % MAX_NUM ;    //Initialize all registers to memorized random values
          registers_array[i] = initial_registers_array[i] ;
          vl_bm_printf(4)("R[%-2d]=%08x\n",i,initial_registers_array[i]);
    }
    

    registers_array[LOAD_STORE_BASE_REG] = (int) dmem_load_store_space ; 
 
    
    vl_bm_printf(4)("R[%-2d]=%08x # dmem load/store space pointer\n\n",
                     LOAD_STORE_BASE_REG,registers_array[LOAD_STORE_BASE_REG]);    
        
	for (int i = 0 ; i < NUM_TESTED_DMEM_WORDS ; i++)  {
        
        initial_dmem_array[i] = lfsr_rand_get_next() % MAX_NUM ;   //Initialize tested memory to memorized random values
        dmem_array[i] = initial_dmem_array[i] ;
        vl_bm_printf(4)("M[%d]=%08x\n",i,initial_dmem_array[i]);
    } 
    vl_bm_printf(4)("\n");
}


void print_expected_registers() {
	for (int r = BASE_TESTED_REG ; r < BASE_TESTED_REG+NUM_TESTED_REGS; r++) { vl_bm_printf(3)(" R%-3d = %-4d (0x%08x)\n" , r, registers_array[r] , registers_array[r]); }
}

void print_expected_dmem_load_store_space() {
	for (int i = 0 ; i < NUM_TESTED_DMEM_WORDS; i++)  { vl_bm_printf(3)(" M%-3d = %-4d (0x%08x)\n" , i, dmem_array[i] , dmem_array[i]); }
}


// ----------------------------------------------------------------------------------------------------------------------

bool_t check_regs(unsigned int * dut_regs, int dut_reg_array_base_idx, int tested_base_reg_idx, int num_tested_regs) {
   
   int retVal = True ;   
   vl_bm_printf(4)("\nComparing DUT applied registers values with selfie5 expected :\n"); 
   for (int r = tested_base_reg_idx ; r < tested_base_reg_idx+num_tested_regs; r++)  {
       vl_bm_printf(4)("DUT R[%d] = 0x%08x\n", r, dut_regs[r-dut_reg_array_base_idx]);
	   if (registers_array[r] != dut_regs[r-dut_reg_array_base_idx]) {
		   vl_bm_printf(0)("FAIL on reg[%d] , dut = 0x%08x , expected = 0x%08x\n", r, dut_regs[r] ,registers_array[r]);
		   retVal = False;
	   }
   }
   if (retVal==True) vl_bm_printf(4)("PASS\n");
   return retVal ;
}    

// ----------------------------------------------------------------------------------------------------------------------


bool_t check_dmem(unsigned int * dut_dmem) {
   int retVal = True ;      
   vl_bm_printf(4)("\nComparing DUT applied data-memory values with selfie5 expected :\n"); 
   for (int di = 0 ; di < NUM_TESTED_DMEM_WORDS; di++)  {
       vl_bm_printf(4)("M[%d] = 0x%08x\n", di, dut_dmem[di]);
	   if (dmem_array[di] != dut_dmem[di]) {
		   vl_bm_printf(0)("FAIL on dmem[%d] , dut = 0x%08x , expected = 0x%08x\n", di, dut_dmem[di] ,dmem_array[di]);
		   retVal = False;
	   }
   }
   if (retVal==True) vl_bm_printf(4)("PASS\n");
   return retVal ;
}    

// ----------------------------------------------------------------------------------------------------------------------


#ifndef _DISABLE_RVSIM__   // Externals for rvsim to simulate machine code execution.
	extern char rvsim_imem[];
    extern unsigned int rvsim_reg[]; // For comparison
    extern unsigned int rvsim_dmem[]; // For comparison    
    extern void run_rvsim(int test_idx, unsigned int test_seed);
#endif

// -------------------------------------------------------------------------------------------------------------------------

int quit_selfie5(void) {
#ifdef __PULPENIX_BM__ 
   #if defined(__DDP_PULPENIX__) || defined(PULPENIX2_LITE_DUT) || defined(__LEO2_BOARD__)
      #if defined(__LEO2_BOARD__)
      bm_quit_app();  // uart message to simulation/pyshell to quit
      return 0;
      #else
      bm_printf("$pyshell quit()\n") ;  // uart message to pyshell to quit (supported also by simulation tb)
      #endif
   #else
      sim_finish();
   #endif
#else
   exit(0) ;
#endif    
}

//---------------------------------------------------------------------------------------------------------------------------
 
int main() {
    
#ifdef __LEO2_BOARD__    
   leo_2_init(); 
#endif           

   #ifdef INTERRUPTS
      //set interrupt enable
      bm_printf("enable interrupts...");
      int_enable();
      bm_printf(" done\n");

      bm_printf("config timer...");
      timer_set_prescaler(0);
      set_timer(0x0123456789abcdefULL);
      timer_set_cmp(0x0123456789ac0000ULL);
      start_timer();
      bm_printf(" done\n");

      bm_printf("config event unit...");
      wr32(EVENT_ENABLE, 1<<7); //enable timer interrupt
      bm_printf(" done\n");
   #endif

 	bm_printf("\nHello Selfie5\n");   


      
    lfsr_rand_set_seed(INITIAL_SEED) ; // Initialize random number seed.

    #ifdef __TEST_DUT__
       #if CHECK_PERFORMANCE
         int start_cycle = 0 ;
         int end_cycle   = 0 ;
               
         int init_cyc_count  = 0 ;
         int flow_cyc_count  = 0 ;
         int gcode_cyc_count = 0 ;    
         int riscv_cyc_count = 0 ;
         int dut_cyc_count   = 0 ; 
           
         ENABLE_CYCLE_COUNT  ; // Enable the cycle counter
         RESET_CYCLE_COUNT  ;  // Reset counter to ensure 32 bit counter does not wrap in-between start and end. 
       #endif
                            
       #ifndef DUT_EXEC_FROM_DMEM
         bm_printf("\nDUT test code will execute from IMEM at address %08x\n\n",DUT_IMEM_CODE_START_ADDR); 
       #else
         bm_printf("\nDUT test code will execute from DMEM at address %08x\n\n",dmem_code_block);
       #endif
       bm_printf("\nDUT LOAD-STORE dmem space located at address %08x\n\n",dmem_load_store_space);    
    #endif
       
               
    unsigned int test_seed  ;
    
    for (int test_idx=0;test_idx<NUM_TESTS_LIMIT;test_idx++) { 

       #if CHECK_PERFORMANCE
         RESET_CYCLE_COUNT  ;   
         GET_CYCLE_COUNT_START(start_cycle) ;
       #endif


       if (test_idx==0) test_seed = 0x90fabdff ;  // Change this to force a specific SEED on the first test 
                                                  // for reproducing and debugging a specific failing test with known seed
       else test_seed = lfsr_rand_get_next() ;    
   
 
       vl_bm_printf(4)("STARTING TEST %d , seed = %08x\n",test_idx,test_seed) ;


 
       // Test Generation 
	   init_selfie5(test_seed);

       #if CHECK_PERFORMANCE
         GET_CYCLE_COUNT_END(end_cycle) ;
         init_cyc_count += (end_cycle-start_cycle) ;
         RESET_CYCLE_COUNT  ;   
         GET_CYCLE_COUNT_START(start_cycle) ;         
       #endif

       
	   create_flow(); 
       
       #if CHECK_PERFORMANCE
         GET_CYCLE_COUNT_END(end_cycle) ;
         flow_cyc_count += (end_cycle-start_cycle) ;
         RESET_CYCLE_COUNT  ;   
         GET_CYCLE_COUNT_START(start_cycle) ;         
       #endif
       
	   create_gcode();
       
       #if CHECK_PERFORMANCE
         GET_CYCLE_COUNT_END(end_cycle) ;
         gcode_cyc_count += (end_cycle-start_cycle) ;
         RESET_CYCLE_COUNT  ;   
         GET_CYCLE_COUNT_START(start_cycle) ;         
       #endif
       
       
       gen_riscv_code() ;
       
       #if CHECK_PERFORMANCE
         GET_CYCLE_COUNT_END(end_cycle) ;
         riscv_cyc_count += (end_cycle-start_cycle) ;
         RESET_CYCLE_COUNT  ;   
         GET_CYCLE_COUNT_START(start_cycle) ;         
       #endif


       // Prints subject to verbosity setting

       if (VERBOSITY_LEVEL>3) {
 
       // Basic coverage selective printing hook
       #define PRINT_CVRG_TH_PRCNT 0      
       int gcode_cover_prcnt = check_gcode_coverage() ;
       if (gcode_cover_prcnt>PRINT_CVRG_TH_PRCNT) { ; 
         bm_printf("\nTest %d Basic GCODE Coverage = %d%%\n",test_idx,gcode_cover_prcnt) ;
         print_full_trace() ;
       }
          
#if 0
          bm_printf("\n\n INSTRUCTION ARRAY AFTER FLOW CODE\n");
          print_instruction_code_flow() ;

          bm_printf("\n\n FLOW EXECUTION TRACE\n");
          print_flow_execution_trace();              ;
              
          bm_printf("\n\n INSTRUCTION ARRAY AFTER GCODE STAGE\n");
          print_instruction_code_gcode() ;
           
          bm_printf("\n\n EXECUTION TRACE: STAGE GCODE\n");
          print_gcode_execution_trace();
                         
          bm_printf("\n\n SELFIE5 REGISTERS EXPECTED CONTENT AT END OF GCODE TRACING\n");
          print_expected_registers();
          bm_printf("\n\n SELFIE5 DMEM LOAD STORE EXPECTED SPACE AT END OF GCODE TRACING\n");
          print_expected_dmem_load_store_space();
#endif
       }

       #ifndef _DISABLE_RVSIM__   // Call rvsim to simulate machine code execution.
        vl_bm_printf(3)("\nStarting RVSIM Test #%d for seed %08x\n",test_idx,test_seed) ; 
       run_rvsim(test_idx,test_seed) ;
       #endif
       
       #ifdef __TEST_DUT__       

	     test_dut(test_idx, test_seed, instructions_array);
         #if CHECK_PERFORMANCE
           GET_CYCLE_COUNT_END(end_cycle) ;
           dut_cyc_count += (end_cycle-start_cycle) ;
           RESET_CYCLE_COUNT  ;   
           GET_CYCLE_COUNT_START(start_cycle) ;         
         #endif


         #ifndef __LEO2_BOARD__        
         {vl_bm_printf(2)("PASSED DUT Test #%d for seed %08x\n",test_idx,test_seed) ;}
         #else
         {vl_bm_printf(4)("PASSED DUT Test #%d for seed %08x\n",test_idx,test_seed) ;}
         #endif
         
       #endif
              
       vl_bm_printf(4)("\n\nEND OF TEST %d\n",test_idx) ;
       if ((test_idx != 0) && ((test_idx % LIVE_SIGN_PRINT_COUNT) == 0)) {
            vl_bm_printf(3)("%d Tests passed ...\n", test_idx);
       }         
    } // for
    

    #ifdef __TEST_DUT__
    #endif
  
    bm_printf("\n\nSELFIE5 EXECUTION SUMMARY:\n\n") ;
    bm_printf("GREAT!!! You have successfully passed %d randomly generated tests.\n",NUM_TESTS_LIMIT) ;
    bm_printf("Each test is composed of %d randomly self generated instructions including branches.\n\n",INSTR_ARRAY_SIZE) ;
   
    #ifdef __TEST_DUT__
    #if CHECK_PERFORMANCE
        int total_cyc_count = flow_cyc_count+gcode_cyc_count+riscv_cyc_count+dut_cyc_count ;        
        int cycles_per_test = total_cyc_count/NUM_TESTS_LIMIT ;
        int system_clk_mhz = 1000/ATSPEED_CLK_PERIOD_NS ;
        int tests_per_second = (system_clk_mhz*1000000)/cycles_per_test  ;
        
        if (NUM_TESTS_LIMIT<=1000) {
          bm_printf("Total cycle count including generation and validation : %d\n",total_cyc_count) ;
          bm_printf("Total cycle count per test including generation and validation : %d\n",cycles_per_test) ;  
          bm_printf("Average tests per elapse second at system clock of %dMHz : %d\n\n",system_clk_mhz,tests_per_second) ; 
          bm_printf("Total init  cycles : %d\n",init_cyc_count) ;  
          bm_printf("Total flow  cycles : %d\n",flow_cyc_count) ;  
          bm_printf("Total gcode cycles : %d\n",gcode_cyc_count) ;  
          bm_printf("Total riscv cycles : %d\n",riscv_cyc_count) ;
          bm_printf("Total dut   cycles : %d\n",dut_cyc_count) ;                   
      }
    #endif
    
    
    #endif
  
   
   quit_selfie5() ; 
   return 0 ;

}

