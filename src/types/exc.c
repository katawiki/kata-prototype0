/* src/types/exc.c - implementation of kexc
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

KTYPE_DECL(Kexc);

KATA_API kexc
kexc_new(ktype tp, s32 nargs, kobj* vargs, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    kobj res = kexc_newv(tp, nargs, vargs, fmt, ap);
    va_end(ap);
    return res;
}

KATA_API kexc
kexc_newv(ktype tp, s32 nargs, kobj* vargs, const char* fmt, va_list ap) {
    assert(tp->sz >= sizeof(struct kexc));

    kbuffer buf = kbuffer_new(0, NULL);
    if (!buf) return NULL;

    ssize sz = kprintfv((kobj)buf, fmt, ap);
    if (sz < 0) {
        KOBJ_DECREF(buf);
        return NULL;
    }

    // convert to string
    kstr msg = kbuffer_strz(buf);
    if (!msg) return NULL;

    kexc obj = kobj_make(tp);
    if (!obj) {
        KOBJ_DECREF(msg);
        return NULL;
    }

    obj->msg = msg;
    obj->args = ktuple_new(nargs, vargs);

    return obj;
}


KATA_API void
kinit_exc() {
    ktype_init(Kexc, sizeof(struct kexc), "Exception", "Exception type");

}
