/* src/types/list.c - implementation of klist
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

KTYPE_DECL(Klist);

KATA_API klist
klist_new(usize len, kobj* data) {
    klist obj = kobj_make(Klist);
    if (!obj) return NULL;

    if (klist_init(obj, len, data) < 0) {
        KOBJ_DECREF(obj);
        return NULL;
    }

    if (klist_pushx(obj, len, data) < 0) {
        KOBJ_DECREF(obj);
        return NULL;
    }

    return obj;
}

KATA_API klist
klist_newz(usize len, kobj* data) {
    klist obj = kobj_make(Klist);
    if (!obj) return NULL;

    obj->cap = obj->len = 0;
    obj->data = NULL;

    if (klist_pushz(obj, len, data) < 0) {
        KOBJ_DECREF(obj);
        return NULL;
    }

    return obj;
}

KATA_API keno
klist_init(struct klist* obj, usize len, kobj* data) {
    obj->len = obj->cap = 0;
    obj->data = NULL;
    if (klist_pushx(obj, len, data) < 0) {
        klist_done(obj);
        return -1;
    }

    return 0;
}

KATA_API void
klist_done(struct klist* obj) {
    kmem_free(obj->data);
}

KATA_API bool
klist_push(struct klist* obj, kobj val) {
    return klist_pushx(obj, 1, &val);
}

KATA_API keno
klist_pushx(struct klist* obj, usize len, kobj* vals) {
    // check if reallocation is needed
    if (obj->cap < obj->len + len) {
        if (!kmem_growx((void**)&obj->data, &obj->cap, sizeof(kobj) * (obj->len + len))) {
            return -1;
        }
    }

    // copy and increment references
    usize i;
    for (i = 0; i < len; ++i) {
        obj->data[obj->len + i] = vals[i];
        KOBJ_INCREF(vals[i]);
    }
    obj->len += len;
    return 0;
}

KATA_API keno
klist_pushz(struct klist* obj, usize len, kobj* vals) {
    // check if reallocation is needed
    if (obj->cap < obj->len + len) {
        if (!kmem_growx((void**)&obj->data, &obj->cap, sizeof(kobj) * (obj->len + len))) {
            return -1;
        }
    }

    // we have enough space, so just copy to the end
    memcpy(obj->data + obj->len, vals, len * sizeof(kobj));
    obj->len += len;
    return 0;
}

KATA_API kobj
klist_pop(struct klist* obj) {
    if (obj->len == 0) return NULL;

    // capture last and return the reference
    return obj->data[--obj->len];
}

KATA_API bool
klist_popu(struct klist* obj) {
    kobj val = klist_pop(obj);
    if (!val) return false;
    KOBJ_DECREF(val);
    return true;
}

static KCFUNC(klist_del_) {
    klist obj;
    KARGS("obj:!", &obj, Klist);

    // free all entries
    usize i;
    for (i = 0; i < obj->len; ++i) {
        KOBJ_DECREF(obj->data[i]);
    }

    kmem_free(obj->data);
    kobj_del(obj);

    return NULL;
}


KATA_API void
kinit_list() {
    ktype_init(Klist, sizeof(struct klist), "list", "List collection type");

    ktype_merge(Klist, KDICT_IKV(
        { "__del", kfunc_new(klist_del_, "list.__del(obj: list)", "") },
    ));
}
