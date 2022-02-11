/* src/mem/init.c - initialize kmem
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>

/// INTERNALS ///


// (internal) compute a**b (mod m)
static usize
my_modpow(usize a, usize b, usize m) {

    // a**2**i (mod m)
    usize a2i = a;

    // the result product
    usize res = 1;

    // basically, iterate over the bits of 'b', and treat it
    //   as a bitset
    while (b) {
        if (b & 1) {
            // this power should be included, since its in the bitset
            res = (res * a2i) % m;
        }

        // compute, and apply modulo to avoid overflow:
        // a**2**(i+1) == (a**2**i)**2
        a2i = (a2i * a2i) % m;

        // now, shift the bitset
        b >>= 1;
    }

    return res;
}

// (internal) perform a witness check in Miller-Rabin, returns whether it is probably prime
//   a: the witness being tested
//   n: the number being checked for primality, n := 2**r * d + 1 (i.e. within the 2-adic number system)
//   r: see 'n'
//   d: see 'n', must be ODD
static bool
my_witness(usize a, usize n, usize r, usize d) {
    // compute a ** d (mod n)
    usize x = my_modpow(a, d, n);

    // special case that it's probably true for this witness
    if (x == 1 || x == n - 1) {
        return true;
    }

    // repeat (r-1) times
    usize ct;
    for (ct = 0; ct < r - 1; ct++) {
        x = (x * x) % n;
        if (x == n - 1) {
            // probably true as well
            return true;
        }
    }

    return false;
}

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
    // always has enough for 0 bytes
    if (!sz) return true;
    if (!*pptr) {
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
    *pcap = kmem_nextcap(*pcap, sz);

    // only now grow to the larger capacity
    return kmem_grow(pptr, *pcap) >= 0;
}


// get the next capacity, given a capacity, for a good default memory
//   reallocation scheme
KATA_API usize
kmem_nextcap(usize cap, usize sz) {
    if (cap >= sz) return cap;

    // TODO: better algorithm explanation
    return (sz + 2) * 2;
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


KATA_API usize
kmem_isprime(usize n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    if (n < 4096) {
        // for some values, it's faster to do a brute force approach
        // SEE: https://cade.site/2021/diy-fast-isprime
        // TODO: performance tune this?

        // do trial division, checking only the odd numbers <= sqrt(n)
        usize i;
        for (i = 3; i * i <= n; i += 2) {
            // divisible, so not prime
            if (n % i == 0) return false;
        }

        // no factors, so must be prime
        return true;
    }

    // compute: n := 2 ** r * d + 1
    // (i.e. decompose into 2-adic number)
    usize d = n - 1;
    usize r = 0;
    while (d % 2 == 0) {
        r++;
        d >>= 1;
    }

    // utility macro for a single witness
    #define WIT(_a) my_witness((_a), n, r, d)

    // use proven bounds
    // TODO: some of these seem suboptimal
    // SEE: https://cade.site/2021/diy-fast-isprime
    /**/ if (n < 2047ULL) return WIT(2);
    else if (n < 1373653ULL) return WIT(2) && WIT(3);
    else if (n < 9080191ULL) return WIT(31) && WIT(73);
    else if (n < 25326001ULL) return WIT(2) && WIT(3) && WIT(5);
    else if (n < 3215031751ULL) return WIT(2) && WIT(3) && WIT(5) && WIT(7);
    else if (n <= 0xFFFFFFFFULL) return WIT(2) && WIT(7) && WIT(61); // 32 bit values
    else {
        // this is a catch all, which should work up to
        // 18446744073709551616 = 2**64
        // so, this is only needed on 32 bit systems
        return WIT(2) && WIT(3) && WIT(5) && WIT(7) && WIT(11) && WIT(13) && WIT(17) && WIT(19) && WIT(23) && WIT(29) && WIT(31) && WIT(37);
    }
}

KATA_API usize
kmem_nextprime(usize n) {
    // get next odd number
    usize x = n % 2 == 0 ? n + 1 : n + 2;
    // advance to next prime
    while (!kmem_isprime(x)) x += 2;
    return x;
}


KATA_API void
kinit_mem() {

}
