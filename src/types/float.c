/* src/types/float.c - implementation of kfloat
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

KTYPE_DECL(Kfloat);

KATA_API kfloat
kfloat_new(const char* val, s32 base, s64 prec) {
    kfloat obj = kobj_make(Kfloat);
    if (!obj) return NULL;

    if (prec == 0) {
        // TODO: get current default
        assert(false);
    } else if (prec < 0) {
        // use infinite precision (i.e. whatever is neccessary)
        prec = BF_PREC_INF;
    } else {
        // assume it is in # of bits
    }

    // init and set to string
    bf_init(&Kbf_ctx, &obj->val);
    const char* next = NULL;
    //int rc = bf_atof(&obj->val, val, &next, base, prec, BF_RNDF);
    slimb_t vexp = 0;
    int rc = bf_atof2(&obj->val, &vexp, val, &next, base, BF_PREC_INF, BF_RNDF);
    if (rc != 0 && rc != BF_ST_INEXACT) {
        kexit(-1);
        return NULL;
    }
    
    return obj;
}

KATA_API kfloat
kfloat_newu(u64 val) {
    kfloat obj = kobj_make(Kfloat);
    if (!obj) return NULL;

    // init and set to string
    bf_init(&Kbf_ctx, &obj->val);
    if (bf_set_ui(&obj->val, val) != 0) {
        kexit(-1);
        return NULL;
    }

    return obj;
}

KATA_API kfloat
kfloat_news(s64 val) {
    kfloat obj = kobj_make(Kfloat);
    if (!obj) return NULL;

    // init and set to string
    bf_init(&Kbf_ctx, &obj->val);
    if (bf_set_si(&obj->val, val) != 0) {
        kexit(-1);
        return NULL;
    }

    return obj;
}



KATA_API kfloat
kfloat_newf(f64 val) {
    kfloat obj = kobj_make(Kfloat);
    if (!obj) return NULL;

    // init and set to double (f64)
    bf_init(&Kbf_ctx, &obj->val);
    if (bf_set_float64(&obj->val, val) != 0) {
        kexit(-1);
        return NULL;
    }
    
    return obj;
}

KATA_API void
kinit_float() {

}

