#ifndef __PL0_H_INCLUDED
#define __PL0_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#include <common/Common.h>
#include <lexer/Lexer.h>

#include "PL_Defs.h"

SymbolStream *init_pl0_lexer();
string run_pl0_lexer(SymbolStream *lexer, FILE *in);
string get_type_string(token_type t);

#endif
