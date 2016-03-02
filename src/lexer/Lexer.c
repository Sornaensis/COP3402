#include <stdlib.h>
#include <stdio.h>

#include <common/Common.h>
#include <lexer/Lexer.h>

// ** SymbolStream Functions **//

SymbolStream *new_symbolstream()
{
    SymbolStream *new = malloc(sizeof(SymbolStream));
    new->lexing_system = NULL;
    new->lextypes = NULL;
    new->lexerct = 0;
    new->symbols = NULL;
    new->symbolct = 0;

    return new;
}

bool add_symbol(SymbolStream *ss, Symbol *s)
{
    if(ss->symbols == NULL)
    {
        ss->symbols = malloc(sizeof(Symbol*) * (ss->symbolct + 1));
        ss->symbols[ss->symbolct] = s;
        ss->symbolct = 1;
        return true;
    }

    ss->symbols = realloc(ss->symbols, sizeof(Symbol*) * (ss->symbolct + 1));
    ss->symbols[ss->symbolct] =  s;
    ss->symbolct += 1;
    return true;
}

void destroy_symbolstream(SymbolStream *ss)
{
    unsigned int i = 0;
    for(;i<ss->symbolct;++i)
    {
        destroy_symbol(ss->symbols[i]);
    }
    for(i=0;i<ss->lexerct;++i)
    {
        destroy_lexnode(ss->lexing_system[i]);
    }

    free(ss);
}

void add_lexer(SymbolStream *ss, LexNode* lexer, int type)
{
    if(ss->lexing_system == NULL)
    {
        ss->lexing_system = malloc(sizeof(LexNode*));
        ss->lextypes = malloc(sizeof(int*));
        ss->lexing_system[0] = lexer;
        ss->lextypes[0] = type;
        ss->lexerct = 1;
        return;
    }

    ss->lexerct += 1;
    ss->lexing_system = realloc(ss->lexing_system, sizeof(LexNode*) * ss->lexerct);
    ss->lextypes = realloc(ss->lextypes, sizeof(int) * ss->lexerct);
    ss->lexing_system[ss->lexerct-1] = lexer;
    ss->lextypes[ss->lexerct-1] = type;
    
}

// ** Symbol Functions ** //

Symbol *new_symbol(string val, int stype)
{
    Symbol *new = malloc(sizeof(Symbol));
    new->symbol = val;
    new->symtype = stype;

    return new;
}

void destroy_symbol(Symbol *s)
{
    destroy_sstr(s->symbol);
    free(s);
}

//** Lexer Automata Functions **//

bool run_automata_r(LexNode* root, string *symbol, string stream, unsigned int *index)
{
    if(root->ct == 0)
        return true;

    if(*index > length_sstr(stream))
        return true;
    
    char c = stream[*index];

    unsigned int i = 0;
    bool match = false;
    *index += 1;
    for(;i<root->ct;++i)
    {
        if(char_match(root->next[i], c))
        {
            match = true;
            bool r = run_automata_r(root->next[i], symbol, stream, index);
            if(r)
            {
                *symbol = append_sstr(*symbol, c);
                return r;
            }
        }
    }
    
    for(i=0;i<root->ct;++i)
    {
        if(root->next[i] != root)
        {
            *index -= 1;
            return root->end;
        }
    }

    if(!match)
        *index -= 1;

    return true;
}

bool run_automata(LexNode* root, string *symbol, string stream, unsigned int *index)
{
    bool ret = run_automata_r(root, symbol, stream, index);
    *symbol = reverse_sstr(*symbol);
    return ret;
}

SymbolStream *read_lexer_from_file(FILE *in)
{
    return NULL;
}

string perform_lex_on_stream(SymbolStream *ss, string stream)
{
    return NULL;
}


LexNode *construct_lexer_from_table(LexNode* lexlist[])
{
    int i = 0, len = 0;
    while(lexlist[len] != NULL)
    {
        ++len;
    }
    for(i=len-1;i>=0;--i)
    {
        if(lexlist[i]->optional)
        {
            lexlist[i]->end = true;
        }
        else
        {
            lexlist[i]->end = true;
            break;
        }
    }
    
    //Connect each node to the node following it
    //Connect it to each following OPTIONAL node first e.g. if aa*b connect a to both a* and b
    for(i=0;i<len;++i)
    {
        int j = 0;
        LexNode* current = lexlist[i];
        for(j=i+1;j<len;++j)
        {
            if(lexlist[j]->optional)
            {
                add_connection(current, lexlist[j]);
            }
            else
            {
                add_connection(current, lexlist[j]);
                break;
            }
        }
    }

    return lexlist[0];

}
LexNode **get_lexer_end_nodesR(LexNode ***visited, LexNode *root)
{
    int* v_len = (int*)(*visited)[0];
    int i;
    // if we have already visited root then GTFO
    for(i=1;i<=*v_len;++i)
    {
        if(root == (*visited)[i])
            return NULL;
    }
    
    *visited = realloc(*visited, sizeof(LexNode*)*(*v_len+2));
    *v_len += 1;
    (*visited)[*v_len] = root;

    LexNode **end_nodes = NULL;
    int end_node_ct = 0;

    if(root->end == true)
    {
        end_nodes = malloc(sizeof(LexNode*));
        end_nodes[0] = root;
        end_node_ct = 1;
    }

    for(i=0;i<root->ct;++i)
    {
        LexNode **sub_end_nodes = get_lexer_end_nodesR(visited, root->next[i]);
        if(sub_end_nodes != NULL)
        {
            int length = 0;
            for(;sub_end_nodes[length]!=NULL;++length);
            int v_i,s_i;
            // Check each sub end node to see if it is already in the list
            for(s_i=0;s_i<length;++s_i)
            {
                bool already = false;
                for(v_i=0;v_i<end_node_ct;++v_i)
                {
                    if(sub_end_nodes[s_i] == end_nodes[v_i])
                    {
                        already = true;
                        break;
                    }
                }
                if(!already)
                {
                    if(end_nodes == NULL)
                    {
                        end_nodes = malloc(sizeof(LexNode*));
                        end_nodes[0] = sub_end_nodes[s_i];
                        end_node_ct = 1;
                    }
                    else
                    {
                        end_nodes = realloc(end_nodes, sizeof(LexNode*)*(end_node_ct+1));
                        end_nodes[end_node_ct] = sub_end_nodes[s_i];
                        ++end_node_ct;
                    }
                }
            }
            free(sub_end_nodes);
        }
    }

    if(end_nodes != NULL)
    {
        end_nodes = realloc(end_nodes, sizeof(LexNode*)*(end_node_ct+1));
        end_nodes[end_node_ct] = NULL;
    }

    return end_nodes;
}

LexNode **get_lexer_end_nodes(LexNode *root)
{
    LexNode **visited = malloc(sizeof(LexNode*));
    int length = 0;
    visited[0] = (LexNode*)&length;

    LexNode **ret = get_lexer_end_nodesR(&visited, root);
    free(visited);
    return ret;
}


LexNode *alphabetical_character()
{
    return new_lexnode(ALPHABET);
}

LexNode *numeric_character()
{
    return new_lexnode(NUMERALS);
}

LexNode *alphanumeric_character()
{
    return new_lexnode(ALPHABET NUMERALS);
}

LexNode *whitespace()
{
    return new_lexnode(WHITESPACE);
}

LexNode *char_literal()
{
    LexNode *root = new_lexnode(""), 
            *beginquote = new_lexnode("\'"),
            *char_lit = new_lexnode(ALPHABET NUMERALS " ![]{}@#$%^&*()-_+=:;<>,./?|`~"),
            *escape_slash = new_lexnode("\\"),
            *escape_char  = new_lexnode("\\nt\'\"0"),
            *endquote = new_lexnode("\'");

    add_connection(root, beginquote);

    add_connection(beginquote, char_lit);
    add_connection(beginquote, escape_slash);

    add_connection(char_lit, endquote);

    add_connection(escape_slash, escape_char);

    add_connection(escape_char, endquote);

    endquote->end = true;

    return root;
}

LexNode *string_literal()
{
    LexNode *root = new_lexnode(""), 
            *beginquote = new_lexnode("\""),
            *char_lit = new_lexnode(ALPHABET NUMERALS " ![]{}@#$%^&*()-_+=:;<>,./?|`~"),
            *escape_slash = new_lexnode("\\"),
            *escape_char  = new_lexnode("\\nt\'\""),
            *endquote = new_lexnode("\"");

    add_connection(root, beginquote);

    add_connection(beginquote, char_lit);
    add_connection(beginquote, escape_slash);
    add_connection(beginquote, endquote);

    add_connection(char_lit, char_lit);
    add_connection(char_lit, escape_slash);
    add_connection(char_lit, endquote);

    add_connection(escape_slash, escape_char);

    add_connection(escape_char, char_lit);
    add_connection(escape_char, escape_slash);
    add_connection(escape_char, endquote);

    endquote->end = true;

    return root;
}

LexNode *c_style_comments()
{
    LexNode *root = new_lexnode("");
    LexNode *slash = new_lexnode("/");
    LexNode *star = new_lexnode("*");
    LexNode *wildcard = new_lexnode(".");
    wildcard->wildcard = true;
    LexNode *endstar = new_lexnode("*");
    LexNode *endslash = new_lexnode("/");

    add_connection(root, slash);
    add_connection(slash, star);
    add_connection(star, wildcard);
    add_connection(wildcard, endstar);
    add_connection(endstar, endslash);
    
    add_connection(wildcard, wildcard);

    endslash->end = true;
    
    return root;
}

LexNode *repeat_node(LexNode *l)
{
    add_connection(l, l);
    return l;
}

LexNode *optional_node(LexNode *l)
{
    l->optional = true;
    return l;
}

