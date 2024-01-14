
#ifndef _SELFIE5_RISCV_H_PARSED__

// IMPORTANT: NEVER CHANGE ORDER OF FIELDS IN THE STRUCTURES, IT INDEED MATTERS AS WE ARE CASTING VALUES INTO IT.
// Compliant with  section 2.3 (pages 16-17) in the ISA spec at 
// https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf

//-----------------------------------------------------------------------------------

// B-Type template

typedef union {
  struct btype_instr {
    unsigned int opcode   : 7;
    unsigned int imm_11   : 1;   // imm[11]   
    unsigned int imm_4_1  : 4;  // imm[4:1]
    unsigned int funct3   : 3;
    unsigned int rs1      : 5;
    unsigned int rs2      : 5;
    unsigned int imm_10_5 : 6;  // imm[5:0]
    unsigned int imm_12   : 1;  // imm[12]
  } field ;
  unsigned int code ;
} btype_instr_t;

typedef union {
  struct btype_imm {
    unsigned int imm_0     : 1;  // always holds 0 
    unsigned int imm_4_1   : 4;  // inst[11:8]
    unsigned int imm_10_5  : 6;  // inst[30:25]   
    unsigned int imm_11    : 1;  // inst[7]
    unsigned int imm_12    : 1;  // all bits are inst[31] (sign extend)
    unsigned int imm_31_13 : 19; // all bits are imm_12 (inst[31]) sign extend
  } field ;
  signed int imm ;  
} btype_imm_t;

//-----------------------------------------------------------------------------------


// J-Type template

typedef union {
    struct jtype_instr {
        unsigned int opcode    : 7;
        unsigned int rd        : 5;   
        unsigned int imm_19_12 : 8;  // imm[19:12]
        unsigned int imm_11    : 1;  // imm[11]
        unsigned int imm_10_1  : 10; // imm[10:1]
        unsigned int imm_20    : 1;  // imm[20]
    } field ;
    unsigned int code;
} jtype_instr_t;

typedef union {
    struct jtype_imm {
        unsigned int imm_0     : 1;     // allways holds 0 
        unsigned int imm_10_1  : 10;    // inst[24:21], inst[30:25]   
        unsigned int imm_11    : 1;     // inst[20]
        unsigned int imm_19_12 : 8;     // inst[19:12]
        unsigned int imm_20    : 1;     // all bits are inst[31] (sign extend)
        unsigned int imm_31_21 : 11;    // all bits are imm_20 (inst[31]) sign extend
    } field;
    signed int imm;
} jtype_imm_t;

//-----------------------------------------------------------------------------------

// I-Type template

typedef union {
    struct itype_instr {
        unsigned int opcode   : 7;
        unsigned int rd       : 5;     
        unsigned int funct3   : 3;
        unsigned int rs1      : 5;
        unsigned int imm_11_0 : 12;  // imm[11:0]
    } field;
    unsigned int code;
} itype_instr_t;


typedef union {
    struct itype_imm {
        unsigned int imm_11_0  : 12; // inst[20],inst[24:21],inst[30:25]   
        unsigned int imm_31_12 : 20; // all bits are imm_11 (inst[31]) sign extend
    } field;
    signed int imm;
} itype_imm_t;


//-----------------------------------------------------------------------------------


// R-Type template

// Notice There are no immediate fields in R-Type

typedef union {
    struct rtype_instr {
        unsigned int opcode   : 7;
        unsigned int rd       : 5;     
        unsigned int funct3   : 3;
        unsigned int rs1      : 5;
        unsigned int rs2      : 5;        
        unsigned int funct7   : 7;  
    } field;
    unsigned int code;
} rtype_instr_t;


//-----------------------------------------------------------------------------------



// S-Type template


typedef union {
    struct stype_instr {
        unsigned int opcode   : 7;
        unsigned int imm_4_0  : 5; // imm[4:0]
        unsigned int funct3   : 3;
        unsigned int rs1      : 5;
        unsigned int rs2      : 5;        
        unsigned int imm_11_5 : 7; // imm[11:5]
    } field;
    unsigned int code;
} stype_instr_t;


typedef union {
    struct stype_imm {
        unsigned int imm_4_0   : 5;  // inst[11:7]   
        unsigned int imm_11_5  : 7;  // inst[31:25]
        unsigned int imm_31_12 : 20; // all bits are imm_11 (inst[31]) sign extend
    } field;
    signed int imm;
} stype_imm_t;

//---------------------------------------------------------------------------

typedef enum {   

  R_type, // register-register.
  I_type, // short immediate and loads.
  S_type, // stores.
  B_type, // conditional branches, a variation of S-type.
  U_type, // long immediate.
  J_type  // unconditional jumps, a variation of U-type.
  
} riscv_instr_type_t;

//---------------------------------------------------------------------------

typedef enum {

    OPC_SYSTEM    = 0x73,
    OPC_FENCE     = 0x0f,
    OPC_OP        = 0x33,
    OPC_OPIMM     = 0x13,
    OPC_STORE     = 0x23,
    OPC_LOAD      = 0x03,
    OPC_BRANCH    = 0x63,
    OPC_JALR      = 0x67,
    OPC_JAL       = 0x6f,
    OPC_AUIPC     = 0x17,
    OPC_LUI       = 0x37,
    OPC_OP_FP     = 0x53,
    OPC_OP_FMADD  = 0x43,
    OPC_OP_FNMADD = 0x4f,
    OPC_OP_FMSUB  = 0x47,
    OPC_OP_FNMSUB = 0x4b,
    OPC_STORE_FP  = 0x27,
    OPC_LOAD_FP   = 0x07,

    // PULP custom
    OPC_LOAD_POST  = 0x0b,
    OPC_STORE_POST = 0x2b,
    OPC_PULP_OP    = 0x5b,
    OPC_VECOP      = 0x57,
    OPC_HWLOOP     = 0x7b
    
} opc_t;

typedef enum {

    // Branches F3 (B-Type)    
    F3_BEQ = 0b000,
    F3_BNE = 0b001,
    F3_BLT = 0b100,
    F3_BGE = 0b101,
    F3_BLTU = 0b110,
    F3_BGEU = 0b111,
    
    // ALU F3 (R,I Types)    
    F3_ADD  = 0b000 ,  // Add                              // TODO Missing SUB
    F3_SLTS = 0b010 ,  // Set to one if Lower Than 
    F3_SLTU = 0b011 ,  // Set to one if Lower Than , Unsigned
    F3_XOR  = 0b100 ,  // Exclusive Or 
    F3_OR   = 0b110 ,  // Or 
    F3_AND  = 0b111 ,  // And with Immediate
    F3_SLL  = 0b001 , // Shift Left Logical by
    F3_SR   = 0b101 , // Shift Right Logical/Arithmetically (depending on F7)
    
    // LOAD F3
    F3_LB   = 0b000  ,
    F3_LH   = 0b001  ,
    F3_LW   = 0b010  ,
    F3_LBU  = 0b110  ,
    F3_LHU  = 0b101  ,
    F3_LWU  = 0b110  ,
    
    // STORE F3    
    F3_SB   = 0b000  ,
    F3_SH   = 0b001  ,
    F3_SW   = 0b010  
     
} funct3_t;

// We locate memory access safely at dmem with regards to current stack-pointer
#define LOAD_STORE_BASE_REG    20     
#define LOAD_STORE_BASE_OFFSET 0 // base offset in bytes 

#define _SELFIE5_RISCV_H_PARSED__
#endif