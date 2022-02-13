/* src/ks/ast.c - ks_ast implementation
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>
#include <kata/ks.h>


/// C API ///

KTYPE_DECL(Ks_ast);

KATA_API ks_ast
ks_ast_new(s32 kind, int nsub, kobj* sub) {
    ks_ast obj = kobj_make(Ks_ast);
    if (!obj) return NULL;

    obj->kind = kind;
    obj->sub = ktuple_new(nsub, sub);

    return obj;
}

KATA_API ks_ast
ks_ast_newz(s32 kind, int nsub, kobj* sub) {
    ks_ast obj = kobj_make(Ks_ast);
    if (!obj) return NULL;

    obj->kind = kind;
    obj->sub = ktuple_newz(nsub, sub);

    return obj;
}

static KCFUNC(ks_ast_del_) {
    ks_ast obj;
    KARGS("obj:!", &obj, Ks_ast);
    
    KOBJ_DECREF(obj->sub);
    kobj_del(obj);
    return NULL;
}

static KCFUNC(ks_ast_repr_) {
    ks_ast obj;
    kobj io;
    KARGS("obj:! io", &obj, Ks_ast, &io);

    ssize res = kprintf(io, "ks.ast(%i, %R)", (int)obj->kind, obj->sub);
    if (res < 0) return NULL;

    // return number of bytes
    return kint_news(res);
}



KATA_API void
kinit_ks_ast() {

    ktype_init(Ks_ast, sizeof(struct ks_ast), "ks.ast", "Abstract Syntax Tree (AST) for the KataScript language");
    ktype_merge(Ks_ast, KDICT_IKV(
        { "__del", (kobj)kfunc_new(ks_ast_del_, "ks.ast.__del(obj: ks.ast)", "") },
        { "__repr", (kobj)kfunc_new(ks_ast_repr_, "ks.ast.__repr(obj: ks.ast, io)", "") },
    ));

}
