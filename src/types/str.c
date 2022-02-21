/* src/types/str.c - implementation of kstr
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

KTYPE_DECL(Kstr);

KATA_API s32
kstr_cmp(kstr a, kstr b) {
    if (a == b) return 0;
    // get minimum (i.e. safe) length
    usize min_len = a->lenb > b->lenb ? b->lenb : a->lenb;
    // compare valid bytes, which will include the NUL-terminator
    s32 cv = memcmp(a->data, b->data, min_len+1);
    // return sign(cv)
    return cv < 0 ? -1 : (cv > 0 ? 1 : 0);
}

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

KATA_API kstr
kstr_fmt(const char* fmt, ...) {
    kbuffer io = kbuffer_new(0, NULL);
    if (!io) return NULL;
    va_list ap;
    va_start(ap, fmt);
    ssize sz = kprintfv(io, fmt, ap);
    va_end(ap);
    if (sz < 0) {
        KOBJ_DECREF(io);
        return NULL;
    }

    return kbuffer_strz(io);
}

KATA_API void
kinit_str() {
    ktype_init(Kstr, sizeof(struct kstr), "str", "String type");

}
