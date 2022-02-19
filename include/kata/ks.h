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
    KS_TOKEN_EOF       = 0,

    // ignorable line comment '# ...*'
    KS_TOKEN_COMMENT,

    // arbitrary precision integer constant
    KS_TOKEN_INT,
    // arbitrary precision floating point constant
    KS_TOKEN_FLOAT,
    // string literal, like "hello, world"
    KS_TOKEN_STR,

    // name reference, typically to a variable, function, or type
    // can also be keyword
    KS_TOKEN_NAME,

    // string interpolation, like $"hey {name} {3+5}" == "hey user 8"
    KS_TOKEN_STR_CASH,

    // (
    KS_TOKEN_LPAR,
    // )
    KS_TOKEN_RPAR,
    // {
    KS_TOKEN_LBRC,
    // }
    KS_TOKEN_RBRC,
    // [
    KS_TOKEN_LBRK,
    // ]
    KS_TOKEN_RBRK,
    // <-
    KS_TOKEN_LARW,
    // ->
    KS_TOKEN_RARW,

    // .
    KS_TOKEN_DOT,
    // ,
    KS_TOKEN_COMMA,
    // :
    KS_TOKEN_COLON,
    // ;
    KS_TOKEN_SEMI,

    // +
    KS_TOKEN_PLUS,
    // ++
    KS_TOKEN_PLUSPLUS,
    // -
    KS_TOKEN_MINUS,
    // --
    KS_TOKEN_MINUSMINUS,
    // *
    KS_TOKEN_STAR,
    // **
    KS_TOKEN_STARSTAR,
    // /
    KS_TOKEN_SLASH,
    // //
    KS_TOKEN_SLASHSLASH,
    // %
    KS_TOKEN_PERC,
    // %%
    KS_TOKEN_PERCPERC,
    // ^
    KS_TOKEN_UP,
    // ~
    KS_TOKEN_SQIG,
    // @
    KS_TOKEN_AT,
    // !
    KS_TOKEN_BANG,
    // ?
    KS_TOKEN_QUES,
    // ??
    KS_TOKEN_QUESQUES,
    // |
    KS_TOKEN_PIPE,
    // ||
    KS_TOKEN_PIPEPIPE,
    // &
    KS_TOKEN_AMP,
    // &&
    KS_TOKEN_AMPAMP,
    // <
    KS_TOKEN_LT,
    // <<
    KS_TOKEN_LTLT,
    // >
    KS_TOKEN_GT,
    // >>
    KS_TOKEN_GTGT,

    // =
    KS_TOKEN_EQ,
    // ==
    KS_TOKEN_EQEQ,
    // :=
    KS_TOKEN_DEFEQ,
    // +=
    KS_TOKEN_PLUSEQ,
    // -=
    KS_TOKEN_MINUSEQ,
    // *=
    KS_TOKEN_STAREQ,
    // %=
    KS_TOKEN_PERCEQ,
    // ^=
    KS_TOKEN_UPEQ,
    // ~=
    KS_TOKEN_SQIGEQ,
    // @=
    KS_TOKEN_ATEQ,
    // !=
    KS_TOKEN_BANGEQ,
    // ?=
    KS_TOKEN_QUESEQ,
    // <=
    KS_TOKEN_LTEQ,
    // >=
    KS_TOKEN_GTEQ,
};

// individual token 
typedef struct ks_token {

    // the kind of token, see 'KS_TOKEN_*'
    s32 kind;

    // position and length, in bytes of the source code
    s32 posb, lenb;

    // line, column, and length in characters of the source code
    s32 line, col, lenc;

}* ks_token;

KATA_API ks_token
ks_token_new(s32 kind, s32 posb, s32 lenb, s32 line, s32 col, s32 lenc);

// describes the kind/type of an AST
enum {
    
    // error/uninitialized
    KS_AST_NONE        = 0,

    // a constant value (int, float, string, etc)
    // sub := (val: any, )
    KS_AST_VAL,

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





};

// abstract syntax tree node for a KataScript program
typedef struct ks_ast {

    // the kind of node, see 'KS_AST_*'
    s32 kind;

    // sub nodes of the AST (this depends on the kind)
    ktuple sub;

}* ks_ast;

// create a new AST node
KATA_API ks_ast
ks_ast_new(s32 kind, int nsub, kobj* sub);

// create a new AST node, absorbing references to 'sub'
// NOTE: this always removes the references to 'sub', and handles the case where
//         some are NULL (if any NULLs are found, a reference is removed from others
//         and no result is returned)
KATA_API ks_ast
ks_ast_newz(s32 kind, int nsub, kobj* sub);




////////////////////////////////////////////////////////////////////////////////

// transform KataScript file contents into tokens, storing and reallocating '*ptoks'
//   as needed
// NOTE: the number of tokens is returned, or if the return value is <0, an error occured
// NOTE: '*ptoks' needs to be freed if there was or wasn't an error! (use 'kmem_free()')
// if you want a list of the tokens, call 'klist_newz(len, toks)', and then 'kmem_free(toks)'
//   which will transfer the references
KATA_API s32
ks_lex(kstr filename, kstr src, ks_token** ptoks);

// parse a KataScript file contents into an AST
// NOTE: you can pass 'toks==NULL' and 'ntoks==-1' to auto-lex the input sources
KATA_API ks_ast
ks_parse(kstr filename, kstr src, s32 ntoks, ks_token* toks);


////////////////////////////////////////////////////////////////////////////////

KATA_API ktype
Ks_ast,
Ks_token
;

#endif // KATA_KS_H
