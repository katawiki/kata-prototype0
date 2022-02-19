/* src/ks/init.c - initialize ks
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

KATA_API void
kinit_ks() {
    kinit_ks_token();
    kinit_ks_ast();

}
