/* src/bf/init.c - initialize bf
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>

bf_context_t
kbf_ctx;

KATA_API bool
kbf_init(bf_t* obj, kobj val) {
    if (val) {
        // TODO: work
        assert(false);
    } else {
        bf_init(&kbf_ctx, obj);
    }

    return true;
}

KATA_API bool
kbf_const(kobj val, bf_t* obj, bool* dodone) {

    ktype tp = KOBJ_TYPE(val);
    if (tp->bfpos >= 0) {
        *dodone = false;
        *obj = *(bf_t*)(((usize)val) + tp->bfpos);
        return true;
    } else {
        *dodone = true;
        return kbf_init(obj, val);
    }
}

KATA_API void
kbf_done(bf_t* obj) {
    bf_delete(obj);
}

KATA_API void*
kbf_realloc(void *opaque, void *ptr, size_t sz) {
    if (!sz) {
        // free pointer
        kmem_free(ptr);
        return NULL;
    }

    void* res = ptr;
    if (!kmem_grow(&res, sz)){ 
        fprintf(stderr, "libbf: out of memory\n");
        kexit(KENO_ERR_OOM);
        return NULL;
    }
    return res;
}

static inline s64
kbf_precab(s64 a, s64 b) {
    return a > b ? a : b;
}
static inline s64
kbf_precabc(s64 a, s64 b, s64 c) {
    return kbf_precab(kbf_precab(a, b), c);
}

KATA_API s64
kbf_prec() {
    return 64;
}

KATA_API s64
kbf_prec2(s64 a, s64 b) {
    return kbf_precabc(a, b, kbf_prec());
}

KATA_API s64
kbf_precx(const bf_t* a) {
    // TODO: better way?
    return kbf_precab(a->len * LIMB_BITS, kbf_prec());
}

KATA_API s64
kbf_precy(const bf_t* a, const bf_t* b) {
    return kbf_prec2(kbf_precx(a), kbf_precx(b));
}


/// C API ///

KATA_API void
kinit_bf() {

}
