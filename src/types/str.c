/* src/types/str.c - implementation of kstr
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

KTYPE_DECL(Kstr);

KATA_API kstr
kstr_new(ssize lenb, const char* data) {
    if (lenb < 0) lenb = strlen(data);

    // pre-calculate the hash
    usize hash = kmem_hash(lenb, data);

    // TODO: intern strings
    struct kobj_meta* meta = kmem_make(sizeof(struct kobj_meta) + sizeof(struct kstr) + (lenb + 1));
    if (!meta) return NULL;
    kstr obj = KOBJ_UNMETA(meta);

    KOBJ_REFC(obj) = 1;
    KOBJ_TYPE(obj) = Kstr;

    // fill in calculated hash
    obj->hash = hash;

    obj->lenb = lenb;
    // TODO: UTF-8 support
    obj->lenc = lenb;

    // TODO: memcpy requirement
    memcpy(obj->data, data, lenb);

    // NUL-terminate the data
    obj->data[lenb] = '\0';

    return obj;
}
