
#ifndef _SELFIE5_GCODE_H_PARSED__

typedef enum {
	JMP, 
    GJAL,
    GJALR,   
	JEQ ,
	JNE,
	LIMM,
    GADD,
    GXOR,
    GOR,
    GAND,    
    STOREW,
    LOADW,
    GFENCE, // Generic FENCE (For now no effect, but just expected to acts as a harmless NOP)
	GEND, 
	NUM_GCODES
} selfie5_gcode_cmd_t;


bool_t is_type_alu(selfie5_gcode_cmd_t gcode_type) ;

char * get_gcode_type_str(selfie5_gcode_cmd_t selfie5_gcode_cmd) ;

#define _SELFIE5_GCODE_H_PARSED__
#endif