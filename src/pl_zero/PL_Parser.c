#include <stdlib.h>

#include <common/Common.h>
#include <string/SStr.h>

#include "PL_Parser.h"

// ** Parse Tree Functions **//


ParseTree *new_parsetree(ParseTree t)
{
    ParseTree *new = malloc(sizeof(ParseTree));
    memcpy(new, &t, sizeof(ParseTree));

    return new;
}

ParseTree *add_parsetree_child(ParseTree *node, ParseTree *n)
{
    n->parent = node;

    if(node->children == NULL)
    {
        node->children = malloc(sizeof(ParseTree*));
        node->children[0] = n;
        node->childct = 1;
        return n;
    }

    node->children = realloc(node->children, sizeof(ParseTree*) * (node->childct + 1));
    node->children[node->childct]  = n;
    node->childct += 1;

    return n;
}

void delete_parsetree_node(ParseTree *parent, ParseTree *child)
{
    ParseTree **newchildren = malloc(sizeof(ParseTree*)*(parent->childct-1));
    unsigned int i = 0, d = 0, q = 0;
    for(;i<parent->childct && d < (parent->childct-1);++i,++d)
    {
        if(parent->children[i] != child)
        {
            newchildren[d] = parent->children[i];
        }
        else
        {
            q = i;
            --d;
        }
    }

    if(i != parent->childct - 1)
    {
        fprintf(stderr, "ERROR DELETING NON EXISTENT CHILD PARSETREE\n");
        exit(1);
    }

    destroy_parsetree(parent->children[q]);
    free(parent->children);
    parent->children = newchildren;
    parent->childct -= 1;
}

void destroy_parsetree(ParseTree *node)
{
    destroy_sstr(node->strval);
    free(node->children);
}

// ** plParser Struct Functions **//

plParser *new_pl_parser(Symbol **symbols, unsigned int count)
{
    plParser *new = malloc(sizeof(plParser));
    new->symbols = symbols;
    new->index = 0;
    new->symct = count;

    new->current = ROOT_NODE;

    return new;
}


static string __parserrors[1] = {"Generic error has occured"};

// ** Nonterminal Parsers **//

bool parse_program(plParser *ps)
{
    RUN_PARSER(parse_block(ps),0);
    RUN_PARSER(parsesym(ps, periodsym),0);

    return true;
}

bool parse_block(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, BLOCK_NODE);
    ParseTree *me = ps->current;

    parse_const_dec(ps);
    ps->current = me;
    parse_var_dec(ps);
    ps->current = me;

    while(parse_proc_dec(ps))
    {
        ps->current = me;
    }

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

    RUN_PARSER(parse_ident(ps), 0);
    ps->current = me;

    RUN_PARSER(parsesym(ps, eqsym), 0);

    RUN_PARSER(parse_number(ps), 0);
    ps->current = me;

    while(parsesym(ps, commasym))
    {
        RUN_PARSER(parse_ident(ps), 0);
        ps->current = me;

        RUN_PARSER(parsesym(ps, eqsym), 0);

        RUN_PARSER(parse_number(ps), 0);
        ps->current = me;
    }

    RUN_PARSER(parsesym(ps, semicolonsym),0);

    return true;
}

bool parse_var_dec(plParser *ps)
{
    if(!parsesym(ps, varsym))
        return false;

    ps->current = add_parsetree_child(ps->current, VAR_DEC_NODE);
    ParseTree *me = ps->current;

    RUN_PARSER(parse_ident(ps), 0);
    ps->current = me;

    while(parsesym(ps, commasym))
    {
        RUN_PARSER(parse_ident(ps), 0);
        ps->current = me;
    }

    RUN_PARSER(parsesym(ps, semicolonsym), 0);

    return true;
}

bool parse_proc_dec(plParser *ps)
{
    if(!parsesym(ps, procsym))
        return false;

    ps->current = add_parsetree_child(ps->current, PROC_DEC_NODE);
    ParseTree *me = ps->current;

    RUN_PARSER(parse_ident(ps), 0);
    ps->current = me;

    RUN_PARSER(parsesym(ps, semicolonsym), 0);

    RUN_PARSER(parse_block(ps), 0);
    ps->current = me;

    return true;
}

bool parse_statement(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, ASSIGN_NODE);
    ParseTree *me = ps->current;

    if(parse_ident(ps))
    {
        ps->current = me;
        RUN_PARSER(parsesym(ps, becomessym), 0);

        RUN_PARSER(parse_expression(ps), 5);
        ps->current = me;
        return true;
    }
    else
    {
        ps->current = me;
        if(parsesym(ps, callsym))
        {
            me->type = Call;
            RUN_PARSER(parse_ident(ps), 0);
            ps->current = me;

            return true;
        }
        else if(parsesym(ps, beginsym))
        {
            me->type = StatementList;
            RUN_PARSER(parse_statement(ps), 0);
            ps->current = me;

            while(parsesym(ps, semicolonsym))
            {
                RUN_PARSER(parse_statement(ps), 0);
                ps->current = me;
            }

            RUN_PARSER(parsesym(ps, endsym), 0);
            return true;
        }
        else if(parsesym(ps, ifsym))
        {
            me->type = If;
            RUN_PARSER(parse_condition(ps), 0);
            ps->current = me;

            RUN_PARSER(parsesym(ps, thensym), 0);

            RUN_PARSER(parse_statement(ps), 0);
            ps->current->type = Then;
            ps->current = me;

            if(parsesym(ps, elsesym))
            {
                RUN_PARSER(parse_statement(ps), 0);
                ps->current = me;
            }
            return true;
        }
        else if(parsesym(ps, whilesym))
        {
            me->type = While;
            RUN_PARSER(parse_condition(ps), 0);
            ps->current = me;

            RUN_PARSER(parsesym(ps, dosym), 0);

            RUN_PARSER(parse_statement(ps), 0);
            ps->current = me;
            return true;
        }
        else if(parsesym(ps, readsym))
        {
            me->type = Read;
            RUN_PARSER(parse_ident(ps), 0);
            ps->current = me;
            return true;
        }
        else if(parsesym(ps, writesym))
        {
            me->type = Write;
            RUN_PARSER(parse_ident(ps), 0);
            ps->current = me;
            return true;
        }
    }

    me->type = Epsilon;

    return true;
}

bool parse_condition(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, ODD_NODE);
    ParseTree *me = ps->current;

    if(parsesym(ps, oddsym))
    {
        RUN_PARSER(parse_expression(ps), 0);
        ps->current = me;
        return true;
    }

    RUN_PARSER(parse_expression(ps), 0);
    ps->current = me;

    RUN_PARSER(parse_rel_op(ps), 0);
    ps->current = me;

    RUN_PARSER(parse_expression(ps), 0);
    ps->current = me;

    return true;
}

bool parse_expression(plParser *ps)
{
    ps->current = add_parsetree_child(ps->current, EXPRESSION_NODE);
    ParseTree *me = ps->current;

    if(parsesym(ps, plussym))
    {
        ps->current = add_parsetree_child(ps->current, UNARY_NODE);
    }
    else if(parsesym(ps, minussym))
    {
        ps->current = add_parsetree_child(ps->current, NEGATE_NODE);
    }

    RUN_PARSER(parse_term(ps), 0);
    ps->current = me;

    while(1)
    {
        if(parsesym(ps, plussym))
        {
            RUN_PARSER(parse_term(ps), 0);
            ps->current->type = Add;
            ps->current = me;
        }
        else if(parsesym(ps, minussym))
        {
            RUN_PARSER(parse_term(ps), 0);
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

    RUN_PARSER(parse_factor(ps), 0);
    ps->current = me;

    while(1)
    {
        if(parsesym(ps, multsym))
        {
            RUN_PARSER(parse_factor(ps), 0);
            ps->current->type = Multiply;
            ps->current = me;
        }
        else if(parsesym(ps, slashsym))
        {
            RUN_PARSER(parse_factor(ps), 0);
            ps->current->type = Divide;
            ps->current = me;
        }
        else
        {
            return true;
        }
    }

}

bool parse_factor(plParser *ps)
{
    ParseTree *me = ps->current;

    if(parse_ident(ps))
    {
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
        RUN_PARSER(parse_expression(ps), 0);
        ps->current = me;
        RUN_PARSER(parsesym(ps, rparentsym), 0);
        return true;
    }

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

    if(ps->symbols[ps->index]->symtype == symbol)
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

