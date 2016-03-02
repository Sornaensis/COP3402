#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <common/Common.h>
#include <string/SStr.h>

#include "pl_zero/PL_Lexer.h"
#include "pl_zero/PL_StackMachine.h"
#include "pl_zero/PL_Parser.h"
#include "pl_zero/PL_Generator.h"

void printfile(const char* fname)
{
    FILE* symtable = fopen(fname, "r");

    int c;
    c = fgetc(symtable);
    while(c != EOF)
    {
        printf("%c", c);
        c = fgetc(symtable);
    }
    puts("\n");
}

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
       if(input == NULL) 
       {
           input = fopen("input.txt", "r");
       }
       else
       {
           --argc;
           ++argv;
       }
    }
    else
    {
        input = fopen("input.txt", "r");
    }

    bool fPrintTokenList = false,
         fPrintSymTable  = false,
         fPrintMCode     = false,
         fPrintDisASM    = false,
         fPrintSTrace    = false;

    for(int i=1;i<argc;++i)
    {
        if(argv[i][0] == '-')
        {    
            switch(argv[i][1])
            {
                case 't':
                    fPrintTokenList = true;
                    break;
                case 's':
                    fPrintSymTable = true;
                    break;
                case 'm':
                    fPrintMCode = true;
                    break;
                case 'a':
                    fPrintDisASM = true;
                    break;
                case 'v':
                    fPrintSTrace = true;
                    break;
                default:
                    fprintf(stderr, "Invalid commandline switch \'%s\'\n", argv[i]);
                    exit(1);
            }
        }
    }

    if(input)
    {
        SymbolStream *pl0lexer = init_pl0_lexer(); 
        string err = run_pl0_lexer(pl0lexer, input);
        if(err != NULL && length_sstr(err) > 0)
        {
            fprintf(stderr, "%s\n", err);
            exit(1);
        }

        Symbol **properSymbols = calloc(pl0lexer->symbolct, sizeof(Symbol*));
        unsigned int scount = 0;
        FILE* tokenlist = fopen("tokenlist.txt", "w"),
            * cleaninput = fopen("cleaninput.txt", "w"),
            * lexemetable = fopen("lexemetable.txt", "w");
        fprintf(lexemetable, "%-11s %-10s\n", "Lexme", "token type"); 
        for(unsigned int i=0;i<pl0lexer->symbolct;++i)
        {
            if(pl0lexer->symbols[i]->symtype > 1)
            {
                fprintf(tokenlist, "%d ", pl0lexer->symbols[i]->symtype);
                if(pl0lexer->symbols[i]->symtype < 4)
                    fprintf(tokenlist, "%s ", pl0lexer->symbols[i]->symbol);

                fprintf(lexemetable, "%-11s %-10d\n", pl0lexer->symbols[i]->symbol, pl0lexer->symbols[i]->symtype);

                properSymbols[scount] = pl0lexer->symbols[i];
                ++scount;
            }
            if(pl0lexer->symbols[i]->symtype > -1)
            {
                fprintf(cleaninput, "%s", pl0lexer->symbols[i]->symbol);
            }
        }

        fclose(lexemetable);
        fclose(cleaninput);
        fclose(tokenlist);
        
        if(fPrintTokenList)
            printfile("tokenlist.txt");

        plParser *the_parser = new_pl_parser(properSymbols, scount);
        parse_program(the_parser);

        CodePage *prog_code = generate_code(the_parser->current);
        FILE* outprgrm = fopen("mcode.txt", "w");

        if(fPrintSymTable)
        {
            printfile("symtable.txt");
        }
        
        for(unsigned int i=0;i<prog_code->instct;++i)
        {
            fprintf(outprgrm, "%d %d %d\n", 
                                 prog_code->program[i].op, 
                                 prog_code->program[i].L,
                                 prog_code->program[i].M);

            if(fPrintMCode)
            {
                printf("%d %d %d\n", 
                                    prog_code->program[i].op, 
                                    prog_code->program[i].L,
                                    prog_code->program[i].M);
            }
        }
        fclose(outprgrm);
        fclose(input);


        FILE *in = fopen("mcode.txt", "r"),
             *out = fopen("stacktrace.txt", "w");

        if(!in || !out)
        {
            fprintf(stderr, "Error opening one or more files...\n");
            exit(1);
        }

        PMachine *machine = new_pmachine(in, out);
        bool exitcond;
        for(exitcond=execute_pmn(machine);exitcond;exitcond=execute_pmn(machine));

        fclose(in);
        fclose(out);

        if(fPrintDisASM)
        {
            printfile("acode.txt");
        }

        if(fPrintSTrace)
        {
            printfile("stacktrace.txt");
        }

    }

}

