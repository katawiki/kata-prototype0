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

    if (klist_pushn(obj, len, data) < 0) {
        KOBJ_DECREF(obj);
        return NULL;
    }

    return obj;
}

KATA_API klist
klist_newz(usize len, kobj* data) {
    klist obj = kobj_make(Klist);
    if (!obj) return NULL;

    if (klist_init(obj, len, data) < 0) {
        KOBJ_DECREF(obj);
        return NULL;
    }

    if (klist_pushn(obj, len, data) < 0) {
        KOBJ_DECREF(obj);
        return NULL;
    }

    return obj;
}



KATA_API keno
klist_init(struct klist* obj, usize len, kobj* data) {
    obj->len = obj->cap = 0;
    obj->data = NULL;
    if (klist_pushn(obj, len, data) < 0) {
        klist_done(obj);
        return -1;
    }

    return 0;
}

KATA_API void
klist_done(struct klist* obj) {
    kmem_free(obj->data);
}

KATA_API keno
klist_push(struct klist* obj, kobj val) {
    return klist_pushn(obj, 1, &val);
}

KATA_API keno
klist_pushn(struct klist* obj, usize len, kobj* vals) {
    keno rc = klist_pushz(obj, len, vals);
    if (rc < 0) return rc;

    // add references
    usize i;
    for (i = 0; i < len; ++i) {
        KOBJ_INCREF(vals[i]);
    }

    return rc;
}

KATA_API keno
klist_pushz(struct klist* obj, usize len, kobj* vals) {
    if (obj->cap >= obj->len + len) {
        // we have enough space
        memcpy(obj->data + obj->len, vals, len * sizeof(kobj));
        obj->len += len;
        return 0;
    }

    // we need to reallocate
    if (!kmem_growx((void**)&obj->data, &obj->cap, sizeof(kobj) * (obj->len + len))) {
        return -1;
    }

    // add to the array, but don't add reference
    usize i;
    for (i = 0; i < len; ++i) {
        obj->data[obj->len + i] = vals[i];
        //KOBJ_INCREF(vals[i]);
    }

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

