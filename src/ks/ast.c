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

KATA_API void
kinit_ks_ast() {

    ktype_init(Ks_ast, sizeof(struct ks_ast), "ks.ast", "Abstract Syntax Tree (AST) for the KataScript language");

}
