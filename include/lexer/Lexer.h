#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <stdio.h>

#include <common/Common.h>
#include <string/SStr.h>
#include <lexer/LexTree.h>

typedef struct symstr SymbolStream;
typedef struct sym Symbol;

// Symbol stream struct && functions ****

typedef struct symstr 
{
    LexNode         **lexing_system;
    int             *lextypes;
    Symbol          **symbols;
    unsigned int    symbolct,
                    lexerct;
} SymbolStream;

SymbolStream *new_symbolstream();
bool add_symbol(SymbolStream *ss, Symbol *s);
void destroy_symbolstream(SymbolStream *ss);
void add_lexer(SymbolStream *ss, LexNode *lexer, int type);

//  Symbol struct && functions ****

typedef struct sym
{
    string        symbol; // Symbol value
    int           symtype;
} Symbol;

Symbol *new_symbol(string val, int stype);
void destroy_symbol(Symbol *s);

// ***  Automata Lexing Functions below *** ///

#define _LA(...) (LexNode *[]){new_lexnode(""),__VA_ARGS__,NULL}
#define _SUBLA(...) (LexNode *[]){__VA_ARGS__,NULL}
#define CREATE_LEXER(...) construct_lexer_from_table(_LA(__VA_ARGS__))
#define SUB_LEXER(...) construct_lexer_from_table(_SUBLA(__VA_ARGS__))
#define _STAR(x) (repeat_node(optional_node((x))))
#define _PLUS(x) (x),(repeat_node(optional_node((x))))
#define _QMARK(x) (optional_node((x)))

#define ALPHABET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define NUMERALS "0123456789"
#define WHITESPACE "\b \t\r\n\0xA"

bool run_automata(LexNode* root, string *symbol, string stream, unsigned int *index);

SymbolStream *read_lexer_from_file(FILE *in);
string perform_lex_on_stream(SymbolStream *ss, string stream);

LexNode *construct_lexer_from_table(LexNode* lexlist[]);
LexNode **get_lexer_end_nodes(LexNode *root);

//** Static LexNodes **//

LexNode *alphabetical_character();
LexNode *numeric_character();
LexNode *alphanumeric_character();
LexNode *whitespace();
LexNode *string_literal();
LexNode *char_literal();
LexNode *c_style_comments();
LexNode *repeat_node(LexNode *l);
LexNode *optional_node(LexNode *l);

#endif
