#include <stdlib.h>
#include <string/SStr.h>

#include "PL_Defs.h"
#include "PL_Parser.h"
#include "PL_Generator.h"

typedef enum {VAR_IDENT = 0, CONST_IDENT, PROC_IDENT} id_type;

CodePage *new_codepage()
{
    CodePage *new = malloc(sizeof(CodePage));
    new->program = NULL;
    new->instct = 0;
    new->L      = 0;
    new->M      = 4;

    new->cur_proc  = new_sstr("__main");
    Pl0Ident mainsym = (Pl0Ident){ .ident_type      = PROC_IDENT,
                                      .param_count  = 0,
                                      .var_count    = 0,
                                      .L            = 0,
                                      .M            = 0,
                                      .value        = 0,
                                      .name         = new->cur_proc};
    Pl0Ident returnsym = (Pl0Ident){ .ident_type    = VAR_IDENT,
                                      .param_count  = 0,
                                      .var_count    = 0,
                                      .L            = -1,
                                      .M            = 0,
                                      .value        = 0,
                                      .name         = new_sstr("return")};
    new->SymbolTable    = malloc(sizeof(Pl0Ident)*2);
    new->symct          = 2;
    new->SymbolTable[0] = mainsym;
    new->SymbolTable[1] = returnsym;
    new->ProcTable      = malloc(sizeof(Pl0Ident));
    new->procct         = 1;
    new->ProcTable[0]   = mainsym;

    return new;
}

// string `proc` must be passed from corresponding Pl0Ident obj
unsigned int find_proc_end(CodePage *page, string proc)
{
    for(int i = page->procct - 1;i>=0;--i)
    {
        if(proc == page->ProcTable[i].name)
        {
            return page->ProcTable[i].M;
        }
    }
    return -1;
}
unsigned int find_proc_begin(CodePage *page, string proc)
{
    for(int i = page->procct - 1;i>=0;--i)
    {
        if(proc == page->ProcTable[i].name)
        {
            return page->ProcTable[i].value;
        }
    }
    return -1;
}

Pl0Ident *lookup_ident(CodePage *page, string ident)
{

    for(int i = page->symct - 1;i>=0;--i)
    {
        if(cmp_sstr(ident, page->SymbolTable[i].name) == -1)
        {
            return &page->SymbolTable[i];
        }
    }

    return NULL;
}

void delete_instruction(CodePage *page, unsigned int loc)
{
    if(loc > page->instct)
    {
        fprintf(stderr, "Invalid LOC index, %u\n"
                        "Current Procedure: %s\n", loc, page->cur_proc);
        exit(1);
    }

    if(page->program == NULL)
    {
        return;
    }
    else
    {
        Instruction *shrink_prog = malloc(sizeof(Instruction) * (page->instct - 1));
        for(unsigned int i=0;i<loc;++i)
        {
            shrink_prog[i] = page->program[i];
            if(shrink_prog[i].op == 7 || shrink_prog[i].op == 8 || shrink_prog[i].op == 5)
            {
                if(shrink_prog[i].M >= loc)
                    shrink_prog[i].M -= 1;
            }
        }
        for(unsigned int i=loc;i<page->instct-1;++i)
        {
            shrink_prog[i] = page->program[i+1];
            if(shrink_prog[i].op == 7 || shrink_prog[i].op == 8 || shrink_prog[i].op == 5)
            {
                if(shrink_prog[i].M >= loc)
                    shrink_prog[i].M -= 1;
            }
        }
        
        free(page->program);
        page->program = shrink_prog;
        page->instct -= 1;
    }

    /* bool found = false; */
    /* for(unsigned int i=0;i<page->procct;++i) */
    /* { */
    /*     if(found) */
    /*     { */
    /*         page->ProcTable[i].value += 1; */
    /*         page->ProcTable[i].M += 1; */
    /*     } */
            
    /*     if(page->cur_proc == page->ProcTable[i].name) */
    /*     { */
    /*         found = true; */
    /*         page->ProcTable[i].M += 1; */
    /*     } */
    /* } */
}

void add_instruction(CodePage *page, Instruction inst, unsigned int loc)
{
    if(loc > page->instct)
    {
        fprintf(stderr, "Invalid LOC index, %u\n"
                        "Current Procedure: %s\n", loc, page->cur_proc);
        exit(1);
    }

    /* if(inst.op == 11) */
    /*     printf("Adding SIO HALT line %d\n", */ 
    /*            loc); */
    

    if(page->program == NULL)
    {
        page->program = malloc(sizeof(Instruction));
        page->program[page->instct] = inst;
        page->instct += 1;
    }
    else
    {
        Instruction *expand_prog = malloc(sizeof(Instruction) * (page->instct + 1));
        for(unsigned int i=0;i<loc;++i)
        {
            expand_prog[i] = page->program[i];
            if(expand_prog[i].op == 7 || expand_prog[i].op == 8 || expand_prog[i].op == 5)
            {
                if(expand_prog[i].M >= loc)
                    expand_prog[i].M += 1;
            }
        }
        expand_prog[loc] = inst;
        for(unsigned int i=loc+1;i<page->instct+1;++i)
        {
            expand_prog[i] = page->program[i-1];
            if(expand_prog[i].op == 7 || expand_prog[i].op == 8 || expand_prog[i].op == 5)
            {
                if(expand_prog[i].M >= loc)
                    expand_prog[i].M += 1;
            }
        }
        
        free(page->program);
        page->program = expand_prog;
        page->instct += 1;
    }

    bool found = false;
    for(unsigned int i=0;i<page->procct;++i)
    {
        if(found)
        {
            page->ProcTable[i].value += 1;
            page->ProcTable[i].M += 1;
        }
            
        if(page->cur_proc == page->ProcTable[i].name)
        {
            found = true;
            page->ProcTable[i].M += 1;
        }
    }
}

void add_identifier(CodePage *page, short type, string ident, int value)
{
    Pl0Ident *chk = lookup_ident(page, ident);
    if(chk != NULL && chk->L == page->L)
    {
        fprintf(stderr, "Error, redefinition of identifier or ambiguous identifier \'%s\'\n", ident);
        exit(1);
    }

    if(type == PROC_IDENT)
    {
        if(page->ProcTable == NULL)
        {
            page->ProcTable = malloc(sizeof(Pl0Ident));
        }
        else
        {
            page->ProcTable = realloc(page->ProcTable, sizeof(Pl0Ident)*(page->procct + 1));
        }

        page->ProcTable[page->procct] = (Pl0Ident){.ident_type = type,
                                                .L = 0,
                                                .var_count = 0,
                                                .param_count = 0,
                                                .M = page->ProcTable[page->procct-1].M, 
                                                .value = page->ProcTable[page->procct-1].M, 
                                                .name = ident};
        page->procct += 1;
    }

    if(page->SymbolTable == NULL)
    {
        page->SymbolTable = malloc(sizeof(Pl0Ident));
    }
    else
    {
        page->SymbolTable = realloc(page->SymbolTable, sizeof(Pl0Ident)*(page->symct + 1));
    }
    
    page->SymbolTable[page->symct] = (Pl0Ident){.ident_type = type,
                                                .L = page->L,
                                                .M = page->M, 
                                                .var_count = 0,
                                                .param_count = 0,
                                                .value = value, 
                                                .name = ident};

    switch(type)
    {
        case PROC_IDENT:
            fprintf(page->symboltable, "%11s %5s %5d %5d\n", ident, "proc", page->L, 1);
            break;
        case VAR_IDENT:
            fprintf(page->symboltable, "%11s %5s %5d %5d\n", ident, "var", page->L, page->M);
            break;
        case CONST_IDENT:
            fprintf(page->symboltable, "%11s %5s %5d %5d\n", ident, "const", page->L, page->M);
            break;
        default:
            fprintf(stderr, "Unexpected error, asploding now...\n");
            exit(1);
    }

    page->symct += 1;
    if(type == 0)
        page->M += 1;
}

void trim_symbol_table(CodePage *page)
{
    Pl0Ident *newtable = NULL;
    unsigned int newct = 0;

    for(unsigned int i=0;i<page->symct;++i)
    {
        if(page->SymbolTable[i].L != page->L)
        {
            if(newtable == NULL)
            {
                newtable = malloc(sizeof(Pl0Ident));
                newtable[newct] = page->SymbolTable[i];
            }
            else
            {
                newtable = realloc(newtable, sizeof(Pl0Ident) * (newct + 1));
                newtable[newct] = page->SymbolTable[i];
            }
            ++newct;
        }
    }
    
    free(page->SymbolTable);
    page->SymbolTable = newtable;
    page->symct = newct;
    page->L -= 1;
    page->M  = 4;
}

void generateConstDec(CodePage*,ParseTree*);
void generateVarDec(CodePage*,ParseTree*);
void generateProcDec(CodePage*,ParseTree*);
void generateBlock(CodePage*,ParseTree*);
void generateStatement(CodePage*,ParseTree*);
void generateExpression(CodePage*,ParseTree*);
void generateTerm(CodePage*,ParseTree*);
void generateFactor(CodePage*,ParseTree*);
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
void generateExpression0(CodePage*,ParseTree*);
void generateRelation(CodePage*,ParseTree*);
void generateRelationA(CodePage*,ParseTree*);
#else
void generateCondition(CodePage*,ParseTree*);
#endif

void generateConstDec(CodePage *code, ParseTree *decl)
{
    for(unsigned int i=0;i<decl->childct;i+=2)
    {
        add_identifier(code, CONST_IDENT, decl->children[i]->strval, decl->children[i+1]->intval);
    }
}

void generateVarDec(CodePage *code, ParseTree *decl)
{
    for(unsigned int i=0;i<decl->childct;++i)
    {
        add_identifier(code, VAR_IDENT, decl->children[i]->strval, 0);
        if(decl->children[i]->childct > 0)
        {
            if(decl->children[i]->children[0]->intval < 1)
            {
                fprintf(stderr, "Error, cannot have array of size less than 1\n");
                exit(1);
            }

            Pl0Ident *arrayvar = lookup_ident(code, decl->children[i]->strval);
            arrayvar->param_count = (decl->children[i]->children[0]->intval);

            code->M += (decl->children[i]->children[0]->intval) - 1;
        }
    }
}

void generateProcDec(CodePage *code, ParseTree *decl)
{
    bool jmpblk = false;
    string proc = NULL;
    for(unsigned int i=0,pct=0;i<decl->childct;++i)
    {
        if(decl->children[i]->type == Ident && !jmpblk)
        {
            add_identifier(code, PROC_IDENT, decl->children[i]->strval, code->instct);
            proc = decl->children[i]->strval;
            jmpblk = true;
        }
        else if(decl->children[i]->type != Ident && jmpblk
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
                && decl->children[i]->type != Address 
#endif
                )
        {
            Pl0Ident *cproc = lookup_ident(code, proc);
            cproc->param_count = pct;
            jmpblk = false;
            pct = 0;
        }
        else 
        {
            pct += 1;
        }
    }

    string me = code->cur_proc;
    jmpblk = false;
    for(unsigned int i=0;i<decl->childct;++i)
    {
        if(decl->children[i]->type == Ident && !jmpblk)
        {
            code->L += 1;
            code->M = 4;
            code->cur_proc = decl->children[i]->strval;
            jmpblk = true;
        }
        else if(decl->children[i]->type != Ident && jmpblk
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
                && decl->children[i]->type != Address 
#endif
               )
        {

            generateBlock(code, decl->children[i]);
            Instruction inst = (Instruction){.op = OPR, .L = 0, .M = RET, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
            trim_symbol_table(code);
            jmpblk = false;
        }
        else
        {
            add_identifier(code, VAR_IDENT, decl->children[i]->strval, 0);
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            if(decl->children[i]->type == Address)
            {
                Pl0Ident *ptr_param = lookup_ident(code, decl->children[i]->strval);
                ptr_param->var_count = 1; // Denote pointer identifier
            }
#endif
        }
    }
    code->cur_proc = me;
}

void generateBlock(CodePage *code, ParseTree *block)
{
    size_t varcount = 0;

    for(unsigned int i=0;i<block->childct;++i)
    {
        if(block->children[i]->type == Var_decl)
        {
            for(unsigned int j=0;j<block->children[i]->childct;++j)
            {
                varcount += block->children[i]->children[j]->childct > 0 
                            ? block->children[i]->children[j]->children[0]->intval : 1;
            }
        }
    }

    Pl0Ident *procgen = lookup_ident(code, code->cur_proc);
    procgen->var_count = varcount;
    
    Instruction inst = (Instruction){.op = INC, .L = 0, .M = 4+varcount+procgen->param_count, .destination = NULL};
    add_instruction(code, inst, find_proc_end(code, code->cur_proc));

    for(unsigned int i=0;i<block->childct;++i)
    {
        switch(block->children[i]->type)
        {
            case Const_decl:
                generateConstDec(code, block->children[i]);
                break;
            case Var_decl:
                generateVarDec(code, block->children[i]);
                break;
            case Proc_decl:
                generateProcDec(code, block->children[i]);
                break;
            default:
                generateStatement(code, block->children[i]);
        }
    }
}

void generateStatement(CodePage *code, ParseTree *stmnt)
{
    if(stmnt->type == StatementList)
    {
        for(unsigned int i=0;i<stmnt->childct;++i)
        {
            generateStatement(code, stmnt->children[i]);
            if(stmnt->children[i]->type == Return)
                break;
        }
    }
    else if(stmnt->type == While)
    {
        Instruction skip_while, while_loop;

        size_t while_begin = find_proc_end(code, code->cur_proc);
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        generateExpression(code, stmnt->children[0]);
#else
        generateCondition(code, stmnt->children[0]);
#endif
        size_t while_skip  = find_proc_end(code, code->cur_proc);

        generateStatement(code, stmnt->children[1]);

        while_loop = (Instruction){.op = JMP, .L = 0, .M = while_begin};
        add_instruction(code, while_loop, find_proc_end(code, code->cur_proc));
        
        Instruction while_end = (Instruction){.op = JMP, .L = 0, .M = find_proc_end(code, code->cur_proc), .destination = NULL};
        add_instruction(code, while_end, find_proc_end(code, code->cur_proc));

        skip_while = (Instruction){.op = JPC, .L = 0, .M = find_proc_end(code, code->cur_proc), .destination = NULL};
        add_instruction(code, skip_while, while_skip);

    }
    else if(stmnt->type == If)
    {
        int jmp_to_else_array[stmnt->childct],
            index = 0;
        size_t skip_if_loc;
        for(unsigned int i=0;i<stmnt->childct;++i)
        {
            jmp_to_else_array[i] = -1;
        }
        Instruction jmp_to_else,jmp_past_else,bogus_jmp;

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        for(;index < (int)stmnt->childct && stmnt->children[index]->type == Or;index+=2)
#else
        for(;index < (int)stmnt->childct && (stmnt->children[index]->type == Odd
                                            || stmnt->children[index]->type == Condition );index+=2)
#endif
        {
            if(index > 0)
            {
                bogus_jmp = (Instruction){.op = JMP, .L = 0, .M = find_proc_end(code, code->cur_proc), .destination = NULL};
                add_instruction(code, bogus_jmp, find_proc_end(code, code->cur_proc));

                jmp_to_else = (Instruction){.op = JPC, .L = 0, .M = jmp_to_else_array[index-2]+1, .destination = NULL};
                add_instruction(code, jmp_to_else, skip_if_loc);
            }

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            generateExpression(code, stmnt->children[index]);
#else
            generateCondition(code, stmnt->children[index]);
#endif
            skip_if_loc = find_proc_end(code, code->cur_proc);

            generateStatement(code, stmnt->children[index+1]);
            jmp_to_else_array[index] = find_proc_end(code, code->cur_proc);
        }

        if(index < (int)stmnt->childct)
        {
            generateStatement(code, stmnt->children[index]);
        }

        bogus_jmp = (Instruction){.op = JMP, .L = 0, .M = find_proc_end(code, code->cur_proc), .destination = NULL};
        add_instruction(code, bogus_jmp, find_proc_end(code, code->cur_proc));

        jmp_to_else = (Instruction){.op = JPC, .L = 0, .M = jmp_to_else_array[index-2]+1, .destination = NULL};
        add_instruction(code, jmp_to_else, skip_if_loc);

        for(unsigned int i=0,ct=0;i<stmnt->childct;++i)
        {
            /* printf("#%d --> %d\n", i, jmp_to_else_array[i]); */
            if(jmp_to_else_array[i] > 0)
            {
                jmp_past_else = (Instruction){.op = JMP, .L = 0, .M = find_proc_end(code, code->cur_proc), .destination = NULL};
                add_instruction(code, jmp_past_else, jmp_to_else_array[i]+1+ct);
                ct++;
            }
        }
        
    }
    else if(stmnt->type == Call)
    {
        Pl0Ident *proc = lookup_ident(code, stmnt->children[0]->strval);
        
        if(proc == NULL)
        {
            fprintf(stderr, "Fatal Error:\nUndeclared identifier \'%s\'\n", 
                        stmnt->children[0]->strval);
            exit(1);
        }
        else if(proc->ident_type != PROC_IDENT)
        {
            fprintf(stderr, "Fatal Error:\nCall to non-procedure identifier \'%s\' is meaningless\n", 
                        stmnt->children[0]->strval);
            exit(1);
        }

        Instruction inst;

#ifdef PL_PARAM_EXTENSION_SUPPORT

        if((int)stmnt->childct - 1 != proc->param_count)
        {
            fprintf(stderr, "Fatal Error:\nCall to procedure \'%s\' does not match procedure declaration\n", 
                    stmnt->children[0]->strval);
            exit(1);
        }

        inst = (Instruction){.op = INC, .L = 0, .M = 4, .destination = NULL};
        add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        for(unsigned int i=1;i<stmnt->childct;++i)
        {
            generateExpression(code, stmnt->children[i]);
        }
        unsigned int offset = (4+proc->param_count);
        inst = (Instruction){.op = INC, .L = 0, .M = -offset, .destination = NULL};
        add_instruction(code, inst, find_proc_end(code, code->cur_proc));
#endif

        inst = (Instruction){.op = CAL, .L = code->L - proc->L, .M = 0, .destination = proc->name};
        add_instruction(code, inst, find_proc_end(code, code->cur_proc));
    }
    else if(stmnt->type == Assignment)
    {
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        bool derefassign = false;
        if(stmnt->children[0]->type == Address)
        {
            derefassign = true;
            stmnt->children[0] = stmnt->children[0]->children[0];
        }
#endif
        Pl0Ident *var = lookup_ident(code, stmnt->children[0]->strval);

        if(var == NULL)
        {
            fprintf(stderr, "Fatal Error:\nUndeclared identifier \'%s\'\n", 
                        stmnt->children[0]->strval);
            exit(1);
        }
        else if(var->ident_type != VAR_IDENT)
        {
            fprintf(stderr, "Fatal Error:\nCannot assign to a procedure \'%s\'\n", stmnt->children[0]->strval);
            exit(1);
        }

        Instruction inst;
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        if(var->var_count > 0)
        {
            derefassign = true;
        }
        if(stmnt->children[1]->type == StrLiteral)
        {
            if(var->param_count+1 < length_sstr(stmnt->children[1]->strval))
            {
                fprintf(stderr, "Error assigning string value to too small array \'%s\'\n", 
                        var->name);
                exit(1);
            }
            if(stmnt->children[0]->childct > 0 || derefassign)
            {
                fprintf(stderr, "Cannot assign string literal into array subscript of \'%s\'\n",
                        stmnt->children[0]->strval);
                exit(1);
            }
            for(int i=0;i<length_sstr(stmnt->children[1]->strval);++i)
            {
                Instruction inst = (Instruction){.op = LIT, .L = 0, .M = stmnt->children[1]->strval[i], 
                                        .destination = NULL},
                store = (Instruction){.op = STO, .L = code->L - var->L, .M = var->M + i,
                                        .destination = NULL};
                add_instruction(code, inst, find_proc_end(code, code->cur_proc));
                add_instruction(code, store, find_proc_end(code, code->cur_proc));
            }
            return;
        }
        else if(stmnt->children[1]->type == CharLiteral)
        {
            inst = (Instruction){.op = LIT, .L = 0, .M = stmnt->children[1]->strval[0], 
                                                .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
        else
        {
            generateExpression(code, stmnt->children[1]);
        }
#else
        generateExpression(code, stmnt->children[1]);
#endif

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        if(stmnt->children[0]->childct > 0)
        {
            generateExpression(code, stmnt->children[0]->children[0]);
            inst = (Instruction){.op = STOX, .L = code->L - var->L, .M = var->M, .destination = NULL};
            if(derefassign)
            {
                Instruction lod = (Instruction){.op = LOD, .L = code->L - var->L, .M = var->M, .destination = NULL},
                            add = (Instruction){.op = OPR, .L = 0, .M = ADD, .destination = NULL};
                add_instruction(code, lod, find_proc_end(code, code->cur_proc));
                add_instruction(code, add, find_proc_end(code, code->cur_proc));
                inst = (Instruction){.op = PUT, .L = 0, .M = 0, .destination = NULL};
            }
        }
        else
        {
#endif
            inst = (Instruction){.op = STO, .L = code->L - var->L, .M = var->M, .destination = NULL};
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            if(derefassign)
            {
                Instruction lod = (Instruction){.op = LOD, .L = code->L - var->L, .M = var->M, .destination = NULL};
                add_instruction(code, lod, find_proc_end(code, code->cur_proc));
                inst = (Instruction){.op = PUT, .L = 0, .M = 0, .destination = NULL};
            }
        }
#endif
        if(var->L < 0)
        {
            inst.L = 0;
            inst.M = 0;
        }
        add_instruction(code, inst, find_proc_end(code, code->cur_proc)); 
    }
    else if(stmnt->type == Write
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            || stmnt->type == WriteLn
#endif     
            )
    {
        Instruction inst;
#ifndef PL_SYNTAX_EXTENSION_SUPPORT
        Pl0Ident *var = lookup_ident(code, stmnt->children[0]->strval);

        if(var == NULL)
        {
            fprintf(stderr, "Undeclared identifier \'%s\'\n", stmnt->children[0]->strval);
            exit(1);
        }
        else if (var->ident_type == PROC_IDENT)
        {
            fprintf(stderr, "Invalid parameter to Write \'%s\'\n", stmnt->children[0]->strval);
            exit(1);
        }

        if(var->ident_type == VAR_IDENT)
        {
            inst = (Instruction){.op = LOD, .L = code->L - var->L, .M = var->M, .destination = NULL};
        }
        else 
        {
            inst = (Instruction){.op = LIT, .L = 0, .M = var->value, .destination = NULL};
        }
        add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        inst = (Instruction){.op = SIO1, .L = 0, .M = 1, .destination = NULL};
        add_instruction(code, inst, find_proc_end(code, code->cur_proc));
#else
        bool begin_at_zero = false;
        if(stmnt->childct > 0 && 
                stmnt->children[0]->type != Write 
                && stmnt->children[0]->type != WriteLn)
        {
            inst = (Instruction){.op = SIO1, .L = 0, .M = 1, .destination = NULL};
            switch(stmnt->children[0]->type)
            {
                case Char:
                    inst = (Instruction){.op = SIO1, .L = 0, .M = 2, .destination = NULL};
                    stmnt->children[0] = stmnt->children[0]->children[0];
                    break;
                case Hex:
                    inst = (Instruction){.op = SIO1, .L = 0, .M = 3, .destination = NULL};
                    stmnt->children[0] = stmnt->children[0]->children[0];
                    break;
                case Oct:
                    inst = (Instruction){.op = SIO1, .L = 0, .M = 4, .destination = NULL};
                    stmnt->children[0] = stmnt->children[0]->children[0];
                    break;
                case Bin:
                    inst = (Instruction){.op = SIO1, .L = 0, .M = 5, .destination = NULL};
                    stmnt->children[0] = stmnt->children[0]->children[0];
                    break;
                case String:
                    inst = (Instruction){.op = SIO1, .L = 0, .M = 2, .destination = NULL};
                    stmnt->children[0] = stmnt->children[0]->children[0];
                    break;
                case StrLiteral:
                    inst = (Instruction){.op = SIO1, .L = 0, .M = 2, .destination = NULL};
                    break;
                case CharLiteral:
                    inst = (Instruction){.op = SIO1, .L = 0, .M = 2, .destination = NULL};
                    break;
            }

            if(stmnt->children[0]->type == StrLiteral ||
                    stmnt->children[0]->type == CharLiteral)
            {
                string literal = stmnt->children[0]->strval;
                Instruction inst2 = inst;
                for(int i=0;i<length_sstr(literal);++i)
                {
                    inst = (Instruction){.op = LIT, .L = 0, .M = literal[i], .destination = NULL};
                    add_instruction(code, inst, find_proc_end(code, code->cur_proc));
                    add_instruction(code, inst2, find_proc_end(code, code->cur_proc));

                }
            }
            else if(stmnt->children[0]->type == Ident)
            {
                Pl0Ident *var = lookup_ident(code, stmnt->children[0]->strval);

                if(var == NULL || var->ident_type != VAR_IDENT)
                {
                    fprintf(stderr, "Undeclared variable \'%s\'\n", 
                            stmnt->children[0]->strval);
                    exit(1);
                }


                if(var->param_count < 1)
                {
                    fprintf(stderr, "Cannot print non-array variable \'%s\' as string\n",
                            stmnt->children[0]->strval);
                    exit(1);
                }

                for(int i=0;i<var->param_count;++i)
                {
                    Instruction inst2 = (Instruction){.op = LOD, .L = code->L - var->L, .M = var->M + i,
                        .destination = NULL};
                    add_instruction(code, inst2, find_proc_end(code, code->cur_proc));
                    add_instruction(code, inst, find_proc_end(code, code->cur_proc));
                }
            }
            else
            {
                generateExpression(code, stmnt->children[0]);
                add_instruction(code, inst, find_proc_end(code, code->cur_proc));
            }
        }
        else if(stmnt->childct > 0)
        {
            begin_at_zero = true;
        }
        if(stmnt->type == WriteLn)
        {
            inst = (Instruction){.op = SIO1, .L = 1, .M = 0, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
        for(int i= begin_at_zero ? 0 : 1;i<stmnt->childct;++i)
        {
            generateStatement(code, stmnt->children[i]);
        }
#endif
    }
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    else if(stmnt->type == Return)
    {
        Instruction inst;
        if(stmnt->childct > 0)
        {
            generateExpression(code, stmnt->children[0]);
            inst = (Instruction){.op = STO, .L = 0, .M = 0, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }

        inst = (Instruction){.op = OPR, .L = 0, .M = RET, .destination = NULL};
        add_instruction(code, inst, find_proc_end(code, code->cur_proc));
    }
#endif
    else if(stmnt->type == Read)
    {
        Instruction input = (Instruction){.op = SIO2, .L = 0, .M = 1, .destination = NULL},
                    store;
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        if(stmnt->children[0]->type == ReadChar)
        {
            input.M = 2;
            stmnt->children[0] = stmnt->children[0]->children[0];
        }
        else if(stmnt->children[0]->type == ReadString)
        {
            input.M = 3;
            stmnt->children[0] = stmnt->children[0]->children[0];
            if(stmnt->children[0]->childct > 0)
            {
                fprintf(stderr, "Cannot read string into array subscript of \'%s\'\n", 
                        stmnt->children[0]->strval);
                exit(1);
            }
        }
#endif
        Pl0Ident *var = lookup_ident(code, stmnt->children[0]->strval);

        if(var == NULL)
        {
            fprintf(stderr, "Undeclared identifier \'%s\'\n", stmnt->children[0]->strval);
            exit(1);
        }
        else if (var->ident_type != VAR_IDENT)
        {
            fprintf(stderr, "Cannot read into non-variable identifer \'%s\'\n", stmnt->children[0]->strval);
            exit(1);
        }

        if(input.M == 3)
        {
            if(var->param_count < 1)
            {
                fprintf(stderr, "Error, cannot read string into non array variable \'%s\'\n",
                        var->name);
                exit(1);
            }
            input.L = var->param_count;
        }
        
        add_instruction(code, input, find_proc_end(code, code->cur_proc));

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        if(stmnt->children[0]->childct > 0)
        {
            generateExpression(code, stmnt->children[0]->children[0]);
            store = (Instruction){.op = STOX, .L = code->L - var->L, .M = var->M, .destination = NULL};
        }
        else if(input.M == 3)
        {
            for(int i=0;i<var->param_count;++i)
            {
                store = (Instruction){.op = STO, .L = code->L - var->L, .M = var->M + i, 
                                        .destination = NULL};
                add_instruction(code, store, find_proc_end(code, code->cur_proc));
            }
        }
        else
        {
#endif
            store = (Instruction){.op = STO, .L = code->L - var->L, .M = var->M, .destination = NULL};
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        }
#endif
        add_instruction(code, store, find_proc_end(code, code->cur_proc));
    }

}

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
void generateExpression0(CodePage* code, ParseTree *expr)
#else
void generateExpression(CodePage* code, ParseTree *expr)
#endif
{
    for(unsigned int i=0;i<expr->childct;++i)
    {
        /* if(expr->children[i]->type == Negate) */
        /* { */
        /*     generateTerm(code, expr->children[i]); */
        /*     Instruction inst = (Instruction){.op = OPR, .L = 0, .M = NEG, .destination = NULL}; */
        /*     add_instruction(code, inst, find_proc_end(code, code->cur_proc)); */
        /* } */
        /* else */ 
        /* { */
            generateTerm(code, expr->children[i]);
        /* } */

        if(i < (expr->childct - 1))
        {
            ++i;
            generateTerm(code, expr->children[i]);
        }
        
        if(expr->children[i]->type == Add)
        {
            Instruction inst = (Instruction){.op =  OPR, .L = 0, .M = ADD, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
        else if(expr->children[i]->type == Subtract)
        {
            Instruction inst = (Instruction){.op =  OPR, .L = 0, .M = SUB, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
    }
}

void generateTerm(CodePage* code, ParseTree* term)
{
    for(unsigned int i=0;i<term->childct;++i)
    {
        generateFactor(code, term->children[i]);

        if(i < (term->childct - 1))
        {
            ++i;
            generateFactor(code, term->children[i]);
        }
        
        if(term->children[i]->type == Multiply)
        {
            Instruction inst = (Instruction){.op =  OPR, .L = 0, .M = MUL, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
        else if(term->children[i]->type == Divide)
        {
            Instruction inst = (Instruction){.op =  OPR, .L = 0, .M = DIV, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        else if(term->children[i]->type == Modulo)
        {
            Instruction inst = (Instruction){.op = OPR, .L = 0, .M = MOD, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
#endif
    }
}

void generateFactor(CodePage* code, ParseTree* factor)
{
    ParseTree *sfac = factor->children[0];

    Instruction inst;
    switch(sfac->type)
    {
        case Negate:
            generateFactor(code, sfac);
            inst = (Instruction){.op = OPR, .L = 0, .M = NEG, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
            break;
        case Number:
            inst = (Instruction){.op = LIT, .L = 0, .M = (sfac->intval), .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
            break;
        case Ident:
            {
                Pl0Ident *ident = lookup_ident(code, sfac->strval);

                if(ident == NULL || ident->ident_type == PROC_IDENT)
                {
                    fprintf(stderr, "Fatal Error:\nUndeclared variable/constant \'%s\'\n", sfac->strval);
                    exit(1);
                }

                if(ident->ident_type ==  CONST_IDENT)
                {
                    inst = (Instruction){.op = LIT, .L = 0, .M = ident->value, .destination = NULL};
                    add_instruction(code, inst, find_proc_end(code, code->cur_proc));
                }
                else if(ident->ident_type == VAR_IDENT)
                {
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
                    if(sfac->childct > 0)
                    {
                        generateExpression(code, sfac->children[0]);
                        if(ident->var_count > 0)
                        {
                            inst = (Instruction){.op = LOD, .L = code->L - ident->L, .M = ident->M, .destination = NULL};
                            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
                            inst = (Instruction){.op = OPR, .L = 0, .M = ADD, .destination = NULL};
                            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
                            inst = (Instruction){.op = GET, .L = 0, .M = 0, .destination = NULL};
                        }
                        else
                        {
                            inst = (Instruction){.op = LODX, .L = code->L - ident->L, .M = ident->M, .destination = NULL};
                        }
                    }
                    else
                    {
#endif
                        inst = (Instruction){.op = LOD, .L = code->L - ident->L, .M = ident->M, .destination = NULL};
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
                    }
#endif
                    if(ident->L < 0)
                    {
                        inst.L = 0;
                        inst.M = 0;
                    }
                    add_instruction(code, inst, find_proc_end(code, code->cur_proc));
                }
            }
            break;
        case Expression:
            generateExpression(code, sfac);
            break;
        case Term:
            generateTerm(code, sfac->children[0]);
            break;
#ifdef PL_PARAM_EXTENSION_SUPPORT
        case GetFromAdr: // '&' ident [ '[' expr ']' ]
        {
            generateFactor(code, sfac);
            Instruction* lod = &code->program[find_proc_end(code, code->cur_proc)-1];
            if(lod->op == LIT)
            {
                fprintf(stderr, "Cannot dereference a constant\n");
                exit(1);
            }

            if(lod->op == LODX)
            {
                lod->op = LOD;
                Instruction addem = (Instruction){.op = OPR, .L = 0, .M = ADD, .destination = NULL};
                add_instruction(code, addem, find_proc_end(code, code->cur_proc));
            }

            Instruction get = (Instruction){.op = GET, .L = 0, .M = 0, .destination = NULL};
            add_instruction(code, get, find_proc_end(code, code->cur_proc));
        }
            break;
        case Address: // '@' ident [ '[' expr ']' ]
        {
            Pl0Ident* var = lookup_ident(code, sfac->children[0]->strval);

            if(var == NULL || var->ident_type != VAR_IDENT)
            {
                fprintf(stderr, "Undeclared variable \'%s\'\n", 
                                sfac->children[0]->strval);
                exit(1);
            }

            if(var->var_count > 0) // we are getting the address of a pointer param
            {
                Instruction load = (Instruction){.op = LOD, .L = code->L - var->L, .M = var->M, .destination = NULL},
                            add  = (Instruction){.op = OPR, .L = 0, .M = ADD, .destination = NULL};
                if(sfac->children[0]->childct > 0)
                {
                    generateExpression(code, sfac->children[0]->children[0]);
                    add_instruction(code, load, find_proc_end(code, code->cur_proc));
                    add_instruction(code, add, find_proc_end(code, code->cur_proc));
                }
                else
                {
                    add_instruction(code, load, find_proc_end(code, code->cur_proc));
                }
            }
            else
            {
                generateFactor(code, sfac);
                Instruction* adrlod = &code->program[find_proc_end(code, code->cur_proc)-1];
                if(adrlod->op == LIT) 
                { 
                    fprintf(stderr, "Cannot get address of constant \'%s\'\n", 
                                    sfac->children[0]->strval);
                    exit(1);
                }

                adrlod->op = adrlod->op == LOD ? ADR : ADRX;
            }
        }
            break;
        
        case SizeOf:
        {
            Pl0Ident *id = lookup_ident(code, sfac->children[0]->strval);
            if(id == NULL || id->ident_type != VAR_IDENT)
            {
                fprintf(stderr, "Undeclared variable \'%s\'\n", 
                        sfac->children[0]->strval);
                exit(1);
            }

            Instruction inst = (Instruction){.op = LIT, .L = 0, .M =
                id->param_count > 0 ? id->param_count : 1, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
            break;
        case Call:
        {

            Pl0Ident *proc = lookup_ident(code, sfac->children[0]->strval);
            
            if(proc == NULL)
            {
                fprintf(stderr, "Fatal Error:\nUndeclared identifier \'%s\'\n", 
                            sfac->children[0]->strval);
                exit(1);
            }
            else if(proc->ident_type != PROC_IDENT)
            {
                fprintf(stderr, "Fatal Error:\nCall to non-procedure identifier \'%s\' is meaningless\n", 
                            sfac->children[0]->strval);
                exit(1);
            }


            if((int)sfac->childct - 1 != proc->param_count)
            {
                fprintf(stderr, "Fatal Error:\nCall to procedure \'%s\' does not match procedure declaration\n", 
                        sfac->children[0]->strval);
                exit(1);
            }

            Instruction inst;

            inst = (Instruction){.op = INC, .L = 0, .M = 4, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
            for(unsigned int i=1;i<sfac->childct;++i)
            {
                generateExpression(code, sfac->children[i]);
            }
            unsigned int offset = (4+proc->param_count);
            inst = (Instruction){.op = INC, .L = 0, .M = -offset, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));

            inst = (Instruction){.op = CAL, .L = code->L - proc->L, .M = 0, .destination = proc->name};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));

            inst = (Instruction){.op = INC, .L = 0, .M = 1, .destination = NULL};
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        } 
            break;
        default:
            generateExpression(code, sfac);
            break;
#else
        default:
            fprintf(stderr, "Undefined error handling incorrect parse tree node FACTOR_NODE"
                            "of type %d\n", sfac->type);
            exit(1);
#endif
    }
}

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
void generateExpression(CodePage* code, ParseTree *cond)
{
    Instruction inst = (Instruction){.op = OPR, .L = 0, .M = OR, .destination = NULL};
    generateRelationA(code, cond->children[0]);
    

    for(unsigned int i=cond->children[0]->type != Ternary ? 1 : 0;i<cond->childct;++i)
    {
        if(cond->children[i]->type == Ternary)
        {
            /* printf("Generating ternary expression...\n"); */
            Instruction jmp_to_else,jmp_past_else,bogus_jmp;

            size_t skip_if_loc = find_proc_end(code, code->cur_proc);

            generateExpression(code, cond->children[i]);
            size_t skip_else_loc = find_proc_end(code, code->cur_proc);
            ++i;

            generateExpression(code, cond->children[i]);

            bogus_jmp = (Instruction){.op = JMP, .L = 0, .M = find_proc_end(code, code->cur_proc), .destination = NULL};
            add_instruction(code, bogus_jmp, find_proc_end(code, code->cur_proc));

            jmp_past_else = (Instruction){.op = JMP, .L = 0, .M = find_proc_end(code, code->cur_proc), .destination = NULL};
            add_instruction(code, jmp_past_else, skip_else_loc);
            jmp_to_else = (Instruction){.op = JPC, .L = 0, .M = skip_else_loc+2, .destination = NULL};
            add_instruction(code, jmp_to_else, skip_if_loc);
            /* printf("Finished generating ternary expression...\n"); */
        }
        else
        {
            generateRelationA(code, cond->children[i]);
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
        }
    }
}

void generateRelationA(CodePage* code, ParseTree *cond)
{
    Instruction inst = (Instruction){.op = OPR, .L = 0, .M = AND, .destination = NULL},
                lnot = (Instruction){.op = OPR, .L = 0, .M = LNOT, .destination = NULL};
    if(cond->children[0]->type == Negate)
    {
        generateRelation(code, cond->children[0]->children[0]);
        add_instruction(code, lnot, find_proc_end(code, code->cur_proc));
    }
    else
    {
        generateRelation(code, cond->children[0]);
    }

    for(unsigned int i=1;i<cond->childct;++i)
    {
        if(cond->children[i]->type == Negate)
        {
            generateRelation(code, cond->children[i]->children[0]);
            add_instruction(code, lnot, find_proc_end(code, code->cur_proc));
        }
        else
        {
            generateRelation(code, cond->children[i]);
        }
        add_instruction(code, inst, find_proc_end(code, code->cur_proc));
    }
}

void generateRelation(CodePage* code, ParseTree  *cond)
#else
void generateCondition(CodePage* code, ParseTree *cond)
#endif
{
    Instruction inst;
    switch(cond->type)
    {
        case Odd:
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            generateExpression0(code, cond->children[0]);
#else
            generateExpression(code, cond->children[0]);
#endif
            inst = (Instruction){.op = OPR, .L = 0,  .M = ODD, .destination = NULL}; 
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
            break;
        case Condition:
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            for(int i=0;i<cond->childct;i+=3)
            {
                generateExpression0(code, cond->children[i]);
                if(cond->childct > 1)
                {
                    generateExpression0(code, cond->children[i+2]);

                    switch(cond->children[i+1]->type)
                    {
                        case GT:
                            inst = (Instruction){.op = OPR, .L = 0,  .M = GTR, .destination = NULL}; 
                            break;
                        case LT:
                            inst = (Instruction){.op = OPR, .L = 0,  .M = LSS, .destination = NULL}; 
                            break;
                        case GTE:
                            inst = (Instruction){.op = OPR, .L = 0,  .M = GEQ, .destination = NULL}; 
                            break;
                        case LTE:
                            inst = (Instruction){.op = OPR, .L = 0,  .M = LEQ, .destination = NULL}; 
                            break;
                        case EQ:
                            inst = (Instruction){.op = OPR, .L = 0,  .M = EQL, .destination = NULL}; 
                            break;
                        case NE:
                            inst = (Instruction){.op = OPR, .L = 0,  .M = NEQ, .destination = NULL}; 
                            break;
                        default:
                            fprintf(stderr, "Unspecified error generating code for conditional expression"
                                            " of type %d\n", cond->children[i+1]->type);
                            exit(1);
                    }
                    add_instruction(code, inst, find_proc_end(code, code->cur_proc));
                }
            }
#else
            generateExpression(code, cond->children[0]);
            generateExpression(code, cond->children[2]);
            switch(cond->children[1]->type)
            {
                case GT:
                    inst = (Instruction){.op = OPR, .L = 0,  .M = GTR, .destination = NULL}; 
                    break;
                case LT:
                    inst = (Instruction){.op = OPR, .L = 0,  .M = LSS, .destination = NULL}; 
                    break;
                case GTE:
                    inst = (Instruction){.op = OPR, .L = 0,  .M = GEQ, .destination = NULL}; 
                    break;
                case LTE:
                    inst = (Instruction){.op = OPR, .L = 0,  .M = LEQ, .destination = NULL}; 
                    break;
                case EQ:
                    inst = (Instruction){.op = OPR, .L = 0,  .M = EQL, .destination = NULL}; 
                    break;
                case NE:
                    inst = (Instruction){.op = OPR, .L = 0,  .M = NEQ, .destination = NULL}; 
                    break;
                default:
                    fprintf(stderr, "Unspecified error generating code for conditional expression\n");
                    exit(1);
            }
            add_instruction(code, inst, find_proc_end(code, code->cur_proc));
#endif
            break;
/* #ifdef PL_SYNTAX_EXTENSION_SUPPORT */
/*         case Relation: */
/*             generateCondition(code, cond->children[0]); */
/*             break; */
/* #endif */
        default:
            fprintf(stderr, "Error parsing invalid condition\n");
            exit(1);
    }
}

CodePage *generate_code(ParseTree *block0)
{
    CodePage *code = new_codepage();
    code->symboltable = fopen("symtable.txt", "w");

    fprintf(code->symboltable, "%11s %5s %5s %5s\n", "name", "type", "level", "value");

    generateBlock(code, block0);
    Instruction inst = {.op = 11, .L = 0, .M = 3, .destination = NULL};
    add_instruction(code, inst, find_proc_end(code, code->cur_proc));

    for(unsigned int i=0;i<code->instct;++i)
    {
        if(code->program[i].op == 7 && code->program[i].M == i)        
        {
            code->program[i].M += 1;
        }
        if(code->program[i].op == 5 && code->program[i].M == i)
        {
            /* printf("Correcting CAL from %d to %d\n", */
            /*         i, */
            /*         i + 2); */
            code->program[i].M += 2;
        }
        else if(code->program[i].destination != NULL)
        {
            /* printf("Procedure jump block detected: %s\n", code->program[i].destination); */
            code->program[i].M = find_proc_begin(code, code->program[i].destination);
        }

        /* printf("%03d | %02d %02d %02d\n", */ 
        /*         i, */
        /*         code->program[i].op, */
        /*         code->program[i].L, */
        /*         code->program[i].M); */

    }
    /* puts("\n\n\n"); */

    for(unsigned int i=0;i<code->instct;++i)
    {
        if(code->program[i].op == JMP && 
                code->program[i].M == (i+1) && 
                code->program[i+1].op == JMP && 
                code->program[i+1].M == (i+2))
        {
            delete_instruction(code, i+1);
            code->program[i].M += 1;
            --i;
        }
        else if(code->program[i].op == OPR &&
                code->program[i].M  == RET &&
                i < code->instct - 1)
        {
            if( code->program[i+1].op == OPR &&
                code->program[i+1].M  == RET)
            {
                delete_instruction(code, i+1);
                --i;
            }
        }
    }

    fclose(code->symboltable);

    return code;
}
