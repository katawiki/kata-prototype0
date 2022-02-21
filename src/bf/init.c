/* src/bf/init.c - initialize bf
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>

KATA_API bool
kbf_init(bf_t* obj, kobj val) {
    if (val) {
        // TODO: work
        assert(false);
    } else {
        bf_init(&Kbf_ctx, obj);
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


/// C API ///

KATA_API void
kinit_bf() {

}
