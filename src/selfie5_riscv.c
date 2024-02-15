
#include "selfie5_common.h"

//----------------------------------------------------------------------------------------------------

extern selfie5_instr_t instructions_array[];
extern unsigned int registers_array[];

//-----------------------------------------------------------------------------------------------------

funct3_t gcode_to_riscv_f3(selfie5_gcode_cmd_t  gcode_type) {
    switch (gcode_type) { 
       case GXOR: return F3_XOR ;
       case GADD: return F3_ADD ;
       case GOR:  return F3_OR  ;
       case GAND: return F3_AND ;       
       default: {
          bm_printf("ERROR gcode_to_riscv_f3 non supported gcode\n");
          quit_selfie5();
          return 0;
       }
    }   
}

//------------------------------------------------------------------------------------------------------

// generate the riscv machine code for the instructions.

void gen_riscv_code() {  // Stage 3 ASM , Machine specific assembly code
     
     
	for (int instruction_line = 0; instruction_line < INSTR_ARRAY_SIZE; instruction_line++) {
         selfie5_instr_t * instr_p = &instructions_array[instruction_line];
         
         if ((instr_p->gcode_type == JEQ) || (instr_p->gcode_type == JNE)) {  // handle B-Type
             btype_imm_t  imm_value ;
             btype_instr_t instr_code ; 
             
             imm_value.imm = (signed int)(4*instr_p->offset) ;
                               

             instr_code.field.opcode     = OPC_BRANCH ;
             instr_code.field.funct3     = (instr_p->gcode_type==JEQ) ? F3_BEQ : F3_BNE ;
             instr_code.field.rs1        = instr_p->RS1 ;
             instr_code.field.rs2        = instr_p->RS2 ;
             instr_code.field.imm_11     = imm_value.field.imm_11 ;    
             instr_code.field.imm_4_1    = imm_value.field.imm_4_1 ;                                                 
             instr_code.field.imm_10_5   = imm_value.field.imm_10_5 ;
             instr_code.field.imm_12     = imm_value.field.imm_12 ; 

		instr_p->riscv_code = instr_code.code;
        
        instr_p->riscv_instr_type = B_type ;       
        instr_p->riscv_opc = instr_code.field.opcode; 
        instr_p->riscv_imm = imm_value.imm;
        instr_p->riscv_f3  = instr_code.field.funct3 ; 
 
        } // JEQ,JNE
	
	    else if ((instr_p->gcode_type==JMP)||(instr_p->gcode_type==GJAL)) {   // handle J-Type
	    		jtype_imm_t  imm_value;
	    		jtype_instr_t instr_code;
        
	    		imm_value.imm = (signed int)(4 * instr_p->offset);
        
	    		instr_code.field.opcode = OPC_JAL;
               
                if (instr_p->gcode_type==JMP)
	    		   instr_code.field.rd = 0 ; // do not link a return address (i.e. link it to R0 fixed to zero)
                 else // GJAL case
                   instr_code.field.rd = instr_p->RD; 

	    		instr_code.field.imm_19_12 = imm_value.field.imm_19_12;
	    		instr_code.field.imm_11 = imm_value.field.imm_11;
	    		instr_code.field.imm_10_1 = imm_value.field.imm_10_1;
	    		instr_code.field.imm_20 = imm_value.field.imm_20;
	    		instr_p->riscv_code = instr_code.code;

                instr_p->riscv_instr_type = J_type ;       
                instr_p->riscv_opc = instr_code.field.opcode;
                instr_p->riscv_imm = imm_value.imm;                
                                
        } // JMP=JAL0/JAL
             

	    else if (instr_p->gcode_type==GJALR) {   // handle I-Type
        
	      	   itype_imm_t  imm_value;
	      	   itype_instr_t instr_code;

               int jump_dest_addr =  DUT_IMEM_CODE_START_ADDR + (4*(instruction_line + (instr_p->offset)));
               int jalr_rs1_offset = jump_dest_addr - (signed int)(instr_p->RS1_exec_val)  ;     
	      	   imm_value.imm = (signed int)jalr_rs1_offset ;
                  
	      	   instr_code.field.opcode = OPC_JALR;
	      	   instr_code.field.funct3 = F3_ADD;
	      	   instr_code.field.rd = instr_p->RD;
	      	   instr_code.field.rs1 = instr_p->RS1 ;	
	      	   instr_code.field.imm_11_0 = imm_value.field.imm_11_0;
               instr_p->riscv_code = instr_code.code;        

               instr_p->riscv_instr_type = I_type ;       
               instr_p->riscv_opc = instr_code.field.opcode ;       
               instr_p->riscv_f3 = instr_code.field.funct3 ; 
               instr_p->riscv_imm = imm_value.imm;

        } // JALR


        else if (instr_p->gcode_type == LIMM) {    // handle I-Type
	      	   itype_imm_t  imm_value;
	      	   itype_instr_t instr_code;
                      
	      	   imm_value.imm = (signed int)(instr_p->RD_exec_val) ;
                      
	      	   instr_code.field.opcode = OPC_OPIMM;
	      	   instr_code.field.funct3 = F3_ADD;
	      	   instr_code.field.rd = instr_p->RD;
	      	   instr_code.field.rs1 = 0;	
	      	   instr_code.field.imm_11_0 = imm_value.field.imm_11_0;
               instr_p->riscv_code = instr_code.code;  

               instr_p->riscv_instr_type = I_type ;       
               instr_p->riscv_opc = instr_code.field.opcode ;       
               instr_p->riscv_f3 = instr_code.field.funct3 ;
               instr_p->riscv_imm = imm_value.imm;               

               
         } // LIMM

        else if (is_type_alu(instr_p->gcode_type)) {    // handle R-Type ALU

	      	   rtype_instr_t instr_code;               
	      	   instr_code.field.opcode = OPC_OP;
	      	   instr_code.field.funct3 = gcode_to_riscv_f3(instr_p->gcode_type) ;
	      	   instr_code.field.rd = instr_p->RD;
	      	   instr_code.field.rs1 = instr_p->RS1;	
	      	   instr_code.field.rs2 = instr_p->RS2;	               
	      	   instr_code.field.funct7 = 0 ;
               instr_p->riscv_code = instr_code.code;  

               instr_p->riscv_instr_type = R_type ;       
               instr_p->riscv_opc = instr_code.field.opcode ;       
               instr_p->riscv_f3 = instr_code.field.funct3 ; 
             
         } // ALU


        else if (instr_p->gcode_type == LOADW) {    // handle I-Type
	      	   itype_imm_t  imm_value;
	      	   itype_instr_t instr_code;
                      
	      	   imm_value.imm = LOAD_STORE_BASE_OFFSET + 4*(signed int)(instr_p->MD_entry_idx) ; // Memory address offset in bytes
                      
	      	   instr_code.field.opcode = OPC_LOAD;
	      	   instr_code.field.funct3 = F3_LW;
	      	   instr_code.field.rd = instr_p->RD;
	      	   instr_code.field.rs1 = LOAD_STORE_BASE_REG ;	
	      	   instr_code.field.imm_11_0 = imm_value.field.imm_11_0;
               instr_p->riscv_code = instr_code.code;    

               instr_p->riscv_instr_type = I_type ;       
               instr_p->riscv_opc = instr_code.field.opcode ;       
               instr_p->riscv_f3 = instr_code.field.funct3 ; 
               instr_p->riscv_imm = imm_value.imm;
               instr_p->RS1 = LOAD_STORE_BASE_REG;	
              
         } // LW


        else if (instr_p->gcode_type == STOREW) {    // handle S-Type
	      	   stype_imm_t  imm_value;
	      	   stype_instr_t instr_code;
                      
	      	   imm_value.imm = LOAD_STORE_BASE_OFFSET + 4*(signed int)(instr_p->MD_entry_idx) ; // Memory address offset in bytes
                      
	      	   instr_code.field.opcode = OPC_STORE;
	      	   instr_code.field.funct3 = F3_SW;
	      	   instr_code.field.rs1 = LOAD_STORE_BASE_REG ;	
	      	   instr_code.field.rs2 = instr_p->RS2 ; ;	               
	      	   instr_code.field.imm_11_5 = imm_value.field.imm_11_5;
	      	   instr_code.field.imm_4_0  = imm_value.field.imm_4_0;               
               instr_p->riscv_code = instr_code.code; 

               instr_p->riscv_instr_type = S_type ;       
               instr_p->riscv_opc = instr_code.field.opcode ;       
               instr_p->riscv_f3 = instr_code.field.funct3 ;
               instr_p->riscv_imm = imm_value.imm; 
               instr_p->RS1 = LOAD_STORE_BASE_REG;               
               
         } // SW
         
   
         else if (instr_p->gcode_type == GFENCE) { 
               instr_p->riscv_code = 0x0ff0000f ; // riscv fence code
               instr_p->riscv_opc = OPC_FENCE ;       
               
         } // GFENCE  
   
  } // for
 } // gen_riscv_code
