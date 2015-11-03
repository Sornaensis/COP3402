#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <common/Common.h>
#include <lexer/LexTree.h>
#include <string/SStr.h>


LexNode *new_lexnode(char* cs)
{
    LexNode *new = malloc(sizeof(LexNode));
    new->next       = NULL;
    new->ct         = 0;
    new->matches    = new_sstr(cs);
    new->end        = false;
    new->optional   = false;
    new->wildcard   = false;

    return new;
}


LexNode *copy_lexnode(LexNode* l)
{
    LexNode *new = new_lexnode(l->matches);
    
    return new;
}

typedef struct del_tree
{
    LexNode **del_list;
    size_t  ct;
} DelTree;

void add_to_be_deleted(DelTree *d_track, LexNode *n)
{
    unsigned i=0;
    for(;i<d_track->ct;++i)
    {
        if(n == d_track->del_list[i])
            return;
    }

    d_track->del_list = realloc(d_track->del_list, sizeof(LexNode*) * (d_track->ct + 1));
    d_track->del_list[d_track->ct] = n;
    d_track->ct += 1;

    for(i=0;i<n->ct;++i)
    {
        add_to_be_deleted(d_track, n->next[i]);
    }
}

void destroy_lexnode(LexNode* root)
{
    unsigned int i = 0;

    DelTree *d_track = malloc(sizeof(DelTree));
    d_track->del_list = malloc(sizeof(LexNode*));
    d_track->ct      = 1;
    d_track->del_list[0] = root;
    for(;i<root->ct;++i)
    {
        add_to_be_deleted(d_track, root->next[i]);
    } 

    for(i=0;i<d_track->ct;++i)
    {
        destroy_sstr(d_track->del_list[i]->matches);
        free(d_track->del_list[i]);
    }
}

bool add_connection(LexNode* n, LexNode* c)
{
    if (n->next == NULL)
    {
        n->next = malloc(sizeof(LexNode*));
        n->ct = 1;
        n->next[0] = c;
        return true;
    }

    int i = 0;
    for(;i<n->ct;++i)
    {
        if(n->next[i] == c)
            return false;
    }

    n->ct += 1;
    n->next = realloc(n->next, sizeof(LexNode*) * n->ct);
    n->next[n->ct-1] = c;
    return true;
}


bool char_match(LexNode* n, char c)
{
    if( n->wildcard || find_in_sstr(n->matches, c) )
    {
        return true;
    }

    return false;
}


