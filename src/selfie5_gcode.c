

#include "selfie5_common.h"

//====================================================================================================================

extern selfie5_instr_t instructions_array[INSTR_ARRAY_SIZE];        // This array represents the 32 instructions
extern char            is_address_available[INSTR_ARRAY_SIZE];      // This array holds a list of all addresses yet to be determined
extern unsigned int    registers_array[32];                         // This array maintains the content of the processor registers
extern unsigned int    dmem_array[NUM_TESTED_DMEM_WORDS];           // This array maintains the content of the tested data-memory

// ----------------------------------------------------------------------------------------------------------------------

// SERVICE FUNCTION : CREATE BRANCH INSTRUCTION ACCORDING TO TYPE

	void get_gcode_branch(int gpc, bool_t branch_is_taken, int offset) {
        
		int RS1,RS2;

		selfie5_instr_t* branch_instruction = &instructions_array[gpc];

		RS1 = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ;            // randomly select register RS1
		RS2 = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ;            // randomly select register RS2

		branch_instruction->offset   = offset;
        
        branch_instruction->RS1      = RS1 ;
        branch_instruction->RS2      = RS2 ;
        branch_instruction->is_taken = branch_is_taken ;

        bool_t reg_vals_are_eql = (registers_array[RS1]==registers_array[RS2]) ? True : False ; 

        // memorize value at execution time, currently each instruction is executed only once.        
        branch_instruction->RS1_exec_val   = registers_array[RS1] ;        
        branch_instruction->RS2_exec_val   = registers_array[RS2] ;  
        
        branch_instruction->gcode_type = branch_is_taken ? (reg_vals_are_eql ? JEQ : JNE) 
                                                         : (reg_vals_are_eql ? JNE : JEQ) ;
	}

// ----------------------------------------------------------------------------------------------------------------------

 void create_and_execute_JMP(int gpc) {             

   // Short JMP no link  
   selfie5_instr_t*  JMP_instruction = &instructions_array[gpc];
   JMP_instruction->gcode_type = JMP;	// If we keep as JMP, all fields of instruction will stay the same, and only the type name is changed to JMP
   JMP_instruction->RD = 0 ;// NO link
   JMP_instruction->RD_exec_val = 0 ;    
   
 }

//---------------------------------------------------------------------------------------------------------------------------                   

 void create_and_execute_GJAL(int gpc) {             

   selfie5_instr_t*  GJAL_instruction = &instructions_array[gpc];
   GJAL_instruction->gcode_type = GJAL;	  // Also links a register with dut-specific absolute return address.   
   int RD = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ;
   GJAL_instruction->RD = RD ;
   unsigned int dut_return_addr = DUT_IMEM_CODE_START_ADDR + (4*(gpc+1)) ; // TODO handle compressed offset once supported by Selfie5 

   registers_array[RD] = dut_return_addr ;     //  Execute link assignment
   // other fields of instruction will stay the same as in flow stage

   // memorize value at execution time, currently each instruction is executed only once.   
   GJAL_instruction->RD_exec_val = dut_return_addr ;    
   
 }

// ------------------------------------------------------------------------------------------------------------------------

 void create_and_execute_GJALR(int gpc) {             

   selfie5_instr_t*  GJALR_instruction = &instructions_array[gpc];
   

   // Find valid RS1 for offset base, limited to signed 12bit from jump destination address
   int jump_dest_addr =  DUT_IMEM_CODE_START_ADDR + (4*(gpc+GJALR_instruction->offset)) ; // TODO handle compressed offset once supported by Selfie5 
   int rs1_cand = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ; // check all registers as potential base stating from a random register
   int diff = 0 ;
   bool_t rs1_found = False ;
   for (int i=0;i<NUM_TESTED_REGS;i++) {
     diff = jump_dest_addr - ((int)(registers_array[rs1_cand])) ;     
     if (((unsigned int)diff) <  (1<<12)) {
       rs1_found = True ;
       break ;
     }
     rs1_cand = (rs1_cand+1)%NUM_TESTED_REGS ;    
   } // for
   
   if (rs1_found) { 
       
      GJALR_instruction->gcode_type = GJALR;	  // Also links a register with dut-specific absolute return address.   
      int RD = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ;
      GJALR_instruction->RD = RD ;
      // memorize value at execution time, currently each instruction is executed only once.   
      GJALR_instruction->RS1 = rs1_cand;
      GJALR_instruction->RS1_exec_val = registers_array[rs1_cand]; 
      unsigned int dut_return_addr = DUT_IMEM_CODE_START_ADDR + (4*(gpc+1)) ; // TODO handle compressed offset once supported by Selfie5 
      registers_array[RD] = dut_return_addr ;     
      GJALR_instruction->RD_exec_val = dut_return_addr ;                           
   }
   

   else {
      get_gcode_branch(gpc,True,GJALR_instruction->offset);
   }
 }

// ------------------------------------------------------------------------------------------------------------------------

	void create_and_execute_LIMM(int gpc) {
		int RD;
        unsigned int num;
               
		RD = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ;
		num = pos_lfsr_rand_get_next() % MAX_NUM ;
        
        selfie5_instr_t* LIMM_instruction = &instructions_array[gpc];
                
        // Create instruction
        
		LIMM_instruction->gcode_type = LIMM ;     // Equivalent to instructions_array[gpc].type = LIMM ;
        LIMM_instruction->offset = 1 ;
        LIMM_instruction->RD = RD ;
        
        // memorize value at execution time, currently each instruction is executed only once.               
        LIMM_instruction->RD_exec_val = num ;
 
        registers_array[RD] = num;     //  Execute LIMM instruction
	}

// --------------------------------------------------------------------------------------------------------------------


	void create_and_execute_ALU(int gpc) {

        selfie5_gcode_cmd_t gcode_type ; 
 
		int RS1 = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ; // randomly select register RS1
		int RS2 = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ; // randomly select register RS2 
		int RD  = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ; // randomly select register RD 
        
        unsigned int alu_result ;
        
        selfie5_instr_t* instruction = &instructions_array[gpc];
                
        instruction->offset = 1 ;
        instruction->RS1 = RS1 ;
        instruction->RS2 = RS2 ;        
        instruction->RD = RD ;


        instruction->RS1_exec_val = registers_array[RS1] ;
        instruction->RS2_exec_val = registers_array[RS2] ;
                
        int random_choice = pos_lfsr_rand_get_next() % 4;   

        switch (random_choice) { 
        case 0:        
          gcode_type = GXOR ; 
          alu_result = registers_array[RS1] ^ registers_array[RS2] ;  
          break;
        case 1:        
          gcode_type = GOR ; 
          alu_result = registers_array[RS1] | registers_array[RS2] ;  
          break;
        case 2:        
          gcode_type = GAND ; 
          alu_result = registers_array[RS1] & registers_array[RS2] ;  
          break;             
        default:
          gcode_type = GADD ; 
          alu_result = registers_array[RS1] + registers_array[RS2] ;          
          break;
        } // switch          

		instruction->gcode_type = gcode_type ;
        registers_array[RD] = alu_result ;     //  Execute gcode_type instruction
        
        // memorize value at execution time, currently each instruction is executed only once.        
        instruction->RD_exec_val  = alu_result ;                
        
	}



// ---------------------------------------------------------------------------------------------------------------------

	void create_and_execute_STOREW(int gpc) {

        int rs_idx = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ; // Select a valid random source reg.
        
		int MD_entry_idx = pos_lfsr_rand_get_next() % NUM_TESTED_DMEM_WORDS ; // Select a valid random memory entry index
        
        selfie5_instr_t* SW_instruction = &instructions_array[gpc];
                
        // Create instruction
        
		SW_instruction->gcode_type = STOREW ;     // Equivalent to instructions_array[gpc].type = LW ;
        SW_instruction->offset = 1 ;              // this is flow/gpc related (if at all needed) , not memory offset
        SW_instruction->RS2 = rs_idx ;        
        
        SW_instruction->MD_entry_idx = MD_entry_idx ;
 
        dmem_array[MD_entry_idx] = registers_array[rs_idx];     //  Reference-Execute SW instruction

        SW_instruction->RS1 = LOAD_STORE_BASE_REG ;
        SW_instruction->RS1_exec_val  = registers_array[SW_instruction->RS1] ;        
        SW_instruction->RS2_exec_val  = registers_array[SW_instruction->RS2] ;                
	}


// --------------------------------------------------------------------------------------------------------------------

	void create_and_execute_LOADW(int gpc) {

        int rd_idx = BASE_TESTED_REG +(pos_lfsr_rand_get_next() % NUM_TESTED_REGS) ; // Select a valid random destination reg.
        
		int MD_entry_idx = pos_lfsr_rand_get_next() % NUM_TESTED_DMEM_WORDS ; // Select a valid random memory entry index

      
        selfie5_instr_t* LW_instruction = &instructions_array[gpc];
                
        // Create instruction
        
		LW_instruction->gcode_type = LOADW ;     // Equivalent to instructions_array[gpc].type = LW ;
        LW_instruction->offset = 1 ;          // this is flow/gpc related (if at all needed) , not memory offset
        LW_instruction->RD = rd_idx ;

        LW_instruction->MD_entry_idx = MD_entry_idx ;
         
        registers_array[rd_idx] = dmem_array[MD_entry_idx];     //  Reference-Execute LW instruction
        
        LW_instruction->RS1 = LOAD_STORE_BASE_REG ;
        LW_instruction->RS1_exec_val  = registers_array[LW_instruction->RS1] ;        
        LW_instruction->RD_exec_val   = registers_array[LW_instruction->RD] ;         

        
	}


// --------------------------------------------------------------------------------------------------------------------


// EXPANSION OF ASSEMBLY CODE TO STAGE 2 


	void create_gcode()   {  // GCODE - Generic Code stage

        int random_choice ;
        //selfie5_gcode_cmd_t expanded_instruction ;
		int gpc = 0;                                                   // Execution trace starts at instruction 0
		for (int cycle = 0; cycle < INSTR_ARRAY_SIZE; cycle++)  {     // Run through assembly code of stage 1 in execution trace order

            selfie5_instr_t* instruction = &instructions_array[gpc];
			
			if (instruction->flow_type == GOTO){  // If GOTO is chosen - we randomly either replace it with BRANCH - Taken or leave it as JMP
                random_choice = pos_lfsr_rand_get_next() % 8;
                switch (random_choice) { 
                  case 0:                                       
                    create_and_execute_JMP(gpc);    // Practically same as JAL but no link (i.e link to R0 in riscv terms)
                    break ;
                  case 1:
                  case 2:	                  
                    create_and_execute_GJAL(gpc); // Also links a register with dut-specific absolute return address.
                    break;
                  case 3:
                  case 4:
                  case 6:                  
                    create_and_execute_GJALR(gpc); // Also links a register with dut-specific absolute return address.
                    break; 
                  default: 
                    get_gcode_branch(gpc, True, instruction->offset); // If branch is taken, we set instruction to JNE or JEQ accordingly                  
                    break ;
                } // switch
			} // if

			else if (instruction->flow_type == STEP) { 
                random_choice = pos_lfsr_rand_get_next() % 8;                
                // If STEP is chosen - we randomly replace it with BRANCH - NOT - Taken or with LIMM,GXOR or with GFENCE
                
                    // TODO a more sophisticated run-time control randomness probabilities efficient allocation
                
                    switch (random_choice) { 
                         case 0 :             
					       //expanded_instruction = LIMM ;
                           create_and_execute_LIMM(gpc);
                           break ;
                         case 1 : 
                         case 2 : 
					       //expanded_instruction = ALU R type (GXOR,GADD etc ...) ;
                           create_and_execute_ALU(gpc);                         
                           break ;
                         case 3 :                            
					       //expanded_instruction = SW ;
                           create_and_execute_STOREW(gpc);
                           break ;
                         case 4 :                    
					      // expanded_instruction = LW ;
                           create_and_execute_LOADW(gpc);
                           break ;                      
                         case 5 :                         
					       //expanded_instruction = GFENCE ;
                           instruction->gcode_type = GFENCE ;
                           break ;
                         default :                         
                           //expanded_instruction =  BRANCH_NOT_TAKEN;                       
                           get_gcode_branch(gpc, False, instruction->offset);
                           break ;                      
                    }   // switch                       
			} // flow_type==STEP

            else if (instruction->flow_type == FLOW_FINISH)  instruction->gcode_type = GEND ;
            
			gpc = gpc + instruction->offset;
		} // for
	} // create_gcode

//------------------------------------------------------------------------------------------------------------------------

   // Used for gcode types prints 

    char * gcode_type_str ;   // Not sure why it does not work when declared inside function    
    char * get_gcode_type_str(selfie5_gcode_cmd_t selfie5_gcode_cmd)  {
  
       #define CASE_ENUM_STR(enum_str,enum_code)  case enum_code: enum_str=""#enum_code""; break; // Local macro to assist below seitcj
      
       switch(selfie5_gcode_cmd) {          // MUST Add an item per enumerated item of selfie5_gcode_cmd_t at selfie_gcode.h
         CASE_ENUM_STR(gcode_type_str, JMP)
         CASE_ENUM_STR(gcode_type_str, GJAL)
         CASE_ENUM_STR(gcode_type_str, GJALR)         
         CASE_ENUM_STR(gcode_type_str, JEQ)
         CASE_ENUM_STR(gcode_type_str, JNE)
         CASE_ENUM_STR(gcode_type_str, LIMM)
         CASE_ENUM_STR(gcode_type_str, GADD)          
         CASE_ENUM_STR(gcode_type_str, GXOR)
         CASE_ENUM_STR(gcode_type_str, GOR)
         CASE_ENUM_STR(gcode_type_str, GAND)         
         CASE_ENUM_STR(gcode_type_str, STOREW)
         CASE_ENUM_STR(gcode_type_str, LOADW) 
         CASE_ENUM_STR(gcode_type_str, GFENCE)
         CASE_ENUM_STR(gcode_type_str, GEND)           
         default: {
          bm_printf("ERROR: selfie5_gcode.c - Invalid code, get_gcode_type_str must handle all codes of selfie5_gcode_cmd_t");
          quit_selfie5() ;
          break ;
         }
       } // switch 
       return gcode_type_str ;
   } 
        
//------------------------------------------------------------------------------------------------------------------------

bool_t is_type_alu(selfie5_gcode_cmd_t gcode_type) {

  return   (gcode_type == GXOR)
         ||(gcode_type == GOR)
         ||(gcode_type == GAND)
         ||(gcode_type == GADD) ;
        
}

//--------------------------------------------------------------------------------------------------------------------------

	void print_instruction_code_gcode() { // Shared for all stages (other than DUT execution)
                   
		for (int instruction_line = 0; instruction_line < INSTR_ARRAY_SIZE; instruction_line++) {
            
            unsigned int dut_byte_addr = DUT_IMEM_CODE_START_ADDR + 4*instruction_line ;
           
			selfie5_instr_t * instruction = &instructions_array[instruction_line];
            
            char * gcode_type_str = get_gcode_type_str(instruction->gcode_type) ;            

			if (instruction->gcode_type == JMP)  {
				vl_bm_printf(3)(" I%-3d (%08x) %08x %7s with offset %-4d TO I%-3d\n" ,
					instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str,
                    instruction->offset, instruction_line + instruction->offset);
			} 
                        
			else if ((instruction->gcode_type == JEQ) || (instruction->gcode_type == JNE)) {
				vl_bm_printf(3)(" I%-3d (%08x) %08x %7s R%-2d,R%-2d with offset %-4d TO I%-3d\n" ,
					instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str, instruction->RS1, instruction->RS2,
                    instruction->offset, instruction_line + instruction->offset);
			} 
            
			else if (instruction->gcode_type == GJAL) {
				vl_bm_printf(3)(" I%-3d (%08x) %08x %7s with offset %-4d TO I%-3d R%-2d\n" ,
					instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str, 
                    instruction->offset, instruction_line + instruction->offset, instruction->RD);
			}   
 
			else if (instruction->gcode_type == GJALR) {
				vl_bm_printf(3)(" I%-3d (%08x) %08x %7s R%-2d with offset %-4d TO I%-3d R%-2d\n" ,
					instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str, instruction->RS1,
                    instruction->offset, instruction_line + instruction->offset, instruction->RD);
			}   
 
			else if (instruction->gcode_type == LIMM) {
                vl_bm_printf(3)(" I%-3d (%08x) %08x %7s R%-2d %08x\n", 
                instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str, 
                instruction->RD, instruction->RD_exec_val) ;
            }
            
			else if  (is_type_alu(instruction->gcode_type)) {
                vl_bm_printf(3)(" I%-3d (%08x) %08x %7s R%-2d, R%-2d,R%-2d\n", 
                instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str, 
                instruction->RD,instruction->RS1,instruction->RS2) ;
            }
			else if (instruction->gcode_type == LOADW) {
                vl_bm_printf(3)(" I%-3d (%08x) %08x %7s R%-2d M[%d]\n", 
                instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str, 
                instruction->RD, instruction->MD_entry_idx) ;
            }

			else if (instruction->gcode_type == STOREW) {
                vl_bm_printf(3)(" I%-3d (%08x) %08x %7s R%-2d M[%d]\n", 
                instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str, 
                instruction->RS2, instruction->MD_entry_idx) ;
            }

			else if (instruction->gcode_type == GFENCE) {
                vl_bm_printf(3)(" I%-3d (%08x) %08x %7s\n",
                instruction_line, dut_byte_addr, instruction->riscv_code, gcode_type_str) ;
            }            
            
		    else if (instruction->gcode_type == GEND) {
                    vl_bm_printf(3)(" I%-3d (%08x) %7s\n", instruction_line, dut_byte_addr,  gcode_type_str);
            }                             
		} // for
	}

// ----------------------------------------------------------------------------------------------------------------------	

	void print_gcode_execution_trace() {
		int gpc = 0;   // Gcode execution trace starts at instruction 0
        
		for (int cycle = 0; cycle < INSTR_ARRAY_SIZE; cycle++) {
            
            unsigned int dut_byte_addr = DUT_IMEM_CODE_START_ADDR + 4*gpc ;

			selfie5_instr_t* instruction = &(instructions_array[gpc]);
            
            char * gcode_type_str = get_gcode_type_str(instruction->gcode_type) ;            
            
			if ((instruction->gcode_type == JEQ) || (instruction->gcode_type == JNE)) {

				vl_bm_printf(3) (" C%-3d I%-3d (%08x) %08x %7s R%-2d = %08x , R%-2d = %08x, offset %-4d %7s\n",
					cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str, instruction->RS1, instruction->RS1_exec_val , instruction->RS2,
                    instruction->RS2_exec_val, instruction->offset,                    
                    instruction->is_taken ? "taken" : "not_taken");                    
			}
			else if (instruction->gcode_type == JMP) {
               vl_bm_printf(3)(" C%-3d I%-3d (%08x) %08x %7s TO I%-3d\n",
               cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str, gpc + instruction->offset);                    
            }   

			else if (instruction->gcode_type == GJAL) {
               vl_bm_printf(3)(" C%-3d I%-3d (%08x) %08x %7s TO I%-3d R%-2d = %08x\n", 
               cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str, gpc + instruction->offset, instruction->RD, instruction->RD_exec_val);                    
            } 

			else if (instruction->gcode_type == GJALR) {
               vl_bm_printf(3)(" C%-3d I%-3d (%08x) %08x %7s TO I%-3d  R%-2d = %08x R%-2d = %08x\n", 
               cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str, gpc + instruction->offset, 
               instruction->RS1, instruction->RS1_exec_val, instruction->RD, instruction->RD_exec_val);                    
            } 
            
            else if (instruction->gcode_type == LIMM) {
               vl_bm_printf(3)(" C%-3d I%-3d (%08x) %08x %7s R%-2d = %08x\n",
               cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str, instruction->RD, instruction->RD_exec_val);
            }
            
            else if (is_type_alu(instruction->gcode_type)) {
               vl_bm_printf(3)(" C%-3d I%-3d (%08x) %08x %7s R%-2d %08x R%-2d %08x -> R%-2d = %08x\n",
                cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str,               
                instruction->RS1, instruction->RS1_exec_val, 
                instruction->RS2, instruction->RS2_exec_val,              
                instruction->RD,  instruction->RD_exec_val);
            }           
            
            else if (instruction->gcode_type == LOADW) {
               vl_bm_printf(3)(" C%-3d I%-3d (%08x) %08x %7s R%-2d <- M[%d]\n", 
               cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str, instruction->RD, instruction->MD_entry_idx);
            }

            else if (instruction->gcode_type == STOREW) {
               vl_bm_printf(3)(" C%-3d I%-3d (%08x) %08x %7s R%-2d -> M[%d]\n", 
               cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str, instruction->RS2, instruction->MD_entry_idx);
            }                   
            else if (instruction->gcode_type == GFENCE) {
               vl_bm_printf(3)(" C%-3d I%-3d (%08x) %08x %7s\n", cycle, gpc, dut_byte_addr, instruction->riscv_code, gcode_type_str);
            }                        
            else if ( (instruction->gcode_type == GEND)) {
                vl_bm_printf(3)(" C%-3d I%-3d %7s\n", cycle, gpc,  gcode_type_str);
		    }

		    gpc = gpc + instruction->offset;     //update next instruction to be executed
		} // for
	}

