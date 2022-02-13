/* src/types/thread.c - implementation of kthread
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

KTYPE_DECL(Kthread);

KATA_API kthread
kthread_get() {

}

KATA_API bool
kthread_push_frame(struct kthread* obj, kobj fn, usize nargs, kobj* args) {

}

KATA_API bool
kthread_pop_frame(struct kthread* obj) {
    if (obj->stk_frame_len == 0) {
        return false;
    }

    struct kframe* frm = &obj->stk_frame[--obj->stk_frame_len];

    // clear references on frame
    KOBJ_DECREF(frm->fn);

    usize i;
    for (i = 0; i < frm->nargs; ++i) {
        KOBJ_DECREF(frm->args[i]);
    }

    return true;
}
