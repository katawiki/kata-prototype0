/* src/io/init.c - initialize kio
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

KATA_API void
kinit_io() {
    kinit_io_bufio();
}
