#include <stdlib.h>
#include <string/SStr.h>

#include "PL_Defs.h"
#include "PL_Parser.h"
#include "PL_Generator.h"

CodePage *new_codepage()
{
    CodePage *new = malloc(sizeof(CodePage));
    new->SymbolTable = NULL;
    new->program = NULL;
    new->symct = 0;
    new->instct = 0;
    new->L      = 0;
    new->M      = 0;

    return new;
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

void add_instruction(CodePage *page, Instruction inst)
{
    if(page->program == NULL)
    {
        page->program = malloc(sizeof(Instruction));
    }
    else
    {
        page->program = realloc(page->program, sizeof(Instruction) * (page->instct + 1));
    }

    page->program[page->instct] = inst;
    page->instct += 1;
}

void add_identifier(CodePage *page, short type, string ident, int value)
{
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
                                                .value = value, 
                                                .name = ident};
    page->symct += 1;
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
}


CodePage *generate_code(ParseTree *block0)
{
    CodePage *code = new_codepage();

    return code;
}
