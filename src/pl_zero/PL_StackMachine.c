#include <stdio.h>
#include <stdlib.h>

#include "PL_Defs.h"
#include "PL_StackMachine.h"
#include <string/SStr.h>

void print_binary(int num);

PMachine* new_pmachine(FILE* input, FILE* output){
    PMachine* new = malloc(sizeof(PMachine));
    new->sp = new->pc = new->ir = 0;
    new->bp = 1;
    new->L = 0;
    int i;
    for(i=0;i<MAX_STACK_HEIGHT;++i)
        new->stack[i]=0;
    for(i=0;i<MAX_CODE_LENGTH;++i)
        new->program[i]=NULL;

    int op, l, m;
    fscanf(input, "%d", &op);
    fscanf(input, "%d", &l);
    fscanf(input, "%d", &m);
    for(i=0;i<MAX_CODE_LENGTH&&!feof(input);++i)
    {
        new->program[i] = new_instruction(i, op, l, m);
        fscanf(input, "%d", &op);
        fscanf(input, "%d", &l);
        fscanf(input, "%d", &m);
    }

    new->output = output;

    print_code_pmn(new);

    return new;
}

void delete_pmachine(PMachine* pmn)
{
    fclose(pmn->output);
    int i;
    for(i=0;i<MAX_CODE_LENGTH;++i)
        destroy_instruction(pmn->program[i]);
    free(pmn);
}

Code* new_instruction(int ln, int op, int L, int M)
{
    Code* new = malloc(sizeof(Code));
    new->line = ln;
    new->op = op;
    new->L = L;
    new->M = M;

    return new;
}
void destroy_instruction(Code* inst)
{
    free (inst);
}

bool execute_pmn(PMachine* pmn)
{


    //IR
    Code* current = pmn->program[pmn->pc];

    if(pmn->program[pmn->pc] == NULL)
    {
        //stacktrace_pmn(pmn);
        return false;
    }

    string instruction = lookup_opcode(current);
    fprintf(pmn->output, "%4d %5s %3d %6d %6d %6d %6d     ", 
            current->line, 
            instruction, 
            current->L, 
            current->M, 
            current->op == 11 ? 0 : pmn->pc, 
            current->op == 11 ? 0 : pmn->bp, 
            current->op == 11 ? 0 : pmn->sp);
    destroy_sstr(instruction);

    pmn->ir = (current->op << 28) | (current->L << 30) | (current->M);

    pmn->pc += 1;
    switch(current->op)
    {
        case 1: //LIT
            pmn->sp += 1;
            pmn->stack[pmn->sp] = current->M;
            break;
        case 2: //OPR
            if(current->M == 0)
            {
                pmn->sp = pmn->bp - 1;
                pmn->pc = pmn->stack[pmn->sp + 4]; 
                pmn->bp = pmn->stack[pmn->sp + 3];
            }
            else
            {
                alu_operation(pmn, current);
            }
            break;
        case 3: //LOD
            pmn->sp += 1;
            pmn->stack[pmn->sp] = pmn->stack[pmn_base(pmn, current->L)+current->M];
            break;
        case 4: //STO
            pmn->stack[pmn_base(pmn, current->L)+current->M] = pmn->stack[pmn->sp];
            pmn->sp -= 1;
            break;
        case 5: //CAL
            pmn->stack[pmn->sp + 1] = 0;
            pmn->stack[pmn->sp + 2] = pmn_base(pmn, current->L);
            pmn->stack[pmn->sp + 3] = pmn->bp;
            pmn->stack[pmn->sp + 4] = pmn->pc;
            pmn->bp = pmn->sp + 1;
            pmn->pc = current->M;
            break;
        case 6: //INC
            pmn->sp += current->M;
            break;
        case 7: //JMP
            pmn->pc = current->M;
            break;
        case 8: //JPC
            if(pmn->stack[pmn->sp] == 0)
            {
                pmn->pc = current->M;
            }
            pmn->sp -= 1;
            break;
        case 9: //SIO
            switch(current->M)
            {
                case 2:
                    printf("%c", pmn->stack[pmn->sp]);
                    break;
                case 3:
                    printf("%x", pmn->stack[pmn->sp]);
                    break;
                case 4:
                    printf("%o", pmn->stack[pmn->sp]);
                    break;
                case 5:
                    {
                        print_binary(pmn->stack[pmn->sp]);
                    }
                    break;
                case 1:
                    printf("%d", pmn->stack[pmn->sp]);
            }
            if(current->L == 1)
            {
                printf("\n");
            }
            else
            {
                pmn->sp -= 1;
            }
            break;
        case 10: //SIO
            pmn->sp += 1;
            switch(current->M)
            {
                case 2:
                    scanf("%c", (char*)&(pmn->stack[pmn->sp]));
                    break;
                case 3:
                    {
                        char buffer[current->L+2];
                        memset(buffer, 0, current->L+2);
                        scanf("%s", buffer);
                        for(int i=current->L+1;i>=0;--i)
                        {
                            pmn->stack[pmn->sp] = buffer[i];
                            pmn->sp += 1;
                        }
                        pmn->sp -= 1;
                    }
                    break;
                default:
                    scanf("%d", &(pmn->stack[pmn->sp]));
            }
            break;
        case 11: //SIO
            pmn->sp = 0;
            pmn->bp = 0;
            pmn->pc = 0;
            stacktrace_pmn(pmn, pmn->bp, -1);
            fprintf(pmn->output, "\n");
            return false;
            break;
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        case 12: // LODX
            pmn->stack[pmn->sp] = pmn->stack[pmn_base(pmn, current->L)+current->M+pmn->stack[pmn->sp]];
            break;
        case 13: // STOX
            pmn->stack[pmn_base(pmn, current->L)+current->M+pmn->stack[pmn->sp]] = pmn->stack[pmn->sp-1];
            pmn->sp -= 2;
            break;
        case 14: // ADR
            pmn->sp += 1;
            pmn->stack[pmn->sp] = pmn_base(pmn, current->L)+current->M;
            break;
        case 15: // ADRX
            pmn->stack[pmn->sp] = pmn_base(pmn, current->L)+current->M + pmn->stack[pmn->sp];
            break;
        case 16: // GET
            pmn->stack[pmn->sp] = pmn->stack[pmn->stack[pmn->sp]];
            break;
        case 17: // PUT
            pmn->stack[pmn->stack[pmn->sp]] = pmn->stack[pmn->sp-1];
            pmn->sp -= 2;
            break;
#endif
        default:
            fprintf(stderr, "FATAL:\n\nError parsing instruction on line %d\n%03d %03d %03d\nTerminating...\n", 
                    pmn->pc,
                    current->op,
                    current->L,
                    current->M);
            exit(1);
    }

    stacktrace_pmn(pmn, pmn->bp, pmn->sp);
    fprintf(pmn->output, "\n");
    return true;
}

void alu_operation(PMachine* pmn, Code* aluop)
{
    switch(aluop->M)
    {
        case 1:
            pmn->stack[pmn->sp] = -1*(pmn->stack[pmn->sp]);
            break;
        case 2:
            pmn->stack[pmn->sp-1] += pmn->stack[pmn->sp];
            pmn->sp -= 1;
            break;
        case 3:
            pmn->stack[pmn->sp-1] -= pmn->stack[pmn->sp];
            pmn->sp -= 1;
            break;
        case 4:
            pmn->stack[pmn->sp-1] *= pmn->stack[pmn->sp];
            pmn->sp -= 1;
            break;
        case 5:
            pmn->stack[pmn->sp-1] /= pmn->stack[pmn->sp];
            pmn->sp -= 1;
            break;
        case 6:
            pmn->stack[pmn->sp] = (pmn->stack[pmn->sp]) % 2 == 1 ? 1 : 0;
            break;
        case 7:
            pmn->stack[pmn->sp-1] %= pmn->stack[pmn->sp];
            pmn->sp -= 1;
            break;
        case 8:
            pmn->stack[pmn->sp-1] = (pmn->stack[pmn->sp-1]) == (pmn->stack[pmn->sp]) ? 1 : 0;
            pmn->sp -= 1;
            break;
        case 9:
            pmn->stack[pmn->sp-1] = (pmn->stack[pmn->sp-1]) != (pmn->stack[pmn->sp]) ? 1 : 0;
            pmn->sp -= 1;
            break;
        case 10:
            pmn->stack[pmn->sp-1] = (pmn->stack[pmn->sp-1]) < (pmn->stack[pmn->sp]) ? 1 : 0;
            pmn->sp -= 1;
            break;
        case 11:
            pmn->stack[pmn->sp-1] = (pmn->stack[pmn->sp-1]) <= (pmn->stack[pmn->sp]) ? 1 : 0;
            pmn->sp -= 1;
            break;
        case 12:
            pmn->stack[pmn->sp-1] = (pmn->stack[pmn->sp-1]) > (pmn->stack[pmn->sp]) ? 1 : 0;
            pmn->sp -= 1;
            break;
        case 13:
            pmn->stack[pmn->sp-1] = (pmn->stack[pmn->sp-1]) >= (pmn->stack[pmn->sp]) ? 1 : 0;
            pmn->sp -= 1;
            break;
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        case 14:
            pmn->stack[pmn->sp-1] = (pmn->stack[pmn->sp-1]) && (pmn->stack[pmn->sp]) ? 1 : 0;
            pmn->sp -= 1;
            break;
        case 15:
            pmn->stack[pmn->sp-1] = (pmn->stack[pmn->sp-1]) || (pmn->stack[pmn->sp]) ? 1 : 0;
            pmn->sp -= 1;
            break;
        case 16:
            pmn->stack[pmn->sp] = pmn->stack[pmn->sp] == 1 ? 0 : 1;
            break;
#endif
        default:
            fprintf(stderr, "FATAL:\n\nError parsing instruction on line %d | %03d %03d %03d\n\nTerminating...\n", 
                    pmn->pc,
                    aluop->op,
                    aluop->L,
                    aluop->M);
            exit(1);
    }
}

int pmn_base(PMachine* pmn, int l)
{
    int base=pmn->bp;
    for(;l>0;--l)
    {
        base = pmn->stack[base+1];
    }

    return base;
}

string lookup_opcode(Code* c)
{
    string opname = NULL;
    switch(c->op)
    {
        case 1:
            opname = new_sstr("lit");
            break;
        case 2:
            opname = new_sstr("opr");
            break;
        case 3:
            opname = new_sstr("lod");
            break;
        case 4:
            opname = new_sstr("sto");
            break;
        case 5:
            opname = new_sstr("cal");
            break;
        case 6:
            opname = new_sstr("inc");
            break;
        case 7:
            opname = new_sstr("jmp");
            break;
        case 8:
            opname = new_sstr("jpc");
            break;
        case 9:
        case 10:
        case 11:
            opname = new_sstr("sio");
            break;
        case 12:
            opname = new_sstr("lodx");
            break;
        case 13:
            opname = new_sstr("stox");
            break;
        case 14:
            opname = new_sstr("adr");
            break;
        case 15:
            opname = new_sstr("adrx");
            break;
        case 16:
            opname = new_sstr("get");
            break;
        case 17:
            opname = new_sstr("put");
            break;
    }
    return opname;
}

void print_code_pmn(PMachine* pmn)
{
    int i;
    FILE* acode = fopen("acode.txt", "w");
    fprintf(acode, "  Line  OP  %2s %6s\n", "L", "M");
    for(i=0;i<MAX_CODE_LENGTH&&pmn->program[i]!=NULL;++i)
    {
        string opname = lookup_opcode(pmn->program[i]);
        fprintf(acode, "%4d  %4s  %2d %6d\n", i, opname, pmn->program[i]->L, pmn->program[i]->M);
        destroy_sstr(opname);
    }
    fclose(acode);

    fprintf(pmn->output, "\n                           pc     bp     sp     stack\n"
                             "  Initial Values                  1      0      0  \n");
}

void stacktrace_pmn(PMachine* pmn, int base, int end)
{
    if(base > 1)
    {
        stacktrace_pmn(pmn, pmn->stack[base+2], base-1);
        if( base <= end )
            fprintf(pmn->output, " | ");
    }

    int i;
    for(i=base;i<=end;++i)
        fprintf(pmn->output, "%2d ", pmn->stack[i]);

    return;
}

void print_binary(int num)
{
    if (num) {
        print_binary(num >> 1);
        putc((num & 1) ? '1' : '0', stdout);
    }
}
