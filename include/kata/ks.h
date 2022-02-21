/* kata/ks.h - KataScript compiler, and interface (prefix ks_ and KS_)
 *
 *
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_KS_H
#define KATA_KS_H

#ifndef KATA_API_H
  #include <kata/api.h>
#endif

////////////////////////////////////////////////////////////////////////////////


enum {

    // end-of-file
    KS_TOK_EOF       = 0,

    // ignorable line comment '# ...*'
    KS_TOK_COMMENT,

    // arbitrary precision integer constant
    KS_TOK_INT,
    // arbitrary precision floating point constant
    KS_TOK_FLOAT,
    // string literal, like "hello, world"
    KS_TOK_STR,

    // name reference, typically to a variable, function, or type
    // can also be keyword
    KS_TOK_NAME,

    // string interpolation, like $"hey {name} {3+5}" == "hey user 8"
    KS_TOK_STR_CASH,

    // (
    KS_TOK_LPAR,
    // )
    KS_TOK_RPAR,
    // {
    KS_TOK_LBRC,
    // }
    KS_TOK_RBRC,
    // [
    KS_TOK_LBRK,
    // ]
    KS_TOK_RBRK,
    // <-
    KS_TOK_LARW,
    // ->
    KS_TOK_RARW,

    // .
    KS_TOK_DOT,
    // ,
    KS_TOK_COMMA,
    // :
    KS_TOK_COLON,
    // ;
    KS_TOK_SEMI,

    // +
    KS_TOK_PLUS,
    // ++
    KS_TOK_PLUSPLUS,
    // -
    KS_TOK_MINUS,
    // --
    KS_TOK_MINUSMINUS,
    // *
    KS_TOK_STAR,
    // **
    KS_TOK_STARSTAR,
    // /
    KS_TOK_SLASH,
    // //
    KS_TOK_SLASHSLASH,
    // %
    KS_TOK_PERC,
    // %%
    KS_TOK_PERCPERC,
    // ^
    KS_TOK_UP,
    // ~
    KS_TOK_SQIG,
    // @
    KS_TOK_AT,
    // !
    KS_TOK_BANG,
    // ?
    KS_TOK_QUES,
    // ??
    KS_TOK_QUESQUES,
    // |
    KS_TOK_PIPE,
    // ||
    KS_TOK_PIPEPIPE,
    // &
    KS_TOK_AND,
    // &&
    KS_TOK_AMPAMP,
    // <
    KS_TOK_LT,
    // <<
    KS_TOK_LTLT,
    // >
    KS_TOK_GT,
    // >>
    KS_TOK_GTGT,

    // =
    KS_TOK_EQ,
    // ==
    KS_TOK_EQEQ,
    // :=
    KS_TOK_DEFEQ,
    // +=
    KS_TOK_PLUSEQ,
    // -=
    KS_TOK_MINUSEQ,
    // *=
    KS_TOK_STAREQ,
    // *=
    KS_TOK_SLASHEQ,
    // //=
    KS_TOK_SLASHSLASHEQ,
    // %=
    KS_TOK_PERCEQ,
    // ^=
    KS_TOK_UPEQ,
    // ~=
    KS_TOK_SQIGEQ,
    // @=
    KS_TOK_ATEQ,
    // !=
    KS_TOK_BANGEQ,
    // ?=
    KS_TOK_QUESEQ,
    // <=
    KS_TOK_LTEQ,
    // >=
    KS_TOK_GTEQ,
};

// individual token 
typedef struct ks_tok {

    // the kind of token, see 'KS_TOK_*'
    s32 kind;

    // position and length, in bytes of the source code
    s32 posb, lenb;

    // line, column, and length in characters of the source code
    s32 line, col, lenc;

}* ks_tok;

KATA_API ks_tok
ks_tok_new(s32 kind, s32 posb, s32 lenb, s32 line, s32 col, s32 lenc);

// describes the kind/type of an AST
enum {
    
    // error/uninitialized
    KS_AST_NONE        = 0,

    // a constant value (int, float, string, etc)
    // sub := (val: any, )
    KS_AST_VAL,

    // attribute reference
    // sub := (val: any, name: str)
    KS_AST_ATTR,

    // a function call, indicated as:
    // sub[0](*sub[1:]) = sub[0](sub[1], sub[2], ...)
    KS_AST_CALL,

    // a name reference (variable, function, type, etc)
    // sub := (name: str, )
    KS_AST_NAME,

    // 'import' block
    // TODO: document how to configure
    // sub := (name: ks.ast, config: ks.ast = none)
    KS_AST_IMPORT,

    // 'ret' block
    // sub := (val: ks.ast, )
    KS_AST_RET,

    // 'throw' block
    // sub := (val: ks.ast, )
    KS_AST_THROW,


    // block of statements
    KS_AST_BLOCK,

    // 'if' block
    // sub := (cond: ks.ast, bodyT: ks.ast, bodyF: ks.ast = [], )
    KS_AST_IF,

    // 'while' block
    // sub := (cond: ks.ast, bodyT: ks.ast, bodyF: ks.ast = [], )
    KS_AST_WHILE,

    // 'for' block
    // sub := (cond: ks.ast, bodyT: ks.ast, bodyF: ks.ast = [], )
    KS_AST_FOR,

    // +
    KS_AST_ADD,
    // ++
    KS_AST_ADDADD,
    // -
    KS_AST_SUB,
    // --
    KS_AST_SUBSUB,
    // *
    KS_AST_MUL,
    // **
    // TODO: use this?
    _KS_AST_MULMUL,
    // /
    KS_AST_DIV,
    // //
    KS_AST_FLOORDIV,
    // %
    KS_AST_MOD,
    // %%
    KS_AST_MODMOD,
    // ^
    KS_AST_POW,
    // ~
    KS_AST_SQIG,
    // @
    KS_AST_MATMUL,
    // !
    KS_AST_NOT,
    // ?
    KS_AST_QUES,
    // ??
    KS_AST_QUESQUES,
    // |
    KS_AST_PIPE,
    // ||
    KS_AST_PIPEPIPE,
    // &
    KS_AST_AND,
    // &&
    KS_AST_ANDAND,
    // <
    KS_AST_LT,
    // <<
    KS_AST_LTLT,
    // >
    KS_AST_GT,
    // >>
    KS_AST_GTGT,
    // <=
    KS_AST_LE,
    // >=
    KS_AST_GE,

    // =
    KS_AST_EQ,
    // ==
    KS_AST_EQEQ,
    // :=
    KS_AST_DEFEQ,
    // +=
    KS_AST_ADDEQ,
    // -=
    KS_AST_SUBEQ,
    // *=
    KS_AST_MULEQ,
    // /=
    KS_AST_DIVEQ,
    // //=
    KS_AST_FLOORDIVEQ,
    // %=
    KS_AST_MODEQ,
    // ^=
    KS_AST_POWEQ,
    // ~=
    KS_AST_SQIGEQ,
    // @=
    KS_AST_MATMULEQ,
    // !=
    KS_AST_NOTEQ,
    // ?=
    KS_AST_QUESEQ,


};

// abstract syntax tree node for a KataScript program
typedef struct ks_ast {

    // the kind of node, see 'KS_AST_*'
    u32 kind;

    // token node
    ks_tok tok;

    // sub nodes of the AST (this depends on the kind, is typically
    //   a constant or a tuple)
    //ktuple sub;
    kobj sub;

}* ks_ast;

// create a new AST node, with a tuple as the sub node
KATA_API ks_ast
ks_ast_new(ks_tok tok, u32 kind, int nsub, kobj* sub);

// create a new AST node, absorbing references to 'sub'
// NOTE: this always removes the references to 'sub', and handles the case where
//         some are NULL (if any NULLs are found, a reference is removed from others
//         and no result is returned)
KATA_API ks_ast
ks_ast_newz(ks_tok tok, u32 kind, int nsub, kobj* sub);

// create a new AST node wrapping a constant
KATA_API ks_ast
ks_ast_wrap(ks_tok tok, kobj sub);
// create a new AST node wrapping a constant
KATA_API ks_ast
ks_ast_wrapx(ks_tok tok, u32 kind, kobj sub);

// return a constant reference to a string for the given AST kind
KATA_API const char*
ks_ast_kindname(u32 kind);


////////////////////////////////////////////////////////////////////////////////

// transform KataScript file contents into tokens, storing and reallocating '*ptoks'
//   as needed
// NOTE: the number of tokens is returned, or if the return value is <0, an error occured
// NOTE: '*ptoks' needs to be freed if there was or wasn't an error! (use 'kmem_free()')
// if you want a list of the tokens, call 'klist_newz(len, toks)', and then 'kmem_free(toks)'
//   which will transfer the references
KATA_API s32
ks_lex(kstr filename, kstr src, ks_tok** ptoks);

// parse a KataScript file contents into an AST
KATA_API ks_ast
ks_parse(kstr filename, kstr src, s32* pntoks, ks_tok** ptoks);


////////////////////////////////////////////////////////////////////////////////

KATA_API ktype
Ks_ast,
Ks_tok
;

#endif // KATA_KS_H
