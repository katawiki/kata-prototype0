/* src/types/buffer.c - implementation of kbuffer
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

struct ktype Kbuffer_, *Kbuffer = &Kbuffer_;


KATA_API kbuffer
kbuffer_new(usize len, const u8* data) {
    kbuffer obj = kobj_alloc(Kbuffer);
    if (!obj) return NULL;

    if (kbuffer_init(obj, len, data) < 0) {
        KOBJ_DECREF(obj);
        return NULL;
    }

    return obj;
}

KATA_API keno
kbuffer_init(struct kbuffer* obj, usize len, const u8* data) {
    obj->len = obj->cap = 0;
    obj->data = NULL;
    return kbuffer_push(obj, len, data);
}

KATA_API void
kbuffer_done(struct kbuffer* obj) {
    kmem_free(obj->data);
}

KATA_API keno
kbuffer_push(struct kbuffer* obj, usize len, const u8* data) {
    // check if we need to reallocate
    if (obj->cap < obj->len + len) {
        if (kmem_growx((void**)&obj->data, &obj->cap, obj->len + len) < 0) {
            return -1;
        }
    }
    // we, we have enough space
    memcpy(obj->data + obj->len, data, len);
    obj->len += len;
    return 0;
}

KATA_API keno
kbuffer_pop(struct kbuffer* obj, usize len) {
    if (obj->len < len) {
        // TODO: throw or handle?
        return -1;
    }

    obj->len -= len;
    return 0;
}
