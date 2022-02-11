/* kata/mem.h - Kata's memory interface and utilities
 *
 * This also includes some math, hashing, and other utilities not directly
 *   tied to memory
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_MEM_H
#define KATA_MEM_H


// make a memory block of a given size
// NOTE: only pass these to 'kmem_grow' or 'kmem_growx' or 'kmem_free'
KATA_API void*
kmem_make(usize sz);

// grow a memory block allocated with 'kmem_make', to have at least
//   'sz' bytes allocated
// NOTE: if '*pptr' is NULL, then a new block is allocated
KATA_API bool
kmem_grow(void** pptr, usize sz);

// grow a memory block, extra interface
// pass a pointer to the current capacity, to grow appropriately. this
//   function only calls 'kmem_grow' in a logarithmic fashion, so on
//   average a reallocation is O(1) (instead of (O(N)))
// NOTE: if '*pptr' is NULL, then a new block is allocated
KATA_API bool
kmem_growx(void** pptr, usize* pcap, usize sz);

// get the next capacity, given a capacity, for a good default memory
//   reallocation scheme
KATA_API usize
kmem_nextcap(usize cap, usize sz);

// free a memory block allocate with 'kmem_make'
KATA_API void
kmem_free(void* ptr);

// hash memory contents, according to to djb2 internal algorithm
// TODO: have routine that fuses the length and hash operation,
//         to only loop once?
KATA_API usize
kmem_hash(usize len, const u8* data);


// return whether 'n' is prime
KATA_API usize
kmem_isprime(usize n);

// return the next prime > 'n'
KATA_API usize
kmem_nextprime(usize n);



#endif // KATA_MEM_H
