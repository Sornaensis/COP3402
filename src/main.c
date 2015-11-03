#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <common/Common.h>
#include <string/SStr.h>

#include "pl_zero/PL_Lexer.h"
#include "pl_zero/PL_Parser.h"

int main ( int argc, char** argv )
{

    /* SymbolStream *somestream = init_pl0_lexer(); */
    /* LexNode **endings = get_lexer_end_nodes(somestream->lexing_system[1]); */
    /* for(int i=0;endings[i] != NULL;++i) */
    /*     printf("%s ", endings[i]->matches); */
    /* puts(""); */

    FILE *input;
    if(argc > 1)
    {
       input = fopen(argv[1], "r");
    }
    else
    {
        input = fopen("input.txt", "r");
    }

    if(input)
    {
        FILE *cleanout = fopen("cleaninput.txt", "w"),
             *lextable = fopen("lexemetable.txt", "w"),
             *tokenlist = fopen("tokenlist.txt", "w");
        SymbolStream *pl0stream = init_pl0_lexer();
        string err = (run_pl0_lexer(pl0stream, input)); 
        if(length_sstr(err) > 0)
        {
            printf("%s\n", err);
            destroy_sstr(err);
            fclose(input);
            fclose(tokenlist);
            fclose(lextable);
            fclose(cleanout);
            exit(1);
        }

        unsigned int i = 0;
        for(;i<pl0stream->symbolct;++i)
        {
            if(pl0stream->symbols[i]->symtype != -1)
            {
                fprintf(cleanout, "%s",pl0stream->symbols[i]->symbol);
            }
        }

        destroy_symbolstream(pl0stream);
        destroy_sstr(err);
        fclose(cleanout);
        cleanout = fopen("cleaninput.txt", "r");
        
        pl0stream = init_pl0_lexer();
        err = (run_pl0_lexer(pl0stream, cleanout)); 
        if(length_sstr(err) > 0)
        {
            printf("%s\n", err);
            destroy_sstr(err);
            fclose(input);
            fclose(tokenlist);
            fclose(lextable);
            fclose(cleanout);
            exit(1);
        }

        int scount = 0;
        Symbol **parsersyms = calloc(pl0stream->symbolct, sizeof(Symbol*));
        for(i=0;i<pl0stream->symbolct;++i)
        {
            if(pl0stream->symbols[i]->symtype > 1)
            {
                printf("(\'%s\',%d) ", pl0stream->symbols[i]->symbol, pl0stream->symbols[i]->symtype);
                parsersyms[scount] = pl0stream->symbols[i];
                ++scount;
            }
        }
        puts("");

        plParser *the_parser = new_pl_parser(parsersyms, scount);
        parse_program(the_parser);

        the_parser->current = the_parser->current->children[2]->children[0];
        printf("Top level node type: %d | Strval: %s | Intval: %d\n", 
                        the_parser->current->type,
                        the_parser->current->strval,
                        the_parser->current->intval);

        for(i=0;i<the_parser->current->childct;++i)
        {
            printf("Second level node type: %d\n", the_parser->current->children[i]->type);
        }

        fprintf(lextable, "%-20s %-20s\n", "Lexeme", "token type");
        for(i=0;i<pl0stream->symbolct;++i)
        {
            if(pl0stream->symbols[i]->symtype > 1)
                fprintf(lextable, "%-20s %-20d\n",pl0stream->symbols[i]->symbol, pl0stream->symbols[i]->symtype);
        }

        for(i=0;i<pl0stream->symbolct;++i)
        {
            if(pl0stream->symbols[i]->symtype > 1)
            {
                fprintf(tokenlist, "%d ", pl0stream->symbols[i]->symtype);
                if(pl0stream->symbols[i]->symtype == identsym || pl0stream->symbols[i]->symtype == numbersym)
                {
                    fprintf(tokenlist, "%s ", pl0stream->symbols[i]->symbol);
                }
            }
        }
        fprintf(tokenlist, "\n");

        destroy_symbolstream(pl0stream);
        destroy_sstr(err);

        fclose(input);
        fclose(tokenlist);
        fclose(lextable);
        fclose(cleanout);
    }

}

