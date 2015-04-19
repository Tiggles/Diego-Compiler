#include "irInstructions.h"
#include <stdlib.h>
#include <string.h>
/*
IR_LINE *make_line_empty() {
	IR_LINE *new_line;
	new_line = (IR_LINE *) malloc(sizeof(IR_LINE));
	new_line->kind = empty_line;
	new_line->instruction = NULL;
	new_line->label = NULL;
	return new_line;
}

IR_LINE *make_line_instruction(IR_INSTRUCTION *instr, char *label) {
	IR_LINE *new_line;
	new_line = (IR_LINE *) malloc(sizeof(IR_LINE));
	new_line->kind = instruction_line;
	new_line->instruction = instr;
	if(label != NULL) { 
		new_line->label = (char*) calloc(strlen(label)+1 , sizeof(char));
		sprintf(new_line->label,"%s",label);
	}else{
		new_line->label = NULL;
	}
	return new_line;
}

IR_LINE *make_line_label(char *label) {
	IR_LINE *new_line;
	new_line = (IR_LINE *) malloc(sizeof(IR_LINE));
	new_line->kind = label_line;
	new_line->instruction = NULL;
	new_line->label = calloc(strlen(label)+1 , sizeof(char));
	sprintf(new_line->label,"%s",label);
	return new_line;
}
*/
IR_INSTRUCTION *make_instruction_movl(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = movl;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_call(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = call;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}


IR_INSTRUCTION *make_instruction_pushl(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = pushl;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_popl(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = popl;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_addl(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = addl;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_subl(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = subl;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_div(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = divl;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_mul(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = mul;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_xor(ARGUMENT *arg1, ARGUMENT *arg2) {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = xor;
	new_instruction->arg1 = arg1;
	new_instruction->arg2 = arg2;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_globl() {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = globl;
	new_instruction->arg1 = NULL;
	new_instruction->arg2 = NULL;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_string() {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = string;
	new_instruction->arg1 = NULL;
	new_instruction->arg2 = NULL;
	return new_instruction;
}

IR_INSTRUCTION *make_instruction_ret() {
	IR_INSTRUCTION *new_instruction;
	new_instruction = (IR_INSTRUCTION *) malloc(sizeof(IR_INSTRUCTION));
	new_instruction->op_code = ret;
	new_instruction->arg1 = NULL;
	new_instruction->arg2 = NULL;
	return new_instruction;	
}

ARGUMENT *make_argument_address() {
	ARGUMENT *new_argument;
	new_argument = (ARGUMENT *) malloc(sizeof(ARGUMENT));
	new_argument->kind = address_arg;
	return new_argument;
}

ARGUMENT *make_argument_register(REGISTERS reg, char *name) {
	ARGUMENT *new_argument;
	new_argument = (ARGUMENT *) malloc(sizeof(ARGUMENT));
	new_argument->kind = register_arg;
	new_argument->reg = reg;
	new_argument->charConst = calloc(strlen(name)+1,sizeof(char));
	new_argument->charConst = name;
	return new_argument;
}

ARGUMENT *make_argument_label(char *label) {
	ARGUMENT *new_argument;
	new_argument = (ARGUMENT *) malloc(sizeof(ARGUMENT));
	new_argument->kind = label_arg;
	new_argument->label = calloc(strlen(label)+1,sizeof(char));
	new_argument->label = label;
	return new_argument;
}

ARGUMENT *make_argument_constant(int i) {
	ARGUMENT *new_argument;
	new_argument = (ARGUMENT *) malloc(sizeof(ARGUMENT));
	new_argument->kind = constant_arg;
	new_argument->intConst = i;
	return new_argument;
}

ARGUMENT *make_argument_tempregister(int i){
	ARGUMENT *new_argument;
	new_argument = (ARGUMENT *) malloc(sizeof(ARGUMENT));
	new_argument->kind = tempreg_arg;
	new_argument->type = TEMP;
	new_argument->tempid = i;
	return new_argument;
}
/*
TEMP *make_temp_register(int id) {
	TEMP *new_temp;
	new_temp = (TEMP *) malloc(sizeof(TEMP));
	new_temp->id = id;
	new_temp->kind = register_temp;
	return new_temp;	
}

TEMP *make_temp_spilled(int id) {
	TEMP *new_temp;
	new_temp = (TEMP *) malloc(sizeof(TEMP));
	new_temp->id = id;
	new_temp->kind = spilled_temp;
	return new_temp;
}

TEMP *make_temp_virtual(int id) {
	TEMP *new_temp;
	new_temp = (TEMP *) malloc(sizeof(TEMP));
	new_temp->id = id;
	new_temp->kind = virtual_temp;
	return new_temp;
}
*/