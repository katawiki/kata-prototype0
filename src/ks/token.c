/* src/ks/ast.c - ks_ast implementation
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>
#include <kata/ks.h>


/// C API ///

KTYPE_DECL(Ks_token);

KATA_API ks_token
ks_token_new(s32 kind, s32 posb, s32 lenb, s32 line, s32 col, s32 lenc) {
    ks_token obj = kobj_make(Ks_token);
    if (!obj) return NULL;
    
    obj->kind = kind;
    obj->posb = posb;

    obj->line = line;
    obj->col = col;
    obj->lenc = lenc;

    return obj;
}


KATA_API void
kinit_ks_token() {

    ktype_init(Ks_token, sizeof(struct ks_token), "ks.token", "Token of text in the KataScript language");

}
