


#include "selfie5_common.h"


//----------------------------------------------------------------------------------------------------------------------

extern char  flow_types_str[][MAX_STR_LEN] ;

//----------------------------------------------------------------------------------------------------------------------

extern char is_address_available[INSTR_ARRAY_SIZE] ;
extern selfie5_instr_t instructions_array[INSTR_ARRAY_SIZE]; 

int get_next_available_address() {
	int available_address;
	bool_t found_available_address = False;

	available_address = pos_lfsr_rand_get_next()%(INSTR_ARRAY_SIZE - 1);            // Returns a number between 0 (included) and instruction array last cell(NOT included)
	for (int address = available_address; address < INSTR_ARRAY_SIZE - 1; address++) {  // Cyclical loop over instruction array to find first random available addredss
		if (is_address_available[address] == True)  {
			available_address = address;                                          // We break search when finding the first available address
			found_available_address = True;
			break;
		}
	}
	if (found_available_address == False)  {
		for (int address = 0; address < available_address; address++)  {
			if (is_address_available[address] == True)
			{
				available_address = address;
				found_available_address = True;
				break;
			}
		}
	}
	if (!found_available_address) available_address = INSTR_ARRAY_SIZE - 1;   // If we looped over all addresses except for the last and all were unavailable, then we are at the final instruction

	return available_address;
}

// ---------------------------------------------------------------------------------------------------------------------

   void create_goto(int current_address) {  // returns offset
	
	selfie5_instr_t * new_goto = &instructions_array[current_address] ;
	
	int goto_destination = get_next_available_address();
	int offset = goto_destination - current_address;

	new_goto->flow_type = GOTO ;     // equivalent to instructions_array[current_address].type = GOTO
	new_goto->offset = offset;

}
// ----------------------------------------------------------------------------------------------------------------------

// STAGE 1CREATION OF INITIAL FLOW (GOTO,STEP Commands only)

	void create_flow() {  // FLOW Stage

		int instruction_prior_to_last = INSTR_ARRAY_SIZE - 2;
		int current_address_in_creation = 0;
		selfie5_flow_cmd_t new_instruction_type ;


		for (int i=0; i < INSTR_ARRAY_SIZE - 1; i++) {

		    selfie5_instr_t * current_instruction = &instructions_array[current_address_in_creation] ;


			is_address_available[current_address_in_creation] = False;                    //Mark the address line as unavailable
			if ((is_address_available[current_address_in_creation + 1] == False) || (current_address_in_creation == instruction_prior_to_last)) //If the next address line is unavailable or if we reached the one instruction prior to last - the current instruction must be GOTO
			{
				new_instruction_type = GOTO;
			}
			else new_instruction_type = (selfie5_flow_cmd_t) (pos_lfsr_rand_get_next() % NUM_RAND_FLOW_CMD) ;              // Choose randomly the next S1 instruction
			
			if (new_instruction_type == STEP) {   // If STEP, create Instruction with offset 1
				current_instruction->flow_type = STEP;
				current_instruction->offset = 1;
			}
			else if (new_instruction_type == GOTO) create_goto(current_address_in_creation);                               // If GOTO, create Instruction with legal offset
				
			current_address_in_creation = current_address_in_creation + current_instruction->offset;    // Go to next address for creation
			
		}
		instructions_array[INSTR_ARRAY_SIZE - 1].flow_type = FLOW_FINISH;
		instructions_array[INSTR_ARRAY_SIZE - 1].offset = 0;
	}

// ----------------------------------------------------------------------------------------------------------------------
                                                                                              
	void print_instruction_code_flow() { // Shared for all stages (other than DUT execution)
    
        // NOTICE , *_type_str must be in EXACT order as in corresponding _cmd_t
                
		for (int instruction_line = 0; instruction_line < INSTR_ARRAY_SIZE; instruction_line++)
		{
			selfie5_instr_t * instruction = &instructions_array[instruction_line];
			if (instruction->flow_type == GOTO) {
				vl_bm_printf(3)(" I%-3d (%08x) %-4s with offset %4d TO I%-3d\n" ,
					instruction_line, 4*instruction_line, flow_types_str[instruction->flow_type], 
                    instruction->offset, instruction_line + instruction->offset);
			}
		    else if ((instruction->flow_type == STEP) || (instruction->flow_type == FLOW_FINISH)) {
                      vl_bm_printf(3)(" I%-3d (%08x) %-4s\n", instruction_line, 4*instruction_line, flow_types_str[instruction->flow_type]);                       
            }                             
		} // for
	}

// ----------------------------------------------------------------------------------------------------------------------	

   //Execution trace starts at instruction 0

   void print_flow_execution_trace() {  
   	int pc = 0;                               
   
   	for (int cycle = 0; cycle < INSTR_ARRAY_SIZE; cycle++) {
   
   		selfie5_instr_t* instruction = &instructions_array[pc];
              char * type_str = flow_types_str[instruction->flow_type] ;        
              if ((instruction->flow_type == GOTO)||(instruction->flow_type == STEP)) {
                  vl_bm_printf(3) ("C%-3d I%-3d %-4s TO I%-3d\n", cycle, pc, type_str , pc + instruction->offset);
              }
             else if (instruction->flow_type == FLOW_FINISH) {
                  vl_bm_printf(3)("C%-3d I%-3d %-4s\n", cycle, pc, type_str);
   	    }
   		pc = pc + instruction->offset;     //update next instruction to be executed
   	}
   }

