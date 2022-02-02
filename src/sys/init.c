/* src/sys/init.c - initialize ksys
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

ksys_rawio
Ksys_stdout,
Ksys_stderr,
Ksys_stdin;


KATA_API void
kinit_sys() {
    kinit_sys_rawio();

    // wrap them
    Ksys_stdin  = ksys_rawio_newd(0);
    Ksys_stdout = ksys_rawio_newd(1);
    Ksys_stderr = ksys_rawio_newd(2);

}
