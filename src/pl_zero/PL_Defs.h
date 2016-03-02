#ifndef PL_DEFS_INCLUDED
#define PL_DEFS_INCLUDED

/* #define PL_PARAM_EXTENSION_SUPPORT */
/* #define PL_SYNTAX_EXTENSION_SUPPORT */

typedef enum {
    LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, SIO1, SIO2, SIO3
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    ,
    LODX,
    STOX,
    ADR,
    ADRX,
    GET,
    PUT
#endif
} opcode;

typedef enum {
    RET, NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    , AND, OR, LNOT
#endif
} aluopcode;

typedef enum {
    nulsym = 1, identsym, numbersym, plussym, minussym, multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym, dosym, callsym, constsym, varsym, procsym, writesym, readsym, elsesym
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    ,
    modsym,
    orsym,
    andsym,
    returnsym,
    lbracketsym,
    rbracketsym,
    qmarksym,
    colonsym,
    strlitsym,
    charlitsym,
    charsym,
    hexsym,
    binsym,
    octsym,
    stringsym,
    sizeofsym,
    lnsym,
    lnotsym,
    adrsym,
    getatsym
#endif
} token_type;

typedef enum {
    Root,
    Block, Const_decl, Var_decl, Proc_decl,
    StatementList, Assignment, Read, Write, If, ElseIf, Else, While, Epsilon, Call, Return,
    Expression, Term,
    Factor, Ident, Number, Condition, Odd, Rel_op,
    Multiply, Divide, Modulo, Negate, Unary, Add, Subtract,
    GT, LT, EQ, NE, LTE, GTE
#ifdef PL_SYNTAX_EXTENSION_SUPPORT
    ,
    And, 
    Or,
    Relation,
    Ternary,
    StrLiteral,
    CharLiteral,
    Char,
    Hex,
    Oct,
    Bin,
    String,
    ReadChar,
    ReadString,
    SizeOf,
    WriteLn,
    Address,
    GetFromAdr
#endif
} parse_type;

#endif
