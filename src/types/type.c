/* src/types/type.c - implementation of ktype
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

KTYPE_DECL(Ktype);

KATA_API void
ktype_init(ktype tp, s32 sz, const char* name, const char* docs) {
    assert(tp != NULL);
    struct kobj_meta* meta = KOBJ_META(tp);

    // TODO: make infinite refcount?
    meta->type = Ktype;
    meta->refc = 1;

    tp->sz = sz;
    tp->name = kstr_new(-1, name);
    tp->docs = kstr_new(-1, docs);

}
