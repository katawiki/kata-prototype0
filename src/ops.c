/* src/ops.c - 'kop_*' operator implementations
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


KATA_API kobj
kop_add(kobj a, kobj b) {
    ktype tpa = KOBJ_TYPE(a), tpb = KOBJ_TYPE(b);

    if (tpa == Kstr) return kstr_fmt("%S%S", a, b);

    if (tpa->is_float && tpb->is_float) {
        bool doa, dob;
        bf_t bfa, bfb;
        if (!kbf_const(a, &bfa, &doa)) return NULL;
        if (!kbf_const(b, &bfb, &dob)) {
            if (doa) kbf_done(&bfa);
            return NULL;
        }

        bf_t bfc;
        if (!kbf_init(&bfc, NULL)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            return NULL;
        }

        if (bf_add(&bfc, &bfa, &bfb, kbf_precy(&bfa, &bfb), BF_RNDF) & ~(BF_ST_INEXACT)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            kbf_done(&bfc);
            return NULL;
        }

        if (doa) kbf_done(&bfa);
        if (dob) kbf_done(&bfb);
        return (kobj)kint_newz(&bfc);
    }

    assert(false);
}

KATA_API kobj
kop_mul(kobj a, kobj b) {
    ktype tpa = KOBJ_TYPE(a), tpb = KOBJ_TYPE(b);

    if (tpa->is_float && tpb->is_float) {
        bool doa, dob;
        bf_t bfa, bfb;
        if (!kbf_const(a, &bfa, &doa)) return NULL;
        if (!kbf_const(b, &bfb, &dob)) {
            if (doa) kbf_done(&bfa);
            return NULL;
        }

        bf_t bfc;
        if (!kbf_init(&bfc, NULL)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            return NULL;
        }

        if (bf_mul(&bfc, &bfa, &bfb, kbf_precy(&bfa, &bfb), BF_RNDD) & ~(BF_ST_INEXACT)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            kbf_done(&bfc);
            return NULL;
        }

        if (doa) kbf_done(&bfa);
        if (dob) kbf_done(&bfb);
        return (tpa->is_int && tpb->is_int) ? (kobj)kint_newz(&bfc) : kfloat_newz(&bfc);
    }

    assert(false);
}

KATA_API kobj
kop_div(kobj a, kobj b) {
    ktype tpa = KOBJ_TYPE(a), tpb = KOBJ_TYPE(b);

    if (tpa->is_float && tpb->is_float) {
        bool doa, dob;
        bf_t bfa, bfb;
        if (!kbf_const(a, &bfa, &doa)) return NULL;
        if (!kbf_const(b, &bfb, &dob)) {
            if (doa) kbf_done(&bfa);
            return NULL;
        }

        bf_t bfc;
        if (!kbf_init(&bfc, NULL)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            return NULL;
        }

        if (bf_div(&bfc, &bfa, &bfb, kbf_precy(&bfa, &bfb), BF_RNDF) & ~(BF_ST_INEXACT)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            kbf_done(&bfc);
            return NULL;
        }

        if (doa) kbf_done(&bfa);
        if (dob) kbf_done(&bfb);
        return kfloat_newz(&bfc);
    }

    assert(false);
}

KATA_API kobj
kop_fdiv(kobj a, kobj b) {
    ktype tpa = KOBJ_TYPE(a), tpb = KOBJ_TYPE(b);

    if (tpa->is_float && tpb->is_float) {
        bool doa, dob;
        bf_t bfa, bfb;
        if (!kbf_const(a, &bfa, &doa)) return NULL;
        if (!kbf_const(b, &bfb, &dob)) {
            if (doa) kbf_done(&bfa);
            return NULL;
        }

        bf_t bfc;
        if (!kbf_init(&bfc, NULL)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            return NULL;
        }

        if ((bf_div(&bfc, &bfa, &bfb, kbf_precy(&bfa, &bfb), BF_RNDF) & ~(BF_ST_INEXACT)) || (bf_rint(&bfc, BF_RNDF) & ~(BF_ST_INEXACT))) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            kbf_done(&bfc);
            return NULL;
        }

        if (doa) kbf_done(&bfa);
        if (dob) kbf_done(&bfb);
        return (kobj)kint_newz(&bfc);
    }

    assert(false);
}



KATA_API kobj
kop_pow(kobj a, kobj b) {
    ktype tpa = KOBJ_TYPE(a), tpb = KOBJ_TYPE(b);

    if (tpa->is_float && tpb->is_float) {
        bool doa, dob;
        bf_t bfa, bfb;
        if (!kbf_const(a, &bfa, &doa)) return NULL;
        if (!kbf_const(b, &bfb, &dob)) {
            if (doa) kbf_done(&bfa);
            return NULL;
        }

        bf_t bfc;
        if (!kbf_init(&bfc, NULL)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            return NULL;
        }
        
        if (bf_pow(&bfc, &bfa, &bfb, kbf_precy(&bfa, &bfb), BF_RNDF) & ~(BF_ST_INEXACT)) {
            if (doa) kbf_done(&bfa);
            if (dob) kbf_done(&bfb);
            kbf_done(&bfc);
            return NULL;
        }

        if (doa) kbf_done(&bfa);
        if (dob) kbf_done(&bfb);
        return kfloat_newz(&bfc);
    }

    assert(false);
}

