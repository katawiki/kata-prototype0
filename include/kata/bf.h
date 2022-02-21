/* kata/bf.h - Kata's libbf module wrapper, for arbitrary precision
 *
 * NOTE: for the original sources, see 'kata/libbf.h'
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_BF_H
#define KATA_BF_H

#ifndef KATA_API_H
  #include <kata/api.h>
#endif

////////////////////////////////////////////////////////////////////////////////

// initialize 'obj' from 'val' (or, if 'val' is NULL, do 0)
// NOTE: be sure to use 'kbf_done()' to free the resources
KATA_API bool
kbf_init(bf_t* obj, kobj val);

// set 'obj' to a constant value from 'val'
// NOTE: the value must not be modified!
// NOTE: sets '*dodone' to whether or not 'kbf_done()' should
//         be called on 'obj'
KATA_API bool
kbf_const(kobj val, bf_t* obj, bool* dodone);


// free and finish usage of 'obj'
KATA_API void
kbf_done(bf_t* obj);



////////////////////////////////////////////////////////////////////////////////

KATA_API ktype
Kbf
;

#endif // KATA_BF_H
