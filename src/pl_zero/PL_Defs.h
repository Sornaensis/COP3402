#ifndef PL_DEFS_INCLUDED
#define PL_DEFS_INCLUDED

typedef enum {
    nulsym = 1, identsym, numbersym, plussym, minussym, multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym, dosym, callsym, constsym, varsym, procsym, writesym, readsym, elsesym
} token_type;

typedef enum {
    Root,
    Block, Const_decl, Var_decl, Proc_decl,
    StatementList, Assignment, Read, Write, If, Then, Else, While, Epsilon, Call,
    Expression, Term,
    Factor, Ident, Number, Condition, Odd, Rel_op,
    Multiply, Divide, Negate, Unary, Add, Subtract,
    GT, LT, EQ, NE, LTE, GTE
} parse_type;

#endif
