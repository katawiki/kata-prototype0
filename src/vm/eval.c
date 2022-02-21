/* src/vm/eval.c - kvm_eval() implementation, a high level approach using AST walking
 *
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>
#include <kata/ks.h>
#include <kata/vm.h>

/// C API ///

KATA_API kobj
kvm_eval(struct kvm* vm, kobj scope, kobj node) {
    ktype tp = KOBJ_TYPE(node);
    if (tp == Ks_ast) {
        ks_ast n = (ks_ast)node;
        switch (n->kind)
        {
            case KS_AST_VAL: return KOBJ_NEWREF(n->sub);
            case KS_AST_CALL: {
                ktuple sub = kcheck(n->sub, Ktuple);
                if (!sub) return NULL;
                assert(sub->len >= 1);

                klist sl = klist_new(0, NULL);

                assert(sl != NULL);
                usize i;
                for (i = 0; i < sub->len; ++i) {
                    kobj o = kvm_eval(vm, scope, sub->data[i]);

                    if (!o) {
                        KOBJ_DECREF(sl);
                        return NULL;
                    }

                    if (!klist_push(sl, o)) {
                        KOBJ_DECREF(sl);
                        KOBJ_DECREF(o);
                        return NULL;
                    }

                    KOBJ_DECREF(o);
                
                }
                // actually perform call
                kobj res = kcall(sl->data[0], sl->len - 1, &sl->data[1]);
                KOBJ_DECREF(sl);
                return res;
            }

            case KS_AST_ADD: {
                ktuple sub = kcheck(n->sub, Ktuple);
                if (!sub) return NULL;

                assert(sub->len == 2);
                kobj a = kvm_eval(vm, scope, sub->data[0]);
                if (!a) return NULL;
                kobj b = kvm_eval(vm, scope, sub->data[1]);
                if (!b) {
                    KOBJ_DECREF(a);
                    return NULL;
                }
                kobj res = kop_add(a, b);
                KOBJ_DECREF(a);
                KOBJ_DECREF(b);
                return res;
            }
            case KS_AST_MUL: {
                ktuple sub = kcheck(n->sub, Ktuple);
                if (!sub) return NULL;

                assert(sub->len == 2);
                kobj a = kvm_eval(vm, scope, sub->data[0]);
                if (!a) return NULL;
                kobj b = kvm_eval(vm, scope, sub->data[1]);
                if (!b) {
                    KOBJ_DECREF(a);
                    return NULL;
                }
                kobj res = kop_mul(a, b);
                KOBJ_DECREF(a);
                KOBJ_DECREF(b);
                return res;
            }
            case KS_AST_DIV: {
                ktuple sub = kcheck(n->sub, Ktuple);
                if (!sub) return NULL;

                assert(sub->len == 2);
                kobj a = kvm_eval(vm, scope, sub->data[0]);
                if (!a) return NULL;
                kobj b = kvm_eval(vm, scope, sub->data[1]);
                if (!b) {
                    KOBJ_DECREF(a);
                    return NULL;
                }
                kobj res = kop_div(a, b);
                KOBJ_DECREF(a);
                KOBJ_DECREF(b);
                return res;
            }
            case KS_AST_POW: {
                ktuple sub = kcheck(n->sub, Ktuple);
                if (!sub) return NULL;

                assert(sub->len == 2);
                kobj a = kvm_eval(vm, scope, sub->data[0]);
                if (!a) return NULL;
                kobj b = kvm_eval(vm, scope, sub->data[1]);
                if (!b) {
                    KOBJ_DECREF(a);
                    return NULL;
                }
                kobj res = kop_pow(a, b);
                KOBJ_DECREF(a);
                KOBJ_DECREF(b);
                return res;
            }
        }
    } else {
        assert(false);
        return KOBJ_NEWREF(node);
    }

    assert(false);
    return NULL;
}

