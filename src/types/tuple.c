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

static KCFUNC(ktuple_del_) {
    ktuple obj;
    KARGS("obj:!", &obj, Ktuple);

    // free all entries
    usize i;
    for (i = 0; i < obj->len; ++i) {
        KOBJ_DECREF(obj->data[i]);
    }

    kobj_del(obj);

    return NULL;
}


KATA_API void
kinit_tuple() {
    ktype_init(Ktuple, sizeof(struct ktuple), "tuple", "Tuple collection type");

    ktype_merge(Ktuple, KDICT_IKV(
        { "__del", kfunc_new(ktuple_del_, "tuple.__del(obj: tuple)", "") },
    ));
}

