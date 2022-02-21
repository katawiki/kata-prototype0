/* src/ks/tok.c - ks_tok implementation
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>
#include <kata/ks.h>


/// C API ///

KTYPE_DECL(Ks_tok);

KATA_API ks_tok
ks_tok_new(s32 kind, s32 posb, s32 lenb, s32 line, s32 col, s32 lenc) {
    ks_tok obj = kobj_make(Ks_tok);
    if (!obj) return NULL;
    
    obj->kind = kind;
    
    obj->posb = posb;
    obj->lenb = lenb;

    obj->line = line;
    obj->col = col;
    obj->lenc = lenc;

    return obj;
}

static KCFUNC(ks_tok_repr_) {
    ks_tok obj;
    kobj io;
    KARGS("obj:! io", &obj, Ks_tok, &io);

    return krrv(kprintf(io, "ks.tok(%v, %v)", (s64)obj->posb, (s64)obj->lenb));
}

KATA_API void
kinit_ks_tok() {

    ktype_init(Ks_tok, sizeof(struct ks_tok), "ks.tok", "Token of text in the KataScript language");
    ktype_merge(Ks_tok, KDICT_IKV(
        { "__repr", (kobj)kfunc_new(ks_tok_repr_, "ks.tok.__repr(obj: ks.tok, io)", "") },
    ));
}
