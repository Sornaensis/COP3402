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
    int       value;
    string    name;
} Pl0Ident;

typedef struct
{
    int     op,
            L,
            M;
} Instruction;

typedef struct 
{
    Pl0Ident       *SymbolTable;
    Instruction    *program;
    int            L, M;
    unsigned int   symct,
                   instct;
} CodePage;

CodePage *new_codepage();
Pl0Ident *lookup_ident(CodePage *page, string ident);
void add_instruction(CodePage *page, Instruction inst);
void add_identifier(CodePage *page, short type, string ident, int value);
void trim_symbol_table(CodePage *page);

CodePage *generate_code(ParseTree *block0);

#endif
