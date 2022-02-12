/* src/types/func.c - implementation of kfunc
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

KTYPE_DECL(Kfunc);

KATA_API kobj
kfunc_new(kcfunc cfunc, const char* name, const char* docs) {
    kfunc obj = kobj_make(Kfunc);
    if (!obj) return NULL;

    obj->kind = KFUNC_CFUNC;
    obj->cfunc_ = cfunc;

    obj->name = kstr_new(-1, name);
    obj->docs = kstr_new(-1, docs);

    return obj;
}


KATA_API void
kinit_func() {
    ktype_init(Kfunc, sizeof(struct kfunc), "func", "Function type");

}
