/* src/ks/parse.c - ks_parse() implementation, a custom parser
 *
 * open language issues/TODOs:
 *   * use 'not in' or '!in'
 *
 *
 *
 * ORIGINAL SOURCES:
 * https://github.com/ChemicalDevelopment/kscript/blob/main/src/parser.c
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>
#include <kata/ks.h>

/// PARSER IMPLEMENTATION ///

// kscript EBNF like syntax:
//
// MODULE    : STMT* EOF
//
// # statement
// STMT      : 'import' NAME N
//           | 'ret' EXPR? N
//           | 'throw' EXPR? N
//           | 'break' INT? N
//           | 'cont' INT? N
//           | 'if' EXPR BORCS ('elif' EXPR BORCS)* ('else' EXPR BORS)?
//           | 'while' EXPR BORCS ('elif' EXPR BORCS)* ('else' EXPR BORS)?
//           | 'for' EXPR BORCS ('elif' EXPR BORCS)* ('else' EXPR BORS)?
//           | 'try' BORCS ('catch' EXPR (('as' | '->') EXPR)?)* ('always' BORS)?
//           | EXPR? N
//
// # block of statements
// B         : '{' STMT* '}'
//
// # block or (comma statement)
// BORCS     : B
//           | ',' STMT
//
// # block or statement
// BORS      : B
//           | STMT
//
// # newline or equivalent
// N         : '\n'
//           | ';'
//
// # expression, which is a pipeline of expressions effectively
// EXPR      : E0
//
// E0        : E1 (('|' | '&') E1)*
//
// E1        : E2 ('=' | '+=' | '-=' | '*=' | '/=' | '//=' | '%=' | '^=' | '@=') E2
//
// E2        : E3 ('->' E3)*
//
// E3        : E4 ('?' E4 ':' E4)*
//
// E4        : E5 ('??' E5)*
//
// E5        : E6 ('||' E6)*
//
// E6        : E7 ('??' E7)*
//
// E7        : E8 (('===' | '==' | '!=' | '<=' | '>=' | '<' | '>' | 'in' | 'not' 'in' ) E8)*
//
// E8        : E9 (('+' | '-') E9)*
//
// E9        : E10 (('*' | '/' | '//' | '%' | '@') E10)*
//
// E10       : E11 ('^' E11)*
//
// E11       : ('++' | '--' | '+' | '-' | '~' | '!' | '?')* E12
//
// E12       : E13 (('.' NAME) | '(' (ARG (',' ARG)*)? ')' | '[' (ARG (',' ARG)*)? ']' | '++' | '--')*
//
// E13        : ATOM
//           | '(' (ELEM (',' ELEM)*))? ','? ')'
//           | '[' (ELEM (',' ELEM)*))? ','? ']'
//           | '{' (ELEM (',' ELEM)*)) ','? | ','? '}'
//           | '{' (ELEMKV (',' ELEMKV)*)) ','? '}'
//           | 'func' NAME? ('(' (PAR (',' PAR)* ','?)? ')')? B
//           | 'type' NAME? ('extends' EXPR)? B
//           | 'enum' NAME? B
//
// ATOM      : INT
//           | FLOAT
//           | STR
//           | BLOB
//           | REGEX
//           | NAME
//           | '$'
//           | '@'
//           | '...'
//
// ARG       : '*' EXPR
//           | EXPR
//
// ELEM      : '*' EXPR
//           | EXPR
//
// ELEMKV    : EXPR ':' EXPR
//
// PAR       : '*' NAME
//           | NAME ('=' EXPR)?
//
// INT       : < literal 'int' >
// FLOAT     : < literal 'float' >
// STR       : < literal 'str' >
// BLOB      : < literal 'blob' >
// REGEX     : < literal 'regex' >
// NAME      : < unicode identifier >
//
// EOF       : < end of file>>
//

// helper buffer maker, with a number of elements to allocate by default
#define HBUF_MAKE(name_, qn_) \
    s32 name_##_n = 0, name_##_maxn = 0, name_##_qn = qn_; \
    kobj name_##_hbuf_q[qn_]; \
    kobj* name_ = name_##_hbuf_q; \

// push an object on to the hbuf
#define HBUF_PUSH(name_, val_) \
    if (name_##_n == name_##_qn) { \
        name_ = kmem_make((name_##_qn+1) * sizeof(kobj)); \
        if (!name_) { \
            assert(false); \
        } \
        memcpy(name_, name_##_hbuf_q, name_##_qn * sizeof(kobj)); \
    } else if (name_ != name_##_hbuf_q && name_##_n >= name_##_maxn) { \
        name_##_maxn = kmem_nextcap(name_##_maxn, sizeof(kobj) * (name_##_n+1)); \
        if (!kmem_grow((void**)&name_, name_##_maxn)) { \
            assert(false); \
        } \
    } \
    name_[name_##_n++] = val_; \

// pop an object and assign reference to 'to_'
#define HBUF_POP(name_, to_) \
    to_ = name_[--name_##_n]; \

// reset/clear the buffer
#define HBUF_CLEAR(name_) do { \
    int i_; \
    for (i_ = 0; i_ < name_##_n; ++i_) { \
        if (name_[i_]) kobj_free(name_[i_]); \
    } \
    name_##_n = 0; \
    name_##_maxn = 0; \
    if (name_ != name_##_hbuf_q) { \
        kmem_free(name_); \
        name_ = name_##_hbuf_q; \
    } \
} while (0);



// helper macro to define a C-style rule function
#define RULE(name_) static ks_ast name_(kstr filename, kstr src, s32 ntoks, ks_tok* toks, s32* ptoki)

// helper macro to define a C-style bool rule function, returns whether it matched or
//   not (NOTE: this does not throw an error !)
#define BOOLRULE(name_) static bool name_(kstr filename, kstr src, s32 ntoks, ks_tok* toks, s32* ptoki)

// helper macro to match the given rule
#define MATCH(name_) name_(filename, src, ntoks, toks, ptoki)

// helper macro for the current token index
#define TOKI (*ptoki)

// helper macro for the current token
#define TOK (toks[TOKI])

// helper macro to eat a token
#define EAT() (toks[TOKI++])

// last token
#define LAST (toks[TOKI-1])

// helper macro for testing whether the given token is equal to a C-style string
#define TOK_EQ(tok_, cstr_) ((sizeof(cstr_) - 1) == (tok_)->lenb && strncmp(cstr_, src->data + (tok_)->posb, (sizeof(cstr_) - 1)) == 0)


RULE(STMT);
RULE(EXPR);
RULE(ATOM);
RULE(ARG);
RULE(NAME);
RULE(INT);
RULE(FLOAT);
RULE(STR);

// skip irrelevant/extra tokens
BOOLRULE(SKIP) {
    while (TOK_EQ(TOK, "\n") || TOK_EQ(TOK, ";")) {
        TOKI++;
    }
    return true;
}

BOOLRULE(N) {
    if (TOK_EQ(TOK, "\n")) {
        TOKI++;
        return true;
    } else if (TOK_EQ(TOK, ";")) {
        TOKI++;
        return true;
    }

    // not found, so signal that
    return false;
}

RULE(B) {
    if (TOK_EQ(TOK, "{")) {
        // '{' STMT* '}'
        ks_tok tok = TOK;
        TOKI++;
        
        // collect statements
        HBUF_MAKE(hb, 16);
        
        if (!MATCH(SKIP)) {
            HBUF_CLEAR(hb);
            return NULL;
        }

        while (!TOK_EQ(TOK, "}")) {

            // collect another statement
            ks_ast a = MATCH(STMT);
            if (!a) {
                HBUF_CLEAR(hb);
                return NULL;
            }
            HBUF_PUSH(hb, a);

            if (!MATCH(SKIP)) {
                HBUF_CLEAR(hb);
                return NULL;
            }
        }

        ks_ast res = ks_ast_newz(tok, KS_AST_BLOCK, hb_n, hb);
        hb_n = 0;
        HBUF_CLEAR(hb);
        return res;

    } else {
        assert(false);
    }
}

RULE(BORS) {
    if (TOK_EQ(TOK, "{")) {
        return MATCH(B);
    } else {
        return MATCH(STMT);
    }
}

RULE(BORCS) {
    if (TOK_EQ(TOK, "{")) {
        return MATCH(B);
    } else if (TOK_EQ(TOK, ",")) {
        TOKI++;
        return MATCH(STMT);
    } else {
        assert(false);
    }
}

RULE(STMT) {
    if (TOK_EQ(TOK, "import")) {
        // 'import' NAME N
        TOKI++;

        ks_ast res = ks_ast_newz(LAST, KS_AST_IMPORT, 1, (kobj[]){ (kobj)MATCH(NAME) });
        if (!res) {
            return NULL;
        }

        if (!MATCH(N)) {
            KOBJ_DECREF(res);
            return NULL;
        }
        return res;
    } else if (TOK_EQ(TOK, "ret")) {
        // 'ret' EXPR? N
        ks_tok tok = EAT();

        if (MATCH(N)) {
            // just do none
            return ks_ast_newz(LAST, KS_AST_RET, 0, NULL);
        } else {
            // parse an expression to return
            ks_ast res = ks_ast_newz(LAST, KS_AST_RET, 1, (kobj[]){ (kobj)MATCH(EXPR) });
            if (!res) return NULL;
            
            // now, we need to match an end
            if (!MATCH(N)) {
                KOBJ_DECREF(res);
                return NULL;
            }

            return res;
        }
    } else if (TOK_EQ(TOK, "throw")) {
        // 'throw' EXPR? N
        ks_tok tok = EAT();

        if (MATCH(N)) {
            // just do none
            return ks_ast_newz(tok, KS_AST_THROW, 0, NULL);
        } else {
            // parse an expression to return
            ks_ast res = ks_ast_newz(tok, KS_AST_THROW, 1, (kobj[]){ (kobj)MATCH(EXPR) });
            if (!res) return NULL;
            
            // now, we need to match an end
            if (!MATCH(N)) {
                KOBJ_DECREF(res);
                return NULL;
            }

            return res;
        }

    } else if (TOK_EQ(TOK, "break")) {
        assert(false);
    } else if (TOK_EQ(TOK, "cont")) {
        assert(false);
    } else if (TOK_EQ(TOK, "if")) {
        // 'if' EXPR BORCS ('elif' EXPR BORCS)* ('else' BORS)?
        TOKI++;

        // make a buffer to push items on to
        HBUF_MAKE(hb, 8);

        do {
            // after the first run, skip the 'elif'
            if (hb_n > 0) TOKI++;

            // parse another (expr, body) pair

            ks_ast a = MATCH(EXPR);
            if (!a) {
                HBUF_CLEAR(hb);
                return NULL;
            }
            HBUF_PUSH(hb, a);

            a = MATCH(BORCS);
            if (!a) {
                HBUF_CLEAR(hb);
                return NULL;
            }
            HBUF_PUSH(hb, a);

        } while (TOK_EQ(TOK, "elif"));

        if (TOK_EQ(TOK, "else")) {
            // read final 'else' clause
            TOKI++;

            ks_ast c = MATCH(BORS);
            if (!c) {
                HBUF_CLEAR(hb);
                return NULL;
            }
            HBUF_PUSH(hb, c);
        }

        // now, construct tree
        ks_ast res;
        while (hb_n > 0) {
            // number to take, on the first time also take the 'else' clause 
            int num = hb_n % 2 == 1 ? 3 : 2;
            hb_n -= num;
            res = ks_ast_newz(NULL, KS_AST_IF, num, hb + hb_n);
            if (!res) {
                HBUF_CLEAR(hb);
                return NULL;
            }

            // push it back on the stack
            HBUF_PUSH(hb, res);
        }

        // now, take the last item which should be the full tree
        assert(hb_n == 1);
        HBUF_POP(hb, res);
        HBUF_CLEAR(hb);
        return res;

    } else if (TOK_EQ(TOK, "while")) {
        assert(false);
    } else if (TOK_EQ(TOK, "for")) {
        assert(false);
    } else if (TOK_EQ(TOK, "try")) {
        assert(false);
    } else {
        // otherwise, assume just a normal expression
        ks_ast res = MATCH(EXPR);
        if (!res) {
            return NULL;
        }
        return res;

        // should be followed by a newline or equivalent
        if (!MATCH(N)) {
            KOBJ_DECREF(res);
            return NULL;
        }

        return res;
    }
}

RULE(ARG) {
    return MATCH(EXPR);
}

RULE(E13) {
    return MATCH(ATOM);
}

RULE(E12) {
    ks_ast res = MATCH(E13);
    if (!res) return NULL;

    while (true) {
        switch (TOK->kind)
        {
        case KS_TOK_DOT:
            // '.' NAME
            TOKI++;
            if (TOK->kind != KS_TOK_NAME) {
                // TODO: error message
                assert(false);
                KOBJ_DECREF(res);
                return NULL;
            }
            res = ks_ast_newz(LAST, KS_AST_ATTR, 2, (kobj[]){ (kobj)res, (kobj)kstr_new(LAST->lenb, src->data + LAST->posb) });
            TOKI++;
            break;
        case KS_TOK_LPAR:
            // '(' (ARG (',' ARG)*)? ')'
            TOKI++;

            // push all arguments
            HBUF_MAKE(hb, 8);

            // start by pushing the current result as the function
            HBUF_PUSH(hb, res);

            while (TOK->kind != KS_TOK_RPAR) {
                ks_ast arg = MATCH(ARG);
                if (!arg) {
                    HBUF_CLEAR(hb);
                    return NULL;
                }
                HBUF_PUSH(hb, arg);

                // check for continuation
                if (TOK->kind == KS_TOK_COMMA) {
                    TOKI++;
                } else {
                    break;
                }
            }

            if (TOK->kind != KS_TOK_RPAR) {
                assert(false);
                return NULL;
            }
            TOKI++;

            // now, construct tree
            res = ks_ast_newz(LAST, KS_AST_CALL, hb_n, hb);
            hb_n = 0;
            HBUF_CLEAR(hb);
            return res;

        default:
            return res;
        }
    }
}

RULE(E11) {
    return MATCH(E12);
}

RULE(E10) {
    ks_ast res = MATCH(E11);
    if (!res) return NULL;

    while (true) {
        switch (TOK->kind)
        {
        case KS_TOK_UP:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_POW, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E10) });
            break;
        default:
            return res;
        }
    }
}

RULE(E9) {
    ks_ast res = MATCH(E10);
    if (!res) return NULL;

    while (true) {
        switch (TOK->kind)
        {
        case KS_TOK_STAR:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_MUL, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E10) });
            break;
        case KS_TOK_SLASH:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_DIV, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E10) });
            break;
        case KS_TOK_SLASHSLASH:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_FLOORDIV, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E10) });
            break;
        case KS_TOK_PERC:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_MOD, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E10) });
            break;
        case KS_TOK_AT:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_MATMUL, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E10) });
            break;

        default:
            return res;
        }
    }
}

RULE(E8) {
    ks_ast res = MATCH(E9);
    if (!res) return NULL;

    while (true) {
        switch (TOK->kind)
        {
        case KS_TOK_PLUS:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_ADD, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E9) });
            break;
        case KS_TOK_MINUS:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_SUB, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E9) });
            break;

        default:
            return res;
        }
    }
}

RULE(E2) {
    return MATCH(E8);
}

RULE(E1) {
    ks_ast res = MATCH(E2);
    if (!res) return NULL;

    while (true) {
        switch (TOK->kind)
        {
        case KS_TOK_EQ:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_EQ, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E2) });
            break;
        case KS_TOK_PLUSEQ:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_ADDEQ, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E2) });
            break;
        case KS_TOK_MINUSEQ:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_SUBEQ, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E2) });
            break;

        case KS_TOK_STAREQ:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_MULEQ, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E2) });
            break;

        case KS_TOK_SLASHEQ:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_DIVEQ, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E2) });
            break;

        case KS_TOK_PERCEQ:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_MODEQ, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E2) });
            break;

        case KS_TOK_UPEQ:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_POWEQ, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E2) });
            break;
        
        case KS_TOK_ATEQ:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_MATMULEQ, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E2) });
            break;
        default:
            return res;
        }
    }
}

RULE(E0) {
    ks_ast res = MATCH(E1);
    if (!res) return NULL;

    while (true) {
        u32 k = TOK->kind;
        switch (TOK->kind)
        {
        case KS_TOK_PIPE:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_PIPE, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E1) });
            break;
        case KS_TOK_AND:
            TOKI++;
            res = ks_ast_newz(LAST, KS_AST_AND, 2, (kobj[]){ (kobj)res, (kobj)MATCH(E1) });
            break;
        
        default:
            return res;
        }
    }
}

RULE(EXPR) {
    return MATCH(E0);
    /*
    if (TOK->kind == KS_TOK_NAME) {
        return MATCH(NAME);
    } else if (TOK->kind == KS_TOK_INT) {
        return MATCH(INT);
    } else {
        printf("GOT: %i\n", TOK->kind);
        assert(false);
    }*/
}

RULE(ATOM) {
    if (TOK->kind == KS_TOK_NAME) {
        return MATCH(NAME);
    } else if (TOK->kind == KS_TOK_INT) {
        return MATCH(INT);
    } else if (TOK->kind == KS_TOK_FLOAT) {
        return MATCH(FLOAT);
    } else {
        printf("GOT: %i\n", TOK->kind);
        assert(false);
    }
}

RULE(NAME) {
    if (TOK->kind == KS_TOK_NAME) {
        TOKI++;
        return ks_ast_wrapx(LAST, KS_AST_NAME, (kobj)kstr_new(LAST->lenb, src->data + LAST->posb));
    } else {
        printf("GOT: %i\n", TOK->kind);
        assert(false);
    }
}

RULE(INT) {
    if (TOK->kind == KS_TOK_INT) {
        // TODO: parse
        TOKI++;
        kstr s = kstr_new(LAST->lenb, src->data + LAST->posb);
        assert(s != NULL);
        kint v = kint_new(s->data, 10);
        assert(v != NULL);
        KOBJ_DECREF(s);
        ks_ast res = ks_ast_wrap(LAST, (kobj)v);
        return res;
    } else {
        printf("GOT: %i\n", TOK->kind);
        assert(false);
    }
}

RULE(FLOAT) {
    if (TOK->kind == KS_TOK_FLOAT) {
        // TODO: parse
        TOKI++;
        kstr s = kstr_new(LAST->lenb, src->data + LAST->posb);
        assert(s != NULL);
        kfloat v = kfloat_new(s->data, 10, -1);
        assert(v != NULL);
        KOBJ_DECREF(s);
        ks_ast res = ks_ast_wrap(LAST, (kobj)v);
        return res;
    } else {
        printf("GOT: %i\n", TOK->kind);
        assert(false);
    }
}

/// C API ///

KATA_API ks_ast
ks_parse(kstr filename, kstr src, s32* pntoks, ks_tok** ptoks) {
    // tokenize as needed
    if (*pntoks <= 0) {
        *pntoks = ks_lex(filename, src, ptoks);
        if (*pntoks < 0) {
            return NULL;
        }
    }

    // actually match rule
    s32 ntoks = *pntoks;
    ks_tok* toks = *ptoks;

    s32 toki = 0;
    s32* ptoki = &toki;

    ks_ast res = MATCH(STMT);
    if (!res) {
        return NULL;
    }

    return res;

    /*
    return ks_ast_newz(KS_AST_NAME, 1, (kobj[]) {
        (kobj)kstr_new(-1, "asdf"),
    });
    */
}

