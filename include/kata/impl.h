/* kata/impl.h - DO NOT INCLUDE THIS FILE IN YOUR PROJECT!
 *
 * This is an internal implementation header, meant only for files compiled
 *   in the Kata project. Do not rely on anything in this header
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_IMPL_H
#define KATA_IMPL_H

// include main Kata API
#ifndef KATA_API_H
  #include <kata/api.h>
#endif



// module initializers
KATA_API void
kinit_sys();
KATA_API void
kinit_sys_rawio();
KATA_API void
kinit_io();
KATA_API void
kinit_io_bufio();

#endif // KATA_IMPL_H
