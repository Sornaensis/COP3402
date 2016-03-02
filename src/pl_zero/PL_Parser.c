#include <stdlib.h>

#include <common/Common.h>
#include <string/SStr.h>

#include "PL_Parser.h"

static string __parserrors[] = {
    "PARSER BUG: error condition not handled",                  //0
    "End period not found after block",                         //1
    "Const, var, procedure must be followed by identifier",     //2
    "Use = instead of :=",                                      //3
    "= must be followed by a number",                           //4
    "Semicolon or comma not found",                             //5
    "Identifier must be followed by :=",                        //6
    "Expression error not handled",                             //7
    "call must be followed by an identifier",                   //8
    "Statement expected",                                       //9
    "end expected",                                             //10
    "Relational operator expected",                             //11
    "then expected",                                            //12
    "do expected",                                              //13
    "Identifier after read expected",                           //14
    "Identifier after write expected",                          //15
    "Expression expected after odd",                            //16
    "Expressionexpected",                                      //17
    "Relational operator expected",                             //18
    "Term expected",                                            //19
    "Factor expected",                                          //20
    "Right parentheses not found",                              //21
    "Expected semicolon following procedure declaration",       //22
    "Expected block within procedure declaration",              //23

     };

// ** Nonterminal Parsers **//

bool parse_program(plParser *ps)
{
    RUN_PARSER(parse_block(ps),0);
    RUN_PARSER(parsesym(ps, periodsym),1);

    return true;
}

bool parse_block(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, BLOCK_NODE);
    ParseTree *me = ps->current;

    if(parse_const_dec(ps))
    {
        ps->current = me;
        parse_var_dec(ps);
        ps->current = me;
    }
    else if(parse_var_dec(ps))
    {
        ps->current = me;
        parse_const_dec(ps);
        ps->current = me;
    }

    parse_proc_dec(ps);
    ps->current = me;

    parse_statement(ps);
    ps->current = me;

    return true;
}

bool parse_const_dec(plParser *ps)
{
    if(!parsesym(ps, constsym))
        return false;

    ps->current = add_parsetree_child(ps->current, CONST_DEC_NODE);
    ParseTree *me = ps->current;

    RUN_PARSER(parse_ident(ps), 2);
    ps->current = me;

    RUN_PARSER(parsesym(ps, eqsym), 3);

    RUN_PARSER(parse_number(ps), 4);
    ps->current = me;

    while(parsesym(ps, commasym))
    {
        RUN_PARSER(parse_ident(ps), 2);
        ps->current = me;

        RUN_PARSER(parsesym(ps, eqsym), 3);

        RUN_PARSER(parse_number(ps), 4);
        ps->current = me;
    }

    RUN_PARSER(parsesym(ps, semicolonsym),5);

    return true;
}

bool parse_var_dec(plParser *ps)
{
    if(!parsesym(ps, varsym))
        return false;

    ps->current = add_parsetree_child(ps->current, VAR_DEC_NODE);
    ParseTree *me = ps->current;

    RUN_PARSER(parse_ident(ps), 2);

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    if(parsesym(ps, lbracketsym))
    {
        RUN_PARSER(parse_number(ps), 0);
        RUN_PARSER(parsesym(ps, rbracketsym), 0);
    }
#endif
    ps->current = me;

    while(parsesym(ps, commasym))
    {
        RUN_PARSER(parse_ident(ps), 2);

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        if(parsesym(ps, lbracketsym))
        {
            RUN_PARSER(parse_number(ps), 0);
            RUN_PARSER(parsesym(ps, rbracketsym), 0);
        }
#endif
        ps->current = me;
    }

    RUN_PARSER(parsesym(ps, semicolonsym), 5);

    return true;
}

bool parse_proc_dec(plParser *ps)
{
    if(!parsesym(ps, procsym))
        return false;

    ps->current = add_parsetree_child(ps->current, PROC_DEC_NODE);
    ParseTree *me = ps->current;

    RUN_PARSER(parse_ident(ps), 2);
    ps->current = me;

#ifdef PL_PARAM_EXTENSION_SUPPORT
    RUN_PARSER(parsesym(ps, lparentsym), 0);

    if(!parsesym(ps, rparentsym))
    {
        RUN_PARSER(parse_ident(ps), 0);
        if(parsesym(ps, getatsym))
        {
            ps->current->type = Address;
        }
        ps->current = me;

        while(parsesym(ps, commasym))
        {
            RUN_PARSER(parse_ident(ps), 0);
            if(parsesym(ps, getatsym))
            {
                ps->current->type = Address;
            }
            ps->current = me;
        }

        RUN_PARSER(parsesym(ps, rparentsym), 0);
    }
#endif

    RUN_PARSER(parsesym(ps, semicolonsym), 5);

    RUN_PARSER(parse_block(ps), 23);
    ps->current = me;

    RUN_PARSER(parsesym(ps, semicolonsym),22);

    while(parsesym(ps, procsym))
    {
        RUN_PARSER(parse_ident(ps), 2);
        ps->current = me;

#ifdef PL_PARAM_EXTENSION_SUPPORT
        RUN_PARSER(parsesym(ps, lparentsym), 0);

        if(!parsesym(ps, rparentsym))
        {
            RUN_PARSER(parse_ident(ps), 0);
            if(parsesym(ps, getatsym))
            {
                ps->current->type = Address;
            }
            ps->current = me;

            while(parsesym(ps, commasym))
            {
                RUN_PARSER(parse_ident(ps), 0);
                if(parsesym(ps, getatsym))
                {
                    ps->current->type = Address;
                }
                ps->current = me;
            }

            RUN_PARSER(parsesym(ps, rparentsym), 0);
        }
#endif
        RUN_PARSER(parsesym(ps, semicolonsym),22);

        RUN_PARSER(parse_block(ps), 0);
        ps->current = me;

        RUN_PARSER(parsesym(ps, semicolonsym),22);
    }

    return true;
}

bool parse_statement(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, ASSIGN_NODE);
    ParseTree *me = ps->current;

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    /* OH GOD IT'S FINALLY HAPPENED, A PRACTICAL USE FOR GOTO */
    if(parsesym(ps, getatsym))
    {
        ps->current = add_parsetree_child(ps->current, PARSENODE(Address));
        RUN_PARSER(parse_ident(ps), 0);
        goto do_assign_stmnt;
    }
#endif

    if(parse_ident(ps))
    {
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
do_assign_stmnt:

        if(parsesym(ps, lbracketsym))
        {
            RUN_PARSER(parse_expression(ps), 17);
            RUN_PARSER(parsesym(ps, rbracketsym), 0);
        }
#endif
        ps->current = me;
        RUN_PARSER(parsesym(ps, becomessym), 6);

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        if(parse_sliteral(ps))
        {
            ps->current->type = StrLiteral;
        }
        else
        {
            if(!parse_cliteral(ps))
            {
                RUN_PARSER(parse_expression(ps), 7);
            }
            else
            {
                ps->current->type = CharLiteral;
            }
        }
        ps->current = me;
        return true;
#endif
        RUN_PARSER(parse_expression(ps), 7);
        ps->current = me;

        return true;
    }
    else
    {
        ps->current = me;
        if(parsesym(ps, callsym))
        {
            me->type = Call;
            RUN_PARSER(parse_ident(ps), 8);
            ps->current = me;

#ifdef PL_PARAM_EXTENSION_SUPPORT
            RUN_PARSER(parsesym(ps, lparentsym), 0);

            if(parsesym(ps, rparentsym))
            {
                return true;
            }

            RUN_PARSER(parse_expression(ps), 0);
            ps->current = me;

            while(parsesym(ps, commasym))
            {
                RUN_PARSER(parse_expression(ps), 0);
                ps->current = me;
            }

            RUN_PARSER(parsesym(ps, rparentsym), 0);
#endif

            return true;
        }
        else if(parsesym(ps, beginsym))
        {
            me->type = StatementList;
            RUN_PARSER(parse_statement(ps), 9);
            ps->current = me;

            while(parsesym(ps, semicolonsym))
            {
                RUN_PARSER(parse_statement(ps), 9);
                ps->current = me;
            }

            RUN_PARSER(parsesym(ps, endsym), 10);
            return true;
        }
        else if(parsesym(ps, ifsym))
        {
            me->type = If;
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            RUN_PARSER(parse_expression(ps), 11);
#else
            RUN_PARSER(parse_condition(ps), 11);
#endif
            ps->current = me;

            RUN_PARSER(parsesym(ps, thensym), 12);

            RUN_PARSER(parse_statement(ps), 9);
            ps->current = me;

            if(parsesym(ps, elsesym))
            {
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
                if(parsesym(ps, ifsym))
                {
                    while(1) 
                    {
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
                        RUN_PARSER(parse_expression(ps), 11);
#else
                        RUN_PARSER(parse_condition(ps), 11);
#endif
                        ps->current = me;

                        RUN_PARSER(parsesym(ps, thensym), 0);
                        RUN_PARSER(parse_statement(ps), 0);
                        ps->current = me;

                        if(parsesym(ps, elsesym))
                        {
                            if(!parsesym(ps, ifsym))
                            {
                                RUN_PARSER(parse_statement(ps), 9);
                                ps->current = me;
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                     } 

                }
                else 
                {
#endif
                    RUN_PARSER(parse_statement(ps), 9);
                    ps->current = me;
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
                }
#endif

            }
            return true;
        }
        else if(parsesym(ps, whilesym))
        {
            me->type = While;
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            RUN_PARSER(parse_expression(ps), 11);
#else
            RUN_PARSER(parse_condition(ps), 11);
#endif
            ps->current = me;

            RUN_PARSER(parsesym(ps, dosym), 13);

            RUN_PARSER(parse_statement(ps), 9);
            ps->current = me;
            return true;
        }
        else if(parsesym(ps, readsym))
        {
            me->type = Read;
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            if(parsesym(ps, charsym))
            {
                ps->current = add_parsetree_child(ps->current, PARSENODE(ReadChar));
            }
            else if(parsesym(ps, stringsym))
            {
                ps->current = add_parsetree_child(ps->current, PARSENODE(ReadString));
            }
#endif

            RUN_PARSER(parse_ident(ps), 14 );
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            if(parsesym(ps, lbracketsym))
            {
                RUN_PARSER(parse_expression(ps), 17);
                RUN_PARSER(parsesym(ps, rbracketsym), 0);
            }
#endif
            ps->current = me;
            return true;
        }
        else if(parsesym(ps, writesym))
        {
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
            ParseTree *me2;
            while (1)
            {
                me2 = ps->current;
                me2->type = Write;
                if(parsesym(ps, lnsym))
                {
                   me2->type = WriteLn;
                }
                /* If this is not a string or character literal */
                if(!parse_literal(ps))
                {
                    if(parsesym(ps, charsym)) /* Character formatting */
                    {
                        ps->current = add_parsetree_child(ps->current, PARSENODE(Char));
                        if(!parse_cliteral(ps))
                        {
                            RUN_PARSER(parse_expression(ps), 17);
                        }
                    }
                    else if(parsesym(ps, hexsym)) /* Hex formatting */
                    {
                        ps->current = add_parsetree_child(ps->current, PARSENODE(Hex));
                        if(!parse_cliteral(ps))
                        {
                            RUN_PARSER(parse_expression(ps), 17);
                        }
                    }
                    else if(parsesym(ps, octsym)) /* Octal formatting */
                    {
                        ps->current = add_parsetree_child(ps->current, PARSENODE(Oct));
                        if(!parse_cliteral(ps))
                        {
                            RUN_PARSER(parse_expression(ps), 17);
                        }
                    }
                    else if(parsesym(ps, binsym)) /* Binary formatting */
                    {
                        ps->current = add_parsetree_child(ps->current, PARSENODE(Bin));
                        if(!parse_cliteral(ps))
                        {
                            RUN_PARSER(parse_expression(ps), 17);
                        }
                    }
                    else if(parsesym(ps, stringsym)) /* Print out a string literal OR an array as a string */
                    {
                        ps->current = add_parsetree_child(ps->current, PARSENODE(String));
                        if(!parse_sliteral(ps))
                        {
                            RUN_PARSER(parse_ident(ps), 0);
                        }
                    }
                    else
                    {
                        if(me2->type == WriteLn)
                        {
                            if(parsesym(ps, semicolonsym))
                            {
                                ps->index -= 1;
                                ps->current = me;
                                return true;
                            }
                            else if(parsesym(ps, endsym))
                            {
                                ps->index -= 1;
                                ps->current = me;
                                return true;
                            }
                            else if(parsesym(ps, periodsym))
                            {
                                ps->index -= 1;
                                ps->current = me;
                                return true;
                            }
                            else if(parsesym(ps, elsesym))
                            {
                                ps->index -= 1;
                                ps->current = me;
                                return true;
                            }
                            else if(parsesym(ps, commasym))
                            {
                                ps->index -= 1;
                            }
                            else
                            {
                                RUN_PARSER(parse_expression(ps), 17);
                            }
                        }
                        else
                        {
                            RUN_PARSER(parse_expression(ps), 17);
                        }
                    }
                }
                if(parsesym(ps, commasym))
                {
                    ps->current = me;
                    ps->current = add_parsetree_child(ps->current, PARSENODE(Write));
                }
                else
                {
                    ps->current = me;
                    return true;
                }
            }
#else
            RUN_PARSER(parse_ident(ps), 15);
            ps->current = me;
#endif
            return true;
        }
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        else if(parsesym(ps, returnsym))
        {
            me->type = Return;
            if(parsesym(ps, semicolonsym))
            {
                ps->index -= 1;
            }
            else if(parsesym(ps, endsym))
            {
                ps->index -= 1;
            }
            else if(parsesym(ps, periodsym))
            {
                ps->index -= 1;
            }
            else if(parsesym(ps, elsesym))
            {
                ps->index -= 1;
            }
            else
            {
                parse_expression(ps);
            }
            ps->current = me;
            return true;
        }
#endif
    }

    me->type = Epsilon;

    return true;
}

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
bool parse_expression(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, OR_NODE);
    ParseTree *me = ps->current;

    RUN_PARSER(parse_relationA(ps), 0);
    ps->current = me;

    while(parsesym(ps, orsym))
    {
        RUN_PARSER(parse_relationA(ps), 0);
        ps->current = me;
    }

    /* TERNARY OPERATOR SUPPORT */
    if(parsesym(ps, qmarksym))
    {
        RUN_PARSER(parse_expression(ps), 17);
        ps->current->type = Ternary;
        ps->current = me;
        RUN_PARSER(parsesym(ps, colonsym), 0);
        ps->current = me;
        RUN_PARSER(parse_expression(ps), 17);
        ps->current = me;
    }

    return true;
}

bool parse_relationA(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, AND_NODE);
    ParseTree *me = ps->current;

    if(parsesym(ps, lnotsym))
    {
        ps->current = add_parsetree_child(ps->current, PARSENODE(Negate));
    }

    RUN_PARSER(parse_relation(ps), 0);
    ps->current = me;

    while(parsesym(ps, andsym))
    {
        if(parsesym(ps, lnotsym))
        {
            ps->current = add_parsetree_child(ps->current, PARSENODE(Negate));
        }
        RUN_PARSER(parse_relation(ps), 0);
        ps->current = me;
    }

    return true;
}

bool parse_relation(plParser *ps)
#else
bool parse_condition(plParser *ps)
#endif
{
    ps->current = add_parsetree_child(ps->current, ODD_NODE);
    ParseTree *me = ps->current;

    if(parsesym(ps, oddsym))
    {
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        RUN_PARSER(parse_expression0(ps), 17);
#else
        RUN_PARSER(parse_expression(ps), 17);
#endif
        ps->current = me;
        return true;
    }
/* #ifdef PL_SYNTAX_EXTENSION_SUPPORT */
/*     else if(parsesym(ps, lbracketsym)) */
/*     { */
/*         me->type = Relation; */
/*         RUN_PARSER(parse_condition(ps), 0); */
/*         ps->current = me; */

/*         RUN_PARSER(parsesym(ps, rbracketsym), 0); */
/*         return true; */ 
/*     } */
/* #endif */

    me->type = Condition;

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    RUN_PARSER(parse_expression0(ps), 17);
#else
    RUN_PARSER(parse_expression(ps), 17);
#endif
    ps->current = me;

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    while(parse_rel_op(ps))
    {
        ps->current = me;
        RUN_PARSER(parse_expression0(ps), 17);
        ps->current = me;
    }
#else
    RUN_PARSER(parse_rel_op(ps), 18);
    ps->current = me;

    RUN_PARSER(parse_expression(ps), 17);
    ps->current = me;
#endif
    return true;
}

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
bool parse_expression0(plParser *ps)
#else
bool parse_expression(plParser *ps)
#endif
{
    ps->current = add_parsetree_child(ps->current, EXPRESSION_NODE);
    ParseTree *me = ps->current;
    /* int first_type = -1; */

    /* if(parsesym(ps, plussym)) */
    /* { */
    /*     /1* ps->current = add_parsetree_child(ps->current, UNARY_NODE); *1/ */
    /* } */
    /* else if(parsesym(ps, minussym)) */
    /* { */
    /*     /1* ps->current = add_parsetree_child(ps->current, NEGATE_NODE); *1/ */
    /*     first_type = Negate; */
    /* } */

    RUN_PARSER(parse_term(ps), 19);
    /* if(first_type == Negate) */
    /* { */
    /*     ps->current->type = Negate; */
    /* } */
    ps->current = me;

    while(1)
    {
        if(parsesym(ps, plussym))
        {
            RUN_PARSER(parse_term(ps), 19);
            ps->current->type = Add;
            ps->current = me;
        }
        else if(parsesym(ps, minussym))
        {
            RUN_PARSER(parse_term(ps), 19);
            ps->current->type = Subtract;
            ps->current = me;
        }
        else
        {
            return true;
        }
    }

}

bool parse_term(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, TERM_NODE);
    ParseTree *me = ps->current;

    RUN_PARSER(parse_factor(ps), 20);
    ps->current = me;

    while(1)
    {
        if(parsesym(ps, multsym))
        {
            RUN_PARSER(parse_factor(ps), 20);
            ps->current->type = Multiply;
            ps->current = me;
        }
        else if(parsesym(ps, slashsym))
        {
            RUN_PARSER(parse_factor(ps), 20);
            ps->current->type = Divide;
            ps->current = me;
        }
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
        else if(parsesym(ps, modsym))
        {
            RUN_PARSER(parse_factor(ps), 20);
            ps->current->type = Modulo;
            ps->current = me;
        }
#endif
        else
        {
            return true;
        }
    }

}

bool parse_factor(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, FACTOR_NODE);
    ParseTree *me = ps->current;

    if(parsesym(ps, plussym))
    {
    }
    else if(parsesym(ps, minussym))
    {
        ps->current = add_parsetree_child(ps->current, NEGATE_NODE);
    }

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    if(parsesym(ps, adrsym))
    {
        ps->current = add_parsetree_child(ps->current, PARSENODE(Address));

        RUN_PARSER(parse_ident(ps), 0);

        if(parsesym(ps, lbracketsym))
        {
            RUN_PARSER(parse_expression(ps), 17);
            RUN_PARSER(parsesym(ps, rbracketsym), 0);
        }

        ps->current = me;
        return true;
    }
    else if(parsesym(ps, getatsym))
    {
        ps->current = add_parsetree_child(ps->current, PARSENODE(GetFromAdr));

        RUN_PARSER(parse_ident(ps), 0);

        if(parsesym(ps, lbracketsym))
        {
            RUN_PARSER(parse_expression(ps), 17);
            RUN_PARSER(parsesym(ps, rbracketsym), 0);
        }

        ps->current = me;
        return true;
    }
    else if(parse_ident(ps))
    {
        if(parsesym(ps, lbracketsym))
        {
            RUN_PARSER(parse_expression(ps), 17);
            RUN_PARSER(parsesym(ps, rbracketsym), 0);
        }
#else
    if(parse_ident(ps))
    {
#endif
        ps->current = me;
        return true;
    }
    else if(parse_number(ps))
    {
        ps->current = me;
        return true;
    }
    else if(parsesym(ps, lparentsym))
    {
        RUN_PARSER(parse_expression(ps), 17);
        ps->current = me;
        RUN_PARSER(parsesym(ps, rparentsym), 21);
        return true;
    }
#ifdef PL_PARAM_EXTENSION_SUPPORT
    else if(parsesym(ps, sizeofsym))
    {
        ps->current = add_parsetree_child(ps->current, PARSENODE(SizeOf));
        RUN_PARSER(parse_ident(ps), 0);
        ps->current = me;
        return true;
    }
    else if(parsesym(ps, callsym))
    {
        ps->current = add_parsetree_child(ps->current, STATEMENT_LIST_NODE);
        ParseTree *me2 = ps->current;
        ps->current->type = Call;

        RUN_PARSER(parse_ident(ps), 0);
        ps->current = me2;

        RUN_PARSER(parsesym(ps, lparentsym), 0);

        if(!parsesym(ps, rparentsym))
        {
            RUN_PARSER(parse_expression(ps), 0);
            ps->current = me2; 

            while(parsesym(ps, commasym))
            {
                RUN_PARSER(parse_expression(ps), 0);
                ps->current = me2;
            }

            RUN_PARSER(parsesym(ps, rparentsym), 0);
        }

        ps->current = me;

        return true;

    }
#endif

    return false;
}

// ** Terminal Parsers **//

#ifdef DEBUG_PARSER_CALLS
#define DEBUG_TERMINAL_PARSER(...) fprintf(stderr, __VA_ARGS__);
#endif
#ifndef DEBUG_PARSER_CALLS
#define DEBUG_TERMINAL_PARSER(...)
#endif

bool parsesym(plParser *ps, token_type symbol)
{
    if(ps->index >= ps->symct)
        return false;

    if(ps->symbols[ps->index]->symtype == (int)symbol)
    {
        DEBUG_TERMINAL_PARSER("Parsing symbol: %s\n", ps->symbols[ps->index]->symbol);

        ps->index += 1;
        return true;
    }
    return false;
}

bool parse_ident(plParser *ps)
{
    if(ps->index >= ps->symct)
        return false;

    if(ps->symbols[ps->index]->symtype == identsym)
    {

        DEBUG_TERMINAL_PARSER("Parsing identifier: %s\n", ps->symbols[ps->index]->symbol);

        string id = ps->symbols[ps->index]->symbol;
        ps->current = add_parsetree_child(ps->current, IDENT_NODE(id,0));
        ps->index += 1;

        return true;
    }
    return false;
}

#ifdef PL_SYNTAX_EXTENSION_SUPPORT
bool parse_literal(plParser *ps)
{
    if(parse_sliteral(ps))
    {
        return true;
    }
    else if(parse_cliteral(ps))
    {
        return true;
    }

    return false;
}
bool parse_sliteral(plParser *ps)
{
    if(ps->index >= ps->symct)
        return false;

    if(ps->symbols[ps->index]->symtype == strlitsym)
    {
        ps->current = add_parsetree_child(ps->current, STR_LIT_NODE);
        string lit = ps->symbols[ps->index]->symbol,
               newlit = new_sstr("");
        for(int i=1;i<length_sstr(lit)-1;++i)
        {
            switch(lit[i])
            {
                case '\\':
                    switch(lit[i+1])
                    {
                        case 'n':
                            newlit = append_sstr(newlit, '\n');
                            break;
                        case 't':
                            newlit = append_sstr(newlit, '\t');
                            break;
                        case '0':
                            newlit = append_sstr(newlit, '\0');
                            break;
                        default:
                            newlit = append_sstr(newlit, lit[i+1]);
                            break;
                    }
                    ++i;
                    break;
                default:
                    newlit = append_sstr(newlit, lit[i]);
                    break;
            }
        }
        ps->current->strval = newlit;
        ps->index += 1;
        return true;
    }

    return false;
}
bool parse_cliteral(plParser *ps)
{
    if(ps->index >= ps->symct)
        return false;

    if(ps->symbols[ps->index]->symtype == charlitsym)
    {
        ps->current = add_parsetree_child(ps->current, CHAR_LIT_NODE);
        string lit = ps->symbols[ps->index]->symbol,
               newlit = new_sstr("");
        for(int i=1;i<length_sstr(lit)-1;++i)
        {
            switch(lit[i])
            {
                case '\\':
                    switch(lit[i+1])
                    {
                        case 'n':
                            newlit = append_sstr(newlit, '\n');
                            break;
                        case 't':
                            newlit = append_sstr(newlit, '\t');
                            break;
                        case '0':
                            newlit = append_sstr(newlit, '\0');
                            break;
                        default:
                            newlit = append_sstr(newlit, lit[i+1]);
                            break;
                    }
                    ++i;
                    break;
                default:
                    newlit = append_sstr(newlit, lit[i]);
                    break;
            }
        }
        ps->current->strval = newlit;
        ps->index += 1;
        return true;
    }

    return false;
}
#endif

bool parse_rel_op(plParser *ps)
{
    if(ps->index >= ps->symct)
        return false;

    switch(ps->symbols[ps->index]->symtype)
    {
        case eqsym:
            add_parsetree_child(ps->current, EQ_NODE);
            break;
        case neqsym:
            add_parsetree_child(ps->current, NEQ_NODE);
            break;
        case lessym:
            add_parsetree_child(ps->current, LT_NODE);
            break;
        case leqsym:
            add_parsetree_child(ps->current, LTE_NODE);
            break;
        case gtrsym:
            add_parsetree_child(ps->current, GT_NODE);
            break;
        case geqsym:
            add_parsetree_child(ps->current, GTE_NODE);
            break;
        default:
            return false;
    }

    DEBUG_TERMINAL_PARSER("Parsing relational-operator: %s\n", ps->symbols[ps->index]->symbol);

    ps->index += 1;

    return true;
}

bool parse_number(plParser *ps)
{
    if(ps->index >= ps->symct)
        return false;

    if(ps->symbols[ps->index]->symtype == numbersym)
    {
        DEBUG_TERMINAL_PARSER("Parsing number: %s\n", ps->symbols[ps->index]->symbol);

        add_parsetree_child(ps->current, NUMBER_NODE(NULL, atoi(ps->symbols[ps->index]->symbol)));
        ps->index += 1;
        return true;
    }

    return false;
}

