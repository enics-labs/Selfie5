
#include "stdio.h"
#include "selfie5_common.h"

//----------------------------------------------------------------------------------------------------------------------


extern selfie5_instr_t instructions_array[INSTR_ARRAY_SIZE];        // This array represents the 32 instructions
extern unsigned int    registers_array[32];                         // This array maintains the content of the processor registers
extern unsigned int    dmem_array[NUM_TESTED_DMEM_WORDS];           // This array maintains the content of the tested data-memory

//----------------------------------------------------------------------------------------------------------------------


char * reg_str(int reg_idx) {
  
  switch (reg_idx) {
      case  0 : return "0" ; break ; 
      case  1 : return "ra" ; break ; 
      case  2 : return "sp" ; break ; 
      case  3 : return "gp" ; break ; 
      case  4 : return "tp" ; break ; 
      case  5 : return "t0" ; break ; 
      case  6 : return "t1" ; break ; 
      case  7 : return "t2" ; break ; 
      case  8 : return "s0" ; break ; 
      case  9 : return "s1" ; break ; 
      case 10 : return "a0" ; break ; 
      case 11 : return "a1" ; break ; 
      case 12 : return "a2" ; break ; 
      case 13 : return "a3" ; break ; 
      case 14 : return "a4" ; break ; 
      case 15 : return "a5" ; break ; 
      case 16 : return "a6" ; break ; 
      case 17 : return "a7" ; break ; 
      case 18 : return "s2" ; break ; 
      case 19 : return "s3" ; break ; 
      case 20 : return "s4" ; break ; 
      case 21 : return "s5" ; break ; 
      case 22 : return "s5" ; break ; 
      case 23 : return "s7" ; break ; 
      case 24 : return "s8" ; break ; 
      case 25 : return "s9" ; break ; 
      case 26 : return "s10" ; break ; 
      case 27 : return "s11" ; break ; 
      case 28 : return "t3" ; break ; 
      case 29 : return "t4" ; break ; 
      case 30 : return "t5" ; break ; 
      case 31 : return "t6" ; break ;
      default : return "MISSING" ;      
  }
  
}

//--------------------------------------------------------------------------------------------------------------------------

char * riscv_mnemonic_str(opc_t riscv_opc, funct3_t riscv_f3) {

  char is_branch = False ;
  char is_alu = False ;
  char is_alu_imm = False ;  
  
  switch(riscv_opc) { 
   
    case OPC_SYSTEM  : return "sys"   ; break ;
    case OPC_FENCE   : return "fence" ; break ;

    case OPC_STORE   : return "sw"    ; break ; // TODO handle byte and half load/stare instructions
    case OPC_LOAD    : return "lw"    ; break ; // TODO handle byte and half load/stare instructions
    case OPC_JALR    : return "jalr"  ; break ;
    case OPC_JAL     : return "jal"   ; break ;
    case OPC_AUIPC   : return "auipc" ; break ;
    case OPC_LUI     : return "lui"   ; break ;

    case OPC_OP      : is_alu = True     ; break ;      
    case OPC_BRANCH  : is_branch = True  ; break ; 
    case OPC_OPIMM   : is_alu_imm = True ; break ;  
    
    default     : {
        bm_printf("trace switch(riscv_opc) missing switch case %d\n",riscv_opc);
        bm_quit_app();
    }    
  }

  if (is_branch) {
    switch(riscv_f3) {
      
      case F3_BEQ : return "beq"  ; break ; 
      case F3_BNE : return "bne"  ; break ; 
      case F3_BLT : return "blt"  ; break ; 
      case F3_BGE : return "bge"  ; break ; 
      case F3_BLTU: return "bltu" ; break ; 
      case F3_BGEU: return "bgeu" ; break ;  
      
      default     : {
          bm_printf("trace is_branch switch(riscv_f3) missing switch case %d\n",riscv_f3);
          bm_quit_app();
      }
    }  
  }
    
  if (is_alu) {
    switch(riscv_f3) {
        case F3_ADD  : return "add"  ; break ;  // TODO missing sub   
        case F3_SLTS : return "slts" ; break ;
        case F3_SLTU : return "sltu" ; break ;
        case F3_XOR  : return "xor"  ; break ;
        case F3_OR   : return "or"   ; break ;
        case F3_AND  : return "and"  ; break ;
        case F3_SLL  : return "sll"  ; break ;
        case F3_SR   : return "sr"   ; break ;
        
        default     : {
            bm_printf("trace is_alu switch(riscv_f3) missing switch case %d\n",riscv_f3);
            bm_quit_app();
        }
    }    
  }   


  else if (is_alu_imm) {
         switch(riscv_f3) {
            case F3_ADD  : return "addi"  ;     
            case F3_SLTS : return "sltsi" ;
            case F3_SLTU : return "sltui" ;
            case F3_XOR  : return "xori"  ;
            case F3_OR   : return "ori"   ;
            case F3_AND  : return "andi"  ;
            case F3_SLL  : return "slli"  ;
            case F3_SR   : return "sri"   ;
            
            default     : {
                bm_printf("trace is_alu_imm switch(riscv_f3) missing switch case %d\n",riscv_f3);
                bm_quit_app();
            }
         }
  }

  return "MISSING" ;

}

//----------------------------------------------------------------------------------------------------------------------

extern unsigned int dmem_load_store_space[] ;
  
void print_full_trace() {

	int gpc = 0;  //Execution trace starts at instruction 0
    
    // print header

    bm_printf("\n\nFull Consolidated Trace\n\n");

    bm_printf("CYC LOC FLOW TO  GCODE RD  RS1  RS2 TO   ");   
    bm_printf("CBT PC       CODE        INST    OPS      ");
    bm_printf("RD  RDV      RS1 RS1V     RS2 RS2V     IMM      DADDR\n\n");    
    
	for (int cycle = 0; cycle < INSTR_ARRAY_SIZE; cycle++) {

        char ops_str[40]  ;
        char imm_str[18] ;
        char offset_str[18] ;        
        char rd_str [18] ;  
        char rs1_str[18] ;          
        char rs2_str[18] ; 
        
		selfie5_instr_t* instruction = &(instructions_array[gpc]);  

        char rd_exist  =   (instruction->riscv_instr_type!=S_type)&&(instruction->riscv_instr_type!=B_type);
        char rs1_exist =   (instruction->riscv_instr_type!=U_type)&&(instruction->riscv_instr_type!=J_type);
        char rs2_exist =   (instruction->riscv_instr_type==S_type)||(instruction->riscv_instr_type==B_type)||(instruction->riscv_instr_type==R_type);
        char imm_exist =   (instruction->riscv_instr_type!=R_type);
         
        char is_load = instruction->riscv_opc==OPC_LOAD  ; 
        char is_store = instruction->riscv_opc==OPC_STORE ;        
        char is_ls_instr = is_store||is_load ;                
       
        int next_gpc = gpc + instruction->offset;
        unsigned int dut_instr_byte_addr = DUT_IMEM_CODE_START_ADDR + 4*gpc ;
          
        char * gcode_type_str = get_gcode_type_str(instruction->gcode_type) ; 

        // FLOW print

        char is_goto = instruction->flow_type == GOTO ;
        char is_cond_br = ((instruction->gcode_type==JEQ)||(instruction->gcode_type==JNE));
        char * flow_str = (instruction->flow_type == STEP) ? "STEP" : (is_goto ? "GOTO" : "DONE");
                
        bm_printf("%3d %3d %s ",cycle,gpc,flow_str);
        if (is_goto) bm_printf("%2d ",next_gpc); else bm_printf("   ") ;
        
        // GCODE print
        
        char is_jmp = (instruction->gcode_type==JMP) ;
        char is_gj = is_jmp||(instruction->gcode_type==GJAL)||(instruction->gcode_type==GJALR) ;
        char is_glimm = (instruction->gcode_type==LIMM) ;        
        char offset_exist = is_gj || instruction->is_taken ;
        
        if (offset_exist) sprintf(offset_str,"I%-2d", gpc+instruction->offset);
        else sprintf(offset_str,"%-3s","");

        if (rd_exist && !is_jmp) sprintf(rd_str,"R%-2d",instruction->RD);
        else sprintf(rd_str,"%-3s","");
        
        if (rs2_exist && !is_glimm) sprintf(rs2_str,"R%-2d",instruction->RS2);
        else sprintf(rs2_str,"%s","   ");
        
        if (is_ls_instr) sprintf(rs1_str,"M[%d]",instruction->MD_entry_idx) ;        
        else if (rs1_exist) sprintf(rs1_str,"R%-3d",instruction->RS1) ;       
        else  sprintf(rs1_str,"%-4s","");       

        
        if ((instruction->gcode_type==GFENCE)||(instruction->gcode_type==GEND)) sprintf(ops_str,"%s","");
        else if (is_glimm) sprintf(ops_str,"%-3s %-3d",rd_str,instruction->RD_exec_val) ;
        else sprintf(ops_str,"%s %s %s %s",rd_str,rs1_str,rs2_str,offset_str); 

        char print_str[80] ;
        sprintf(print_str,"%6s %-17s %3s ",
        gcode_type_str, ops_str ,is_cond_br ? (instruction->is_taken ? "YES" : "NO") : "  ") ;
        bm_printf("%s",print_str) ;

        // RISCV print
                
        bm_printf("%08x %08x ", dut_instr_byte_addr, instruction->riscv_code);
        
        
        if (cycle!=(INSTR_ARRAY_SIZE-1)) { 
        

          if (imm_exist) sprintf(imm_str,"%d",(int)instruction->riscv_imm);
          else sprintf(imm_str,"%s","");

          if (rd_exist) sprintf(rd_str,"%s,",reg_str(instruction->RD));
          else sprintf(rd_str,"%s","");
          
          if (rs2_exist) sprintf(rs2_str,"%s%s",reg_str(instruction->RS2), imm_exist?",":"");
          else sprintf(rs2_str,"%s","");
             
          if (rs1_exist) sprintf(rs1_str,"%s%s",reg_str(instruction->RS1), (rs2_exist|imm_exist)?",":"") ;
          else sprintf(rs1_str,"%s","");          
          
          if (instruction->riscv_opc==OPC_FENCE) sprintf(ops_str,"%s","");
          else sprintf(ops_str,"%s%s%s%s",rd_str,rs1_str,rs2_str,imm_str); 
  
          bm_printf("%6s %10s   ",riscv_mnemonic_str(instruction->riscv_opc, instruction->riscv_f3),ops_str) ;  

          if (instruction->riscv_opc!=OPC_FENCE) {
              if (rd_exist) bm_printf("x%-2d %08x ",instruction->RD,instruction->RD_exec_val);
              else bm_printf("%13s","");
              
              bm_printf("x%-2d %08x ",instruction->RS1,instruction->RS1_exec_val);
              
              if (rs2_exist)
                 bm_printf("x%-2d %08x ",instruction->RS2,instruction->RS2_exec_val);
              else bm_printf("%13s","");
              
              if (imm_exist) bm_printf("%08x ",instruction->riscv_imm) ;
              else bm_printf("%9s","");
              
              if (is_ls_instr) {
                int dmem_addr = (int)dmem_load_store_space + (int)instruction->riscv_imm ;
                bm_printf("%08x ",(unsigned int)dmem_addr) ;
              }
          }
        }  ; 
        
        bm_printf("\n");
        
	    gpc =  next_gpc ; //update next instruction to be executed
        
           
	} // for
    
}


//==================================================================================

// Basic coverage check


  int check_gcode_coverage() {
      
     char gcode_count_vec[NUM_GCODES] ; 
     int num_gcodes_tested = 0 ;
     
     for (int i=0;i<NUM_GCODES;i++) gcode_count_vec[i] = 0 ;
     for (int cycle = 0; cycle < INSTR_ARRAY_SIZE; cycle++)       
         gcode_count_vec[instructions_array[cycle].gcode_type] +=1 ;
     
     for (int i=0;i<NUM_GCODES;i++) if (gcode_count_vec[i]>0) num_gcodes_tested++ ;
     
     return ((1000*num_gcodes_tested)/NUM_GCODES)/10 ;
          
  } 
    