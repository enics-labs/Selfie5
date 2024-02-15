
#ifndef _SELFIE5_FLOW_H_PARSED__

typedef enum
{
	GOTO,
	STEP,
    NUM_RAND_FLOW_CMD,
    FLOW_FINISH = NUM_RAND_FLOW_CMD,
	NUM_FLOW_CMD

} selfie5_flow_cmd_t;

void create_flow(void) ;

void print_instruction_code_flow  (void);

#define _SELFIE5_FLOW_H_PARSED__
#endif