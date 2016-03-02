#ifndef PL_PARSER_INCLUDED
#define PL_PARSER_INCLUDED

#include <common/Common.h>
#include <lexer/Lexer.h>

#include "PL_Defs.h"

typedef struct pTree
{
    struct 
    pTree        *parent,
                 **children;
    size_t       childct;
    parse_type   type;
    string       strval;
    int          intval;
} ParseTree;

#define PARSENODE(x) new_parsetree((ParseTree){.parent     = NULL, \
                                               .children   = NULL, \
                                               .childct    = 0,    \
                                               .type       = (x),  \
                                               .strval     = NULL, \
                                               .intval     = 0})
#define PARSENODE3(x,strv,intv) new_parsetree((ParseTree){.parent     = NULL, \
                                               .children   = NULL,   \
                                               .childct    = 0,      \
                                               .type       = (x), \
                                               .strval     = (strv), \
                                               .intval     = (intv)})
#define ROOT_NODE PARSENODE (Root)
#define BLOCK_NODE PARSENODE(Block)
#define CONST_DEC_NODE PARSENODE(Const_decl)
#define VAR_DEC_NODE PARSENODE(Var_decl)
#define PROC_DEC_NODE PARSENODE(Proc_decl)
#define STATEMENT_LIST_NODE PARSENODE(StatementList)
#define ASSIGN_NODE PARSENODE(Assignment)
#define READ_NODE PARSENODE(Read)
#define WRITE_NODE PARSENODE(Write)
#define EXPRESSION_NODE PARSENODE(Expression)
#define TERM_NODE PARSENODE(Term)
#define FACTOR_NODE PARSENODE(Factor)
#define IDENT_NODE(x,y) PARSENODE3(Ident, x, y)
#define NUMBER_NODE(x,y) PARSENODE3(Number, x, y)
#define CONDITION_NODE PARSENODE(Condition)
#define IF_NODE PARSENODE(If)
#define THEN_NODE PARSENODE(Then)
#define ELSE_NODE PARSENODE(Else)
#define WHILE_NODE PARSENODE(While)
#define ODD_NODE PARSENODE(Odd)
#define REL_OP_NODE PARSENODE(Rel_op)
#define EQ_NODE PARSENODE(EQ)
#define GT_NODE PARSENODE(GT)
#define LT_NODE PARSENODE(LT)
#define GTE_NODE PARSENODE(GTE)
#define NEQ_NODE PARSENODE(NE)
#define LTE_NODE PARSENODE (LTE)
#define NEGATE_NODE PARSENODE (Negate)
#define UNARY_NODE PARSENODE (Unary)
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
#define OR_NODE PARSENODE (Or)
#define AND_NODE PARSENODE (And)
#define STR_LIT_NODE PARSENODE (StrLiteral)
#define CHAR_LIT_NODE PARSENODE (CharLiteral)
#endif


ParseTree *new_parsetree(ParseTree t);
ParseTree *add_parsetree_child(ParseTree *node, ParseTree *n);
void destroy_parsetree(ParseTree *node);
void delete_parsetree_node(ParseTree *parent, ParseTree *child);

typedef struct 
{
    Symbol         **symbols;
    unsigned int   index,
                   symct;
    ParseTree      *current;
} plParser;

plParser *new_pl_parser(Symbol **symbols, unsigned int count);

/* #define DEBUG_PARSER_CALLS */
#ifdef DEBUG_PARSER_CALLS
#define DEBUG_PARSER(x) { fprintf(stderr, "Calling parser "#x"\n"); }
#endif
#ifndef DEBUG_PARSER_CALLS
#define DEBUG_PARSER(x) 
#endif

#define PARSERROR(errmsg) { if(ps->index < ps->symct) {fprintf(stderr, "Failing on symbol #%d: \'%s\' ==> ", ps->index, ps->symbols[ps->index]->symbol);} fprintf(stderr, "%s\n", __parserrors[errmsg]); exit(1); }
#define RUN_PARSER(x,errmsg) { DEBUG_PARSER(x) if( (x) == false) { PARSERROR(errmsg) }  }

// ** Non-Terminal Consumption Parser **//

bool parse_program(plParser *ps);     // ::= block "."
bool parse_block(plParser *ps);       // ::= [const-dec] [var-dec] {proc-dec} statement
bool parse_const_dec(plParser *ps);   // ::= "const" ident "=" number {"," ident "=" number} ";"
bool parse_var_dec(plParser *ps);     // ::= "var" ident {"," ident} ";"
bool parse_proc_dec(plParser *ps);    // ::= "procedure" ident "(" [ ident {"," ident} ] ")" ";" block ";"
bool parse_statement(plParser *ps);   // ::= ident ":=" expression                               |
                                      //     "call" ident  "(" [ expression {, expression} ] ")" |
                                      //     "begin" statement {";" statement} "end"             |
                                      //     "if" condition "then" statement                     
                                      //     {"else if" condition "then" statement} 
                                      //     ["else" statement]  
                                      //     "while" condition "do" statement                    |
                                      //     "read" ident                                        |
                                      //     "write" expression                                  |
                                      //     empty                                               |
bool parse_expression(plParser *ps);   // ::= relationA {"or" relationA}
/* Duplicate for multiple options */
bool parse_condition(plParser *ps);   // ::= relationA {"or" relationA}
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
bool parse_relationA(plParser *ps);   // ::= relation {"and" relation}
bool parse_relation(plParser *ps);    // ::= "odd" expression0 | expression0 rel-op expression0
bool parse_expression0(plParser *ps);  // ::= ["+"|"-"] term {("+"|"-") term}
#endif
bool parse_term(plParser *ps);        // ::= ["+"/"-"] factor {("*"|"/"|"%") ["+"/"-"] factor}
bool parse_factor(plParser *ps);      // ::= number | ident | "(" expression ")" | 
                                      //     "call" ident "(" [ expression {"," expression} ] ")"

// ** Terminal Consumption Parsers **//

bool parsesym(plParser *ps, token_type symbol); // ::= symbol literal
bool parse_ident(plParser *ps);
bool parse_rel_op(plParser *ps);
bool parse_number(plParser *ps);
bool parse_literal(plParser *ps);
bool parse_cliteral(plParser *ps);
bool parse_sliteral(plParser *ps);


#endif


