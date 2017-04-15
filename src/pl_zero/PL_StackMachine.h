#ifndef STACK_MACHINE_INCLUDED
#define STACK_MACHINE_INCLUDED

#include <stdio.h>
#include <common/Common.h>
#include <string/SStr.h>

#include "PL_Defs.h"

#define MAX_STACK_HEIGHT 100000
#define MAX_CODE_LENGTH 15000
#define MAX_LEXI_LVLS 3

typedef struct pmn PMachine;
typedef struct Code Code;

typedef struct pmn
{
    FILE* output;
    int sp,
        ir,
        bp,
        pc,
        L;
    int stack[MAX_STACK_HEIGHT];
    Code* program[MAX_CODE_LENGTH];
}PMachine;

typedef struct Code
{
    int line,
        op,
        L,
        M;
}Code;

PMachine* new_pmachine(FILE* input, FILE* output);
void delete_pmachine(PMachine* pmn);

Code* new_instruction(int ln, int op, int L, int M);
void destroy_instruction(Code* inst);

bool execute_pmn(PMachine* pmn);
int pmn_base(PMachine* pmn, int l);
string lookup_opcode(Code* c);
void alu_operation(PMachine* pmn, Code* aluop);
void stacktrace_pmn(PMachine* pmn, int base, int end);
void print_code_pmn(PMachine* pmn);

#endif
