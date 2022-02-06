/* src/types/tuple.c - implementation of ktuple
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

KTYPE_DECL(Ktuple);

KATA_API ktuple
ktuple_new(usize len, kobj* data) {
    ktuple obj = ktuple_newz(len, data);
    if (!obj) return NULL;

    usize i;
    for (i = 0; i < len; ++i) {
        KOBJ_INCREF(data[i]);
    }

    return obj;
}

KATA_API ktuple
ktuple_newz(usize len, kobj* data) {
    // allocate enough memory to hold the pointers too
    struct kobj_meta* meta = kmem_make(sizeof(struct kobj_meta) + sizeof(struct ktuple) + sizeof(kobj) * len);
    if (!meta) return NULL;
    
    meta->refc = 1;
    meta->type = Ktuple;

    ktuple obj = KOBJ_UNMETA(meta);
    // copy over elements
    obj->len = len;
    usize i;
    for (i = 0; i < len; ++i) {
        obj->data[i] = data[i];
    }

    return obj;
}

