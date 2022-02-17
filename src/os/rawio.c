/* src/os/rawio.c - kos_rawio type implementation
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

struct ktype Kos_rawio_, *Kos_rawio = &Kos_rawio_;

KATA_API kos_rawio
kos_rawio_newd(s32 fd_) {
    kos_rawio obj = kobj_make(Kos_rawio);
    if (!obj) return NULL;

    // set the properties
    obj->fd_ = fd_;

    return obj;
}

KATA_API void
kinit_os_rawio() {

}
