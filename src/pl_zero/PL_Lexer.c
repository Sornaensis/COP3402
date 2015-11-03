#include <stdio.h>
#include <stdlib.h>

#include <lexer/Lexer.h>
#include <string/SStr.h>

#include "PL_Lexer.h"

static string *___PL0_token_type_list = NULL;

int colno_from_pos(string file, unsigned int pos)
{
    int colno = 1;
    unsigned int i= 0;
    for(;i<pos && pos < length_sstr(file);++i)
    {
        if(file[i] == '\n')
           colno = 1;
        else
            ++colno;
    }

    return colno;
}

int lineno_from_pos(string file, unsigned int pos)
{
    int linect = 1;
    unsigned int i= 0;
    for(;i<pos && pos < length_sstr(file);++i)
    {
        if(file[i] == '\n')
            ++linect;
    }

    return linect;
}

SymbolStream *init_pl0_lexer()
{
    if ( ___PL0_token_type_list == NULL )
    {
        ___PL0_token_type_list = malloc(sizeof(string) * 33);
        ___PL0_token_type_list[0]  = new_sstr("nulsym");
        ___PL0_token_type_list[1]  = new_sstr("identsym");
        ___PL0_token_type_list[2]  = new_sstr("numbersym");
        ___PL0_token_type_list[3]  = new_sstr("plussym");
        ___PL0_token_type_list[4]  = new_sstr("minussym");
        ___PL0_token_type_list[5]  = new_sstr("multsym");
        ___PL0_token_type_list[6]  = new_sstr("slashsym");
        ___PL0_token_type_list[7]  = new_sstr("oddsym");
        ___PL0_token_type_list[8]  = new_sstr("eqsym");
        ___PL0_token_type_list[9]  = new_sstr("neqsym");
        ___PL0_token_type_list[10] = new_sstr("lessym");
        ___PL0_token_type_list[11] = new_sstr("leqsym");
        ___PL0_token_type_list[12] = new_sstr("gtrsym");
        ___PL0_token_type_list[13] = new_sstr("geqsym");
        ___PL0_token_type_list[14] = new_sstr("lparentsym");
        ___PL0_token_type_list[15] = new_sstr("rparentsym");
        ___PL0_token_type_list[16] = new_sstr("commasym");
        ___PL0_token_type_list[17] = new_sstr("semicolonsym");
        ___PL0_token_type_list[18] = new_sstr("periodsym");
        ___PL0_token_type_list[19] = new_sstr("becomesym");
        ___PL0_token_type_list[20] = new_sstr("beginsym");
        ___PL0_token_type_list[21] = new_sstr("endsym");
        ___PL0_token_type_list[22] = new_sstr("ifsym");
        ___PL0_token_type_list[23] = new_sstr("thensym");
        ___PL0_token_type_list[24] = new_sstr("whilesym");
        ___PL0_token_type_list[25] = new_sstr("dosym");
        ___PL0_token_type_list[26] = new_sstr("callsym");
        ___PL0_token_type_list[27] = new_sstr("constsym");
        ___PL0_token_type_list[28] = new_sstr("varsym");
        ___PL0_token_type_list[29] = new_sstr("procsym");
        ___PL0_token_type_list[30] = new_sstr("writesym");
        ___PL0_token_type_list[31] = new_sstr("readsym");
        ___PL0_token_type_list[32] = new_sstr("elsesym");
    }

    SymbolStream *pl0_symbols = new_symbolstream();
    
    add_lexer(pl0_symbols, c_style_comments(), -1);

    // Invalid identifier
    add_lexer(pl0_symbols, 
             CREATE_LEXER(
                         _PLUS(numeric_character()),
                         _PLUS(alphabetical_character()),
                         _STAR(alphanumeric_character())
                         ), -3);

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("<"),
                         new_lexnode(">")
                         
                         ), neqsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode(":"),
                         new_lexnode("=")
                         
                         ), becomessym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("<"),
                         new_lexnode("=")
                         
                         ), leqsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode(">"),
                         new_lexnode("=")
                         
                         ), geqsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode(".")
                         ), periodsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode(";")
                         ), semicolonsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("+")
                         ), plussym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("-")
                         ), minussym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("*")
                         ), multsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("/")
                         ), slashsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("=")
                         ), eqsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode(">")
                         ), gtrsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("<")
                         ), lessym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("(")
                         ), lparentsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode(")")
                         ), rparentsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode(",")
                         ), commasym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         numeric_character(),
                         _STAR(numeric_character())
                         ), numbersym);

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         alphabetical_character(),
                         _STAR(alphanumeric_character())
                         ), identsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("e"),
                         new_lexnode("l"),
                         new_lexnode("s"),
                         new_lexnode("e")
                         
                         ), elsesym );
    
    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("o"),
                         new_lexnode("d"),
                         new_lexnode("d")
                         
                         ), oddsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("r"),
                         new_lexnode("e"),
                         new_lexnode("a"),
                         new_lexnode("d")
                         
                         ), readsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("w"),
                         new_lexnode("r"),
                         new_lexnode("i"),
                         new_lexnode("t"),
                         new_lexnode("e")
                         
                         ), writesym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("p"),
                         new_lexnode("r"),
                         new_lexnode("o"),
                         new_lexnode("c"),
                         new_lexnode("e"),
                         new_lexnode("d"),
                         new_lexnode("u"),
                         new_lexnode("r"),
                         new_lexnode("e")
                         
                         ), procsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("c"),
                         new_lexnode("o"),
                         new_lexnode("n"),
                         new_lexnode("s"),
                         new_lexnode("t")
                         
                         ), constsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("v"),
                         new_lexnode("a"),
                         new_lexnode("r")
                         
                         ), varsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("c"),
                         new_lexnode("a"),
                         new_lexnode("l"),
                         new_lexnode("l")
                         
                         ), callsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("b"),
                         new_lexnode("e"),
                         new_lexnode("g"),
                         new_lexnode("i"),
                         new_lexnode("n")
                         
                         ), beginsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("c"),
                         new_lexnode("a"),
                         new_lexnode("l"),
                         new_lexnode("l")
                         
                         ), callsym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("w"),
                         new_lexnode("h"),
                         new_lexnode("i"),
                         new_lexnode("l"),
                         new_lexnode("e")
                         
                         ), whilesym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("d"),
                         new_lexnode("o")
                         
                         ), dosym );

    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("t"),
                         new_lexnode("h"),
                         new_lexnode("e"),
                         new_lexnode("n")
                         
                         ), thensym );
    
    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("i"),
                         new_lexnode("f")
                         
                         ), ifsym );
    
    add_lexer(pl0_symbols,
             CREATE_LEXER(
                         new_lexnode("e"),
                         new_lexnode("n"),
                         new_lexnode("d")
                         
                         ), endsym );
    
    add_lexer(pl0_symbols,
            CREATE_LEXER(
                        _PLUS(whitespace())
                ), nulsym );


    return pl0_symbols;
}

string run_pl0_lexer(SymbolStream *lexer, FILE *in)
{

    string errormsg = new_sstr("");
    string stream = new_sstr("");
    int c = fgetc(in);
    while(c != EOF)
    {
        stream = append_sstr(stream, c);
        c = fgetc(in);
    }

    unsigned int index = 0;
    while(index < length_sstr(stream))
    {
        string token = new_sstr("");
        unsigned int i = 0;
        for(;i<lexer->lexerct;++i)
        {
            bool ret = run_automata(lexer->lexing_system[i], &token, stream, &index);
            if(ret)
            {
                // Parse keywords from identifiers
                if(lexer->lextypes[i] == identsym)
                {
                    string token2 = new_sstr("");
                    unsigned int index2 = 0;
                    unsigned int j = i+1;
                    for(;j<lexer->lexerct-1;++j)
                    {
                        bool ret2 = run_automata(lexer->lexing_system[j], &token2, token, &index2);
                        if(ret2 && index2 == length_sstr(token))
                        {
                            destroy_sstr(token);
                            add_symbol(lexer, new_symbol(token2, lexer->lextypes[j]));
                            token2 = NULL;
                            token = NULL;
                            break;
                        }
                    }
                    if(token2 != NULL)
                    {
                        destroy_sstr(token2);
                        /* add_symbol(lexer, new_symbol(token, lexer->lextypes[i])); */
                        /* token = NULL; */
                    }
                }
                
                if(token != NULL)
                {
                    if(lexer->lextypes[i] == -3)
                    {
                        destroy_sstr(errormsg);
                        char buf[2048];

                        sprintf(buf, "--FATAL--\nInvalid identifier \'%s\' on line %d, column %d", 
                                    token,
                                    lineno_from_pos(stream, index - length_sstr(token)),
                                    colno_from_pos(stream, index - length_sstr(token)));
                        errormsg = new_sstr(buf);
                        destroy_sstr(token);
                        return errormsg;
                    }
                    else if (lexer->lextypes[i] == identsym && length_sstr(token) > 11)
                    {
                        destroy_sstr(errormsg);
                        char buf[2048];

                        sprintf(buf, "--FATAL--\nIdentifier \'%s\' is too long on line %d, column %d", 
                                    token,
                                    lineno_from_pos(stream, index - length_sstr(token)),
                                    colno_from_pos(stream, index - length_sstr(token)));
                        errormsg = new_sstr(buf);
                        destroy_sstr(token);
                        return errormsg;
                    }
                    else if (lexer->lextypes[i] == numbersym && length_sstr(token) > 5)
                    {
                        destroy_sstr(errormsg);
                        char buf[2048];

                        sprintf(buf, "--FATAL--\nLiteral \'%s\' is too long on line %d, column %d", 
                                    token,
                                    lineno_from_pos(stream, index - length_sstr(token)),
                                    colno_from_pos(stream, index - length_sstr(token)));
                        errormsg = new_sstr(buf);
                        destroy_sstr(token);
                        return errormsg;
                    }
                    add_symbol(lexer, new_symbol(token, lexer->lextypes[i]));
                    token = NULL;
                }

                break;

            }
        }
        if(token != NULL && index < length_sstr(stream))
        {
            destroy_sstr(errormsg);
            char buf[2048];

            sprintf(buf, "--FATAL--\nLexer error uknown character \'%c\' (0x%X) on line %d, column %d", 
                        stream[index],
                        stream[index],
                        lineno_from_pos(stream, index),
                        colno_from_pos(stream, index)
                        );
            errormsg = new_sstr(buf);
            return errormsg;
        }
        
    }

    return errormsg;
}

string get_type_string(token_type t)
{
    if ( ___PL0_token_type_list != NULL )
    {
        return ___PL0_token_type_list[t - 1];
    }

    return NULL;
}
