/* src/ks/lex.c - ks_lex() implementation, a custom lexer
 *
 * TODO: replace this all with a regex solution?
 *
 * ORIGINAL SOURCES:
 * https://github.com/ChemicalDevelopment/kscript/blob/main/src/lexer.c
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>
#include <kata/ks.h>


/// internal utilities ///

// test if 'c' is a valid digit in base 'b'
// NOTE: this does NOT accept unicode digits, because those should not be encouraged
static bool
myis_digit(kucp c, s32 base) {
    /**/ if (base == 2) return '0' <= c && c <= '1';
    else if (base == 8) return '0' <= c && c <= '7';
    else if (base == 10) return '0' <= c && c <= '9';
    else if (base == 16) return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
    else {
        // unknown base, should never happen
        assert(false);
    }
}

// test if 'c' is a valid starting character of a name
static bool
myis_names(kucp c) {
    if (c < 128) {
        if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_') return true;
    } else {
        /* TODO: unicode */
        /*
        struct ksucd_info info;
        ks_ucp cp = ksucd_get_info(&info, c);
        if (cp > 0) {
            if (info.cat_gen >= 0 && info.cat_gen <= ksucd_cat_L) return true;
        }
        */
        assert(false);
    }
    return false;
}

// test if 'c' is a valid character of a name
static bool
myis_name(kucp c) {
    if (myis_digit(c, 10)) return true;
    if (myis_names(c)) return true;

    return false;
}

/// C API ///

KATA_API s32
ks_lex(kstr filename, kstr src, ks_tok** ptoks) {
    s32 res = 0, res_max = 0;

    // DO NOT OVERWRITE THESE, they are used in 'ADV()'
    s32 i = 0, line = 0, col = 0;

    // current byte/character
    #define CUR (src->data[i])
    // next byte
    #define NEXT (src->data[i+1])

    // advance one character, writing to 'c'
    #define ADV() do { \
        if (i >= src->lenb) break; \
        if (CUR == '\n') { \
            line++; \
            col = 0; \
        } else { \
            col++; \
        } \
        i++; \
    } while (0)

    // emit a token, given the kind
    #define EMIT(kind_) do { \
        ks_tok tok = ks_tok_new((kind_), i_start, i - i_start, line_start, col_start, col - col_start); \
        if (!tok) return -1; \
        s32 newres = res + 1; \
        if (newres > res_max) { \
            res_max = newres * 2; \
            if (!kmem_grow((void**)ptoks, sizeof(ks_tok) * res_max)) return -1; \
        } \
        (*ptoks)[res++] = tok; \
    } while (0)


    // TODO: unicode?
    while (i < src->lenb) {

        s32 i_start = i, line_start = line, col_start = col;

        // read in a character
        if (CUR == ' ' || CUR == '\t' || CUR == '\v' || CUR == '\n') {
            // ignore, whitespace
            ADV();
        } else if (CUR == '#') {
            // comment, so skip to the end of the next line
            while (i < src->lenb && CUR != '\n') {
                ADV();
            }

            EMIT(KS_TOK_COMMENT);
        } else if (myis_names(CUR)) {
            do {
                ADV();
            } while (myis_name(CUR));

            EMIT(KS_TOK_NAME);

        } else if (myis_digit(CUR, 10) || (CUR == '.' && myis_digit(NEXT, 10))) {
            // either a number beginning with a digit, or possibly with '.'
            // TODO: should that be invalid syntax?

            // attempt to autodetect base
            s32 base = 0;
            if (CUR == '0') {
                ADV();
                /**/ if (CUR == 'b' || CUR == 'B') base = 2;
                else if (CUR == 'o' || CUR == 'O') base = 8;
                else if (CUR == 'x' || CUR == 'X') base = 16;
                if (base != 0) ADV();
            }
            if (!base) base = 10;

            // now, skip all digits until we hit a non-digit
            while (myis_digit(CUR, base)) {
                ADV();
            }
            
            // if we hit '.', then we have a float
            bool is_float = CUR == '.';
            if (is_float) {
                ADV();
                // skip fractional component
                while (myis_digit(CUR, base)) {
                    ADV();
                }
            }

            if (base == 10 && (CUR == 'e' || CUR == 'E')) {
                // scientific decimal notation
                // TODO: should this make it a float?
                ADV();
                is_float = true;
                // skip sign of number
                if (CUR == '+' || CUR == '-') ADV();
                // skip digits of exponent
                while (myis_digit(CUR, 10)) {
                    ADV();
                }
            } else if ((base == 2 || base == 8 || base == 16) && (CUR == 'p' || CUR == 'P')) {
                // scientific binary/octal/hex notation
                // TODO: should this make it a float?
                ADV();
                is_float = true;
                // skip sign of number
                if (CUR == '+' || CUR == '-') ADV();
                // skip digits of exponent (which are always decimal)
                while (i < src->lenb && myis_digit(CUR, 10)) {
                    ADV();
                }
            }

            // if we hit 'i', we have an imaginary number
            if (CUR == 'i' || CUR == 'I') {
                ADV();
                is_float = true;
            }

            EMIT(is_float ? KS_TOK_FLOAT : KS_TOK_INT);
        }

        #define CASE(kind_, str_) else if (strncmp(str_, src->data + i, sizeof(str_)-1) == 0) { \
            int j; \
            for (j = 0; j < (sizeof(str_)-1); ++j) { \
                ADV(); \
            } \
            EMIT(kind_); \
        }

        CASE(KS_TOK_LPAR, "(")
        CASE(KS_TOK_RPAR, ")")

        CASE(KS_TOK_PLUS, "+")
        CASE(KS_TOK_MINUS, "-")
        CASE(KS_TOK_STAR, "*")
        CASE(KS_TOK_SLASH, "/")
        CASE(KS_TOK_UP, "^")



        else {
            fprintf(stderr, "ERROR: bad character: %c\n", (char)CUR);
            kexit(-1);
            return -1;
        }
    }

    // emit a final EOF
    s32 i_start = i, line_start = line, col_start = col;
    EMIT(KS_TOK_EOF);


    // debug print out them
    for (i = 0; i < res; ++i) {
        fprintf(stderr, "  toks[%i]: posb=%i, lenb=%i\n", (int)i, (int)(*ptoks)[i]->posb, (int)(*ptoks)[i]->lenb);
    }

    return res;
}

