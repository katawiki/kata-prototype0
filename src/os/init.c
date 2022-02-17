/* src/os/init.c - initialize kos
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

kos_rawio
Kos_stdout,
Kos_stderr,
Kos_stdin;


KATA_API void
kinit_os() {
    kinit_os_rawio();

    // wrap them
    Kos_stdin  = kos_rawio_newd(0);
    Kos_stdout = kos_rawio_newd(1);
    Kos_stderr = kos_rawio_newd(2);

}
