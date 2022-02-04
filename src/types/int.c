/* src/types/int.c - implementation of kint
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

struct ktype Kint_, *Kint = &Kint_;

KATA_API kint
kint_new(const char* val, s32 base) {
    kint obj = kobj_alloc(Kint);
    if (!obj) return NULL;

    // init and set to string
    bf_init(&Kbf_ctx, &obj->val);
    const char* next = NULL;
    int rc = bf_atof(&obj->val, val, &next, base, BF_PREC_INF, 0);
    if (rc != 0) {
        kexit(-1);
        return NULL;
    }

    return obj;
}

KATA_API kint
kint_newu(u64 val) {
    kint obj = kobj_alloc(Kint);
    if (!obj) return NULL;

    // init and set to string
    bf_init(&Kbf_ctx, &obj->val);
    if (bf_set_ui(&obj->val, val) != 0) {
        kexit(-1);
        return NULL;
    }

    return obj;
}

KATA_API kint
kint_news(s64 val) {
    kint obj = kobj_alloc(Kint);
    if (!obj) return NULL;

    // init and set to string
    bf_init(&Kbf_ctx, &obj->val);
    if (bf_set_si(&obj->val, val) != 0) {

        kexit(-1);
        return NULL;
    }

    return obj;
}

