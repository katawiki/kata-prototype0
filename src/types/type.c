/* src/types/type.c - implementation of ktype
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

KTYPE_DECL(Ktype);

KATA_API void
ktype_init(ktype tp, s32 sz, const char* name, const char* docs) {
    assert(tp != NULL);
    struct kobj_meta* meta = KOBJ_META(tp);

    meta->type = Ktype;
    meta->refc = 1;

    tp->sz = sz;
    tp->attr = kdict_new(NULL);
    assert(tp->attr != NULL);
    
    tp->name = kstr_new(-1, name);
    assert(tp->name != NULL);
    tp->docs = kstr_new(-1, docs);
    assert(tp->docs != NULL);

}

KATA_API void
ktype_merge(ktype tp, struct kdict_ikv* ikv) {
    assert(ikv != NULL);
    struct kdict_ikv* it = ikv;
    while (ikv->key != NULL) {
        kstr key = kstr_new(-1, ikv->key);
        assert(key != NULL);

        // TODO: check for particular values?
        // TODO: use a hash table?
        if (kstr_cmp(key, Ksc_del) == 0) {
            tp->fn_del = ikv->val;
        } else if (kstr_cmp(key, Ksc_repr) == 0) {
            tp->fn_repr = ikv->val;
        }

        // always set manually to the dictionary
        if (kdict_setx(tp->attr, key, key->hash, ikv->val) < 0) {
            kexit(-1);
        }

        KOBJ_DECREF(key);
        ikv++;
    }
}
