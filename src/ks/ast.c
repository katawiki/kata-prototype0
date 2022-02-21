/* src/ks/ast.c - ks_ast implementation
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>
#include <kata/ks.h>


/// C API ///

KTYPE_DECL(Ks_ast);

KATA_API ks_ast
ks_ast_wrapx(ks_tok tok, u32 kind, kobj sub) {
    ks_ast obj = kobj_make(Ks_ast);
    if (!obj) return NULL;

    obj->kind = kind;
    KOBJ_NINCREF(tok);
    obj->tok = tok;
    obj->sub = sub;

    return obj;
}

KATA_API ks_ast
ks_ast_wrap(ks_tok tok, kobj sub) {
    return ks_ast_wrapx(tok, KS_AST_VAL, sub);
}

KATA_API ks_ast
ks_ast_new(ks_tok tok, u32 kind, int nsub, kobj* sub) {
    ks_ast obj = kobj_make(Ks_ast);
    if (!obj) return NULL;

    obj->kind = kind;
    KOBJ_NINCREF(tok);
    obj->tok = tok;
    obj->sub = (kobj)ktuple_new(nsub, sub);

    return obj;
}

KATA_API ks_ast
ks_ast_newz(ks_tok tok, u32 kind, int nsub, kobj* sub) {

    // check for any NULLs and error out if thats the case
    int i;
    for (i = 0; i < nsub; ++i) {
        if (!sub[i]) {
            for (i = 0; i < nsub; ++i) {
                if (sub[i] != NULL) KOBJ_DECREF(sub[i]);
            }
            return NULL;
        }
    }

    ks_ast obj = kobj_make(Ks_ast);
    if (!obj) return NULL;

    obj->kind = kind;
    KOBJ_NINCREF(tok);
    obj->tok = tok;
    obj->sub = ktuple_newz(nsub, sub);

    return obj;
}

KATA_API const char*
ks_ast_kindname(u32 kind) {
    switch (kind) {
        case KS_AST_VAL: return "ks.ast.VAL";
        case KS_AST_NAME: return "ks.ast.NAME";
        case KS_AST_ADD: return "ks.ast.ADD";
        case KS_AST_SUB: return "ks.ast.SUB";
        case KS_AST_MUL: return "ks.ast.MUL";
        case KS_AST_DIV: return "ks.ast.DIV";
        case KS_AST_FLOORDIV: return "ks.ast.FDIV";
        case KS_AST_POW: return "ks.ast.POW";
        case KS_AST_CALL: return "ks.ast.CALL";
    }

    return "ks.ast.UNKNOWN";
}


static KCFUNC(ks_ast_del_) {
    ks_ast obj;
    KARGS("obj:!", &obj, Ks_ast);
    
    KOBJ_NDECREF(obj->tok);
    KOBJ_DECREF(obj->sub);
    kobj_del(obj);
    return NULL;
}

static KCFUNC(ks_ast_repr_) {
    ks_ast obj;
    kobj io;
    KARGS("obj:! io", &obj, Ks_ast, &io);

    switch (obj->kind) {
        // TODO: should VAL even report that its an AST?
        //case KS_AST_VAL: return krrv(kprintf(io, "ks.ast(%R)", obj->sub));
        case KS_AST_VAL: return krrv(kprintf(io, "%R", obj->sub));
        case KS_AST_NAME: return krrv(kprintf(io, "ks.name(%R)", obj->sub));
        case KS_AST_ADD: return krrv(kprintf(io, "ks.add(%J)", obj->sub));
        case KS_AST_SUB: return krrv(kprintf(io, "ks.sub(%J)", obj->sub));
        case KS_AST_MUL: return krrv(kprintf(io, "ks.mul(%J)", obj->sub));
        case KS_AST_DIV: return krrv(kprintf(io, "ks.div(%J)", obj->sub));
        case KS_AST_POW: return krrv(kprintf(io, "ks.pow(%J)", obj->sub));
        case KS_AST_CALL: return krrv(kprintf(io, "ks.call(%J)", obj->sub));
    }

    return krrv(kprintf(io, "ks.ast(%s, %R)", ks_ast_kindname(obj->kind), obj->sub));

}

KATA_API void
kinit_ks_ast() {

    ktype_init(Ks_ast, sizeof(struct ks_ast), "ks.ast", "Abstract Syntax Tree (AST) for the KataScript language");
    ktype_merge(Ks_ast, KDICT_IKV(
        { "__del", (kobj)kfunc_new(ks_ast_del_, "ks.ast.__del(obj: ks.ast)", "") },
        { "__repr", (kobj)kfunc_new(ks_ast_repr_, "ks.ast.__repr(obj: ks.ast, io)", "") },
    ));

}
