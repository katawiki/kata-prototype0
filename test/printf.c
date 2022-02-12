/* test/str.c - testing 'kprintf'
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/test.h>

int main(int argc, char** argv) {
    kinit(true);

    assert(kprintf(Ksys_stdout, "basic floats:\n  0.0=%f\n  1.0=%f\n  -1.0=%f\n  1.2345=%f\n  -1.2345=%f\n", 0.0, 1.0, -1.0, 1.2345, -1.2345) >= 0);
    assert(kprintf(Ksys_stdout, "constants:\n  tau=%f\n  pi=%f\n  e=%f\n  ln(2)=%f\n  ln(10)=%f\n", F64_TAU, F64_PI, F64_E, F64_LN2, F64_LN10) >= 0);
    assert(kprintf(Ksys_stdout, "special values:\n  inf=%f\n  -inf=%f\n  nan=%f\n  -0.0=%f\n", F64_INF, -F64_INF, F64_NAN, -0.0) >= 0);


    kstr xyz = kstr_new(3, "xyz");
    assert(xyz != NULL);
    assert(xyz->lenb == 3);
    assert(xyz->lenc == 3);
    assert(strcmp(xyz->data, "xyz") == 0);
    assert(xyz->data[3] == 0x00);

    assert(kprintf(Ksys_stdout, "hw: %R\n", (kobj)xyz) >= 0);
    KOBJ_DECREF(xyz);

    kint x = kint_new("11234128364921612387642197642917542795492144264912653423", 10);
    assert(x != NULL);
    assert(kprintf(Ksys_stdout, "x: %R\n", (kobj)x) >= 0);
    KOBJ_DECREF(x);

    kfloat y = kfloat_new("123456789.0987654321012345678909876543210", 10, KFLOAT_PREC_INF);
    assert(y != NULL);
    assert(kprintf(Ksys_stdout, "y: %R\n", (kobj)y) >= 0);
    KOBJ_DECREF(y);

    // create C escape strings
    // SEE: https://en.wikipedia.org/wiki/Escape_sequences_in_C
    kstr escs = kstr_new(-1, "\a\b\t\n\v\f\r\"");
    assert(escs != NULL);
    assert(kprintf(Ksys_stdout, "escs: %R\n", (kobj)escs) >= 0);
    KOBJ_DECREF(escs);

    // create a tuple, absorbing object references
    ktuple t = ktuple_newz(3, (kobj[]) {
        kint_new("4", 10),
        kint_new("5", 10),
        kint_new("6", 10),
    });
    assert(t != NULL);
    assert(kprintf(Ksys_stdout, "t: %R\n", (kobj)t) >= 0);
    KOBJ_DECREF(t);


    // create a list, absorbing object references
    klist l = klist_newz(3, (kobj[]) {
        kint_new("1", 10),
        kint_new("2", 10),
        kint_new("3", 10),
    });
    assert(l != NULL);
    assert(kprintf(Ksys_stdout, "l: %R\n", (kobj)l) >= 0);
    KOBJ_DECREF(l);

    // create a dict, absorbing object references
    kdict d = kdict_new(KDICT_IKV(
        { "foobar", (kobj)kint_new("12345", 10) },
        { "blahblah", (kobj)kstr_new(-1, "some useless info...") },
        { "blah", (kobj)kstr_new(-1, "even more useless info") },
    ));
    assert(d != NULL);
    assert(kprintf(Ksys_stdout, "d: %R\n", (kobj)d) >= 0);
    KOBJ_DECREF(d);
    
    return 0;
}

