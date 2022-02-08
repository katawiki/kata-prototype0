/* src/ks/lex.c - ks_lex() implementation, a custom lexer
 *
 *
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
myis_name_s(kucp c) {
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
    if (myis_name_s(c)) return true;

    return false;
}



/// C API ///

KATA_API s32
ks_lex(kstr filename, kstr src, ks_token** ptoks) {
    s32 res = 0, res_max = 0;

    // DO NOT OVERWRITE THESE, they are used in 'ADV()'
    kucp c;
    s32 i = 0, line = 0, col = 0;

    // advance one character, writing to 'c'
    #define ADV() do { \
        if (i >= src->lenb) break; \
        if (c == '\n') { \
            line++; \
            col = 0; \
        } else { \
            col++; \
        } \
        c = src->data[i]; \
        i++; \
    } while (0)

    // emit a token, given the kind
    #define EMIT(kind_) do { \
        ks_token tok = ks_token_new((kind_), i_start, i - i_start, line_start, col_start, col - col_start); \
        if (!tok) return -1; \
        s32 newres = res + 1; \
        if (newres > res_max) { \
            res_max = 4 + (newres) * 3 / 2; \
            if (!kmem_grow((void**)ptoks, sizeof(ks_token) * res_max)) return -1; \
        } \
        (*ptoks)[res++] = tok; \
    } while (0)

    // TODO: unicode?
    while (i < src->lenb) {

        // read in a character
        ADV();

        s32 i_start = i, line_start = line, col_start = col;

        if (c == ' ' || c == '\t' || c == '\v' || c == '\n') {
            // ignore, whitespace
        } else if (c == '#') {
            // comment, so skip to the end of the next line
            while (i < src->lenb) {
                if (src->data[i] == '\n') break;
                ADV();
            }

            EMIT(KS_TOKEN_COMMENT);
        } else if (myis_digit(c, 10) || (c == '.' && myis_digit(src->data[i], 10))) {
            // either a number beginning with a digit, or possibly with '.'
            // TODO: should that be invalid syntax?

            // attempt to autodetect base
            s32 base = 0;
            if (c == '0') {
                ADV();
                /**/ if (c == 'b' || c == 'B') base = 2;
                else if (c == 'o' || c == 'O') base = 8;
                else if (c == 'x' || c == 'X') base = 16;
                if (base != 0) ADV();
            }
            if (!base) base = 10;

            // now, skip all digits until we hit a non-digit
            while (i < src->lenb && myis_digit(c, base)) {
                ADV();
            }
            
            // if we hit '.', then we have a float
            bool is_float = c == '.';
            if (is_float) {
                ADV();
                // skip fractional component
                while (i < src->lenb && myis_digit(c, base)) {
                    ADV();
                }
            }

            if (base == 10 && (c == 'e' || c == 'E')) {
                // scientific decimal notation
                // TODO: should this make it a float?
                is_float = true;
                ADV();
                // skip sign of number
                if (c == '+' || c == '-') ADV();
                // skip digits of exponent
                while (i < src->lenb && myis_digit(c, 10)) {
                    ADV();
                }
            } else if ((base == 2 || base == 8 || base == 16) && (c == 'p' || c == 'P')) {
                // scientific binary/octal/hex notation
                // TODO: should this make it a float?
                is_float = true;
                ADV();
                // skip sign of number
                if (c == '+' || c == '-') ADV();
                // skip digits of exponent (which are always decimal)
                while (i < src->lenb && myis_digit(c, 10)) {
                    ADV();
                }
            }

            // if we hit 'i', we have an imaginary number
            if (c == 'i' || c == 'I') {
                is_float = true;
                ADV();
            }

            EMIT(is_float ? KS_TOKEN_FLOAT : KS_TOKEN_INT);
        } else {
            fprintf(stderr, "ERROR: bad character: %c\n", (char)c);
            kexit(-1);
            return -1;
        }
    }

    return res;
}

