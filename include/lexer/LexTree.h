#ifndef LEX_TREE_H_INCLUDED
#define LEX_TREE_H_INCLUDED

#include <common/Common.h>
#include <string/SStr.h>

typedef struct lexautomata LexNode;

typedef struct lexautomata
{
    LexNode        **next;
    unsigned int     ct;
    bool             end,
                     wildcard,
                     optional;
    string           matches;
} LexNode;

LexNode *new_lexnode(char* cs);
LexNode *copy_lexnode(LexNode *l);
void destroy_lexnode(LexNode* root);
bool add_connection(LexNode* n, LexNode* c);
bool char_match(LexNode* n, char c);

#endif
