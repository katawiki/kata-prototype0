/* src/ks/parse.c - ks_parse() implementation, a custom parser
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


/// C API ///

KATA_API ks_ast
ks_parse(kstr filename, kstr src, s32 ntoks, ks_token* toks) {

    return ks_ast_newz(KS_AST_NAME, 1, (kobj[]) {
        (kobj)kstr_new(-1, "asdf"),
    });
}

