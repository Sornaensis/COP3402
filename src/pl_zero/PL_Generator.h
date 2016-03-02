#ifndef __PL_GENERATOR_H_INCLUDED
#define __PL_GENERATOR_H_INCLUDED

#include <stdlib.h>
#include <string/SStr.h>

#include "PL_Defs.h"
#include "PL_Parser.h"

typedef struct 
{
    short     ident_type, //0 - Var; 1 - Const; 2 - Procedure
              L,
              M;
    int       value,
              param_count,
              var_count;
    string    name;
} Pl0Ident;

typedef struct
{
    int             op,
                    L,
                    M;
    string          destination; // CALL this procedure if non-NULL
} Instruction;

typedef struct 
{
    Pl0Ident       *SymbolTable,
                   *ProcTable;
    Instruction    *program;
    FILE           *symboltable;
    string         cur_proc;
    int            L, M;
    unsigned int   symct,
                   procct,
                   instct;
} CodePage;

CodePage *new_codepage();
unsigned int find_proc_end(CodePage *page, string proc);
unsigned int find_proc_begin(CodePage *page, string proc);
Pl0Ident *lookup_ident(CodePage *page, string ident);
void add_instruction(CodePage *page, Instruction inst, unsigned int loc);
void delete_instruction(CodePage *page, unsigned int loc);
void add_identifier(CodePage *page, short type, string ident, int value);
void trim_symbol_table(CodePage *page);

CodePage *generate_code(ParseTree *block0);

#endif
