/* kata/bf.h - Kata's libbf module wrapper, for arbitrary precision
 *
 * NOTE: for the original sources, see 'kata/libbf.h'
 * 
 * this is basically the driver for math operations, precision selection, and so on
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

// internal allocation function for libbf
KATA_API void*
kbf_realloc(void *opaque, void *ptr, size_t sz);

// get the current working precision, in bits
KATA_API s64
kbf_prec();

// get the precision for an operation on operands with 'a' and 'b' bits of accuracy
KATA_API s64
kbf_prec2(s64 a, s64 b);

// get the precision of a structure
// NOTE: allows for 'prec<0' to autogenerate/use default, otherwise just return that
KATA_API s64
kbf_precx(const bf_t* a);

// get the precision for an operation on operands with 'a' and 'b' bits of accuracy
KATA_API s64
kbf_precy(const bf_t* a, const bf_t* b);

// return if we are in decimal mode
KATA_API bool
kbf_isdec();

////////////////////////////////////////////////////////////////////////////////

// context for all of libbf
KATA_API bf_context_t
kbf_ctx;

KATA_API ktype
Kbf
;

#endif // KATA_BF_H
