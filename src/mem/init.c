/* src/mem/init.c - initialize kmem
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


/// C API ///

KATA_API void*
kmem_make(usize sz) {
    // special case, no allocation but also no failure
    if (!sz) return NULL;
    // use C library
    return malloc(sz);
}

KATA_API bool
kmem_grow(void** pptr, usize sz) {
    if (!*pptr) {
        if (!sz) return true;
        *pptr = kmem_make(sz);
        return *pptr != NULL;
    }

    // use C library, and check return... this should behave
    //   like this on WASM at least
    void* newptr = realloc(*pptr, sz);
    if (!newptr) {
        // error
        return false;
    }

    *pptr = newptr;
    return true;
}

KATA_API bool
kmem_growx(void** pptr, usize* pcap, usize sz) {
    // already have enough capacity
    if (*pcap >= sz) return true;
    // TODO: better algorithm explanation
    *pcap = (sz + 2) * 2;

    // only now grow to the larger capacity
    return kmem_grow(pptr, *pcap) >= 0;
}


KATA_API void
kmem_free(void* ptr) {
    free(ptr);
}

KATA_API usize
kmem_hash(usize len, const u8* data) {
    // accumulate according to djb-2
    // SEE: http://www.cse.yorku.ca/~oz/hash.html
    usize res = 5381 /* a magic number that works well */, i;
    for (i = 0; i < len; i++) {
        res = res * 33 + data[i];
    }
    return res;
}
