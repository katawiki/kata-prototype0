/* src/sys/rawio.c - ksys_rawio type implementation
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

struct ktype Ksys_rawio_, *Ksys_rawio = &Ksys_rawio_;

KATA_API ksys_rawio
ksys_rawio_newd(s32 fd_) {
    ksys_rawio obj = kobj_alloc(Ksys_rawio);
    if (!obj) return NULL;

    // set the properties
    obj->fd_ = fd_;

    return obj;
}

KATA_API void
kinit_sys_rawio() {

}
