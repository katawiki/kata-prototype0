/* src/types/dict.c - implementation of kdict, a highly performant (mutable) hash table
 *
 * Kata uses a lot of tricks that have been found to optimize hash table implementations:
 * 
 *   * variable datatype bucket sizes, for example use (u8*) when length<256, u16* when length<65536, etc.
 *   * use out-of-place entries array to avoid space waste for empty/deleted entries
 *   * use open addressing, for better cache locality
 *   * use hash-aware methods for pre-computed hashes
 *   * use prime length tables for even distributions (see 'src/mem/init.c' for utility function implementations)
 *   * use an exponential resizing scheme to amortize growth to O(1), and avoid reallocations (see 'src/mem/init.c')
 * 
 * as a result, there are some peculiarities to the implementation, including:
 *   
 *   * 'KDICT_PER_BUKS' (defined in 'include/kata/api.h'), to handle the different sized buckets array
 * 
 * 
 * ------------------------------------
 * - GENERAL OVERVIEW
 * 
 * the 'kdict' type (aka 'struct kdict*') is an object type in the Kata runtime (see https://kata.wtf),
 *   which is allocated and initialized via 'kdict_new()'. The dictionary structure stores a mapping of keys to values,
 *   and can be queried, iterated, and mutated.
 * 
 * fundamentally, this is a highly optimized hash table (https://en.wikipedia.org/wiki/Hash_table), that takes
 *   Kata objects as keys and values. Therefore, it is type-generic, and only requires that keys are immutable
 *   and hashable (for example, ints, floats, strings, tuples, maps, etc)
 *
 *
 * ------------------------------------
 * - REFERENCES
 *
 * originally based on kscript's dict implementation
 *   * https://kscript.org
 *   * https://github.com/ChemicalDevelopment/kscript/blob/main/src/types/dict.c
 *
 * some other useful links that explain concepts:
 *   * https://stackoverflow.com/questions/327311/how-are-pythons-built-in-dictionaries-implemented
 *   * https://en.wikipedia.org/wiki/Hash_table
 * 
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>

/// INTERNALS ///

// maximum load factor, when this is met or exceeded, the dictionary will be resized/rehashed
#define LOAD_MAX       0.7

// new target load factor, for when dictionaries are resized
#define LOAD_NEW       0.3

// calculate internal load factor, which is based on the length of entries, not the number
//   of real entries (i.e. this counts deleted entries as well, which is important because
//   deleted entries actually keep using up space in the table)
static double
my_load(struct kdict* obj) {
    return obj->buks_len > 0 ? (double)obj->ents_len / obj->buks_len : 0.0;
}

// search through the dictionary for a given key (given hash(key)), returns whether
//   it happened without error (i.e. if false, an exception was generated)
// sets 'rbi' and 'rei' to the index of the bucket and entry, respectively, or
//   -1 if the bucket/entry was not found
static bool
my_search(struct kdict* obj, kobj key, usize hash, ssize* rbi, ssize* rei) {
    if (obj->buks_len == 0) {
        // no elements, so avoid modulo by 0
        *rbi = *rei = -1;
        return true;
    }

    // calculate bucket index by probing modulo the array size
    usize bi = hash % obj->buks_len;

    // keep track of the original bucket index, to check for cycles, and the try index
    usize bi0 = bi, tries = 0;
    KDICT_PER_BUKS(obj, obj->buks_len, {
        do {
            ssize ei = BUKS[bi];
            if (ei == BUK_EMPTY) {
                // hit an empty bucket, so the key was not present
                // however, it can be inserted at this bucket index, so signal that
                *rbi = bi;
                *rei = -1;
                return true;
            } else if (ei == BUK_DEL) {
                // hit a deleted bucket, which we should skip to not break the hash
                //   chain
            } else if (obj->ents[ei].hash == hash) {
                // hashes match, so now check equality
                bool is_eq = obj->ents[ei].key == key;
                if (!is_eq) {
                    // different objects, so find out dynamically
                    if (!kobj_eq(obj->ents[ei].key, key, &is_eq)) return false;
                }

                if (is_eq) {
                    // found a match, so signal the position
                    *rbi = bi;
                    *rei = ei;
                    return true;
                }
            }

            // we are trying again, so update the count to calculate probes
            tries++;

            // probe for next bucket
            // TODO: allow for triangular, linear, quadratic, etc
            // linear probing
            //bi = (bi + 1) % obj->buks_len;

            // quadratic probing
            //bi = (bi0 + tries * tries) % obj->buks_len;

            // triangular probing, based on triangular number offsets
            // this may be faster than quadratic probing with similar results,
            //   since it is a quadratic function (x(x+1)/2), but easier to compute
            //   since F(x)-F(x-1)=x, thus skipping a multiplication
            bi = (bi + tries) % obj->buks_len;

        } while (bi != bi0);
    });

    // not found, and no room to insert, so signal that
    *rbi = -1;
    *rei = -1;
    return false;
}

// resize and rehash the hash table to hold at least 'new_buks_len' buckets
static bool
my_resize(struct kdict* obj, usize new_buks_len) {
    // if its already large enough, quit early
    if (obj->buks_len > new_buks_len) return true;

    // make sure the length is prime
    new_buks_len = kmem_nextprime(new_buks_len);

    // calculate the size requirement for the buckets array
    usize i, new_buks_sz = 0;
    KDICT_PER_BUKS(obj, obj->buks_len, {
        new_buks_sz = sizeof(*BUKS) * new_buks_len;
    });

    // reallocate the buckets buffer if possible
    if (new_buks_sz > 0) {
        if (!kmem_growx(&obj->buks, &obj->buks_cap, new_buks_sz)) return false;
    }

    // do rehashing
    usize ct = 0;
    obj->buks_len = new_buks_len;
    KDICT_PER_BUKS(obj, obj->buks_len, {
        // clear all buckets (effectively, an empty array)
        for (i = 0; i < obj->buks_len; ++i) {
            BUKS[i] = BUK_EMPTY;
        }

        // now, reinsert all entries
        for (i = 0; i < obj->ents_len; ++i) {
            struct kdict_ent* ent = &obj->ents[i];
            if (ent->key != NULL) {
                // entry is valid, so search for its new place
                ssize bi, ei;
                if (!my_search(obj, ent->key, ent->hash, &bi, &ei)) return false;
                assert(bi >= 0);
                assert(ei < 0);

                // let the bucket point to the entry
                usize newi = ct;
                BUKS[bi] = newi;

                // also, fill holes
                if (newi != i) obj->ents[newi] = obj->ents[i];

                ct++;
            }
        }
    });

    assert(obj->ents_real == ct);
    return true;

    //return self->ents_len * S_HOLES_MAX >= self->len_real ? s_fill_holes(self) : true;
}

/// C API ///

KTYPE_DECL(Kdict);

KATA_API kdict
kdict_new(struct kdict_ikv* ikv) {
    kdict obj = kobj_make(Kdict);
    if (!obj) return NULL;

    obj->buks_cap = obj->buks_len = 0;
    obj->buks = NULL;

    obj->ents_cap = obj->ents_len = obj->ents_real = 0;
    obj->ents = NULL;

    if (kdict_merge(obj, ikv) < 0) {
        KOBJ_DECREF(obj);
        return NULL;
    }

    return obj;
}

KATA_API keno
kdict_merge(struct kdict* obj, struct kdict_ikv* ikv) {
    if (!ikv) return 0;
    
    // now, add all elements
    struct kdict_ikv* it = ikv;
    while (it->key != NULL) {
        kstr okey = kstr_new(-1, it->key);

        if (!okey || kdict_seth(obj, (kobj)okey, okey->hash, it->val) < 0) {
            if (okey) KOBJ_DECREF(okey);
            KOBJ_DECREF(it->val);

            // remove references to other values that have not been eliminated
            while (it->key != NULL) {
                KOBJ_DECREF(it->val);
                it++;
            }
            return -1;
        }

        KOBJ_DECREF(it->val);
        KOBJ_DECREF(okey);

        it++;
    }

    return 0;
}

KATA_API keno
kdict_get(struct kdict* obj, kobj key, kobj* val) {
    usize hash;
    if (!kobj_hash(key, &hash)) return -1;
    return kdict_geth(obj, key, hash, val);
}

KATA_API keno
kdict_geth(struct kdict* obj, kobj key, usize hash, kobj* val) {
    // search for the key
    ssize bi, ei;
    if (!my_search(obj, key, hash, &bi, &ei)) return false;

    // if the key was not found, then return NULL
    if (ei < 0) return false;

    // otherwise, return the value
    *val = obj->ents[ei].val;
    return true;
}

KATA_API keno
kdict_set(struct kdict* obj, kobj key, kobj val) {
    usize hash;
    if (!kobj_hash(key, &hash)) return -1;
    return kdict_seth(obj, key, hash, val);
}

KATA_API keno
kdict_seth(struct kdict* obj, kobj key, usize hash, kobj val) {
    // resize if needed
    if (my_load(obj) > LOAD_MAX || obj->buks_len == 0) {
        if (!my_resize(obj, (usize)(1 + (double)obj->ents_len / LOAD_NEW))) {
            return false;
        }
    }

    // search for the key
    ssize bi, ei;
    if (!my_search(obj, key, hash, &bi, &ei)) return false;

    if (ei < 0) {
        // key not found, so insert it
        ei = obj->ents_len++;
        if (obj->ents_len > obj->ents_cap) {
            obj->ents_cap = kmem_nextcap(obj->ents_cap, obj->ents_len);
            if (!kmem_grow((void**)&obj->ents, sizeof(*obj->ents) * obj->ents_cap)) {
                return false;
            }
        }
        
        // add to entries
        obj->ents_real++;
        KOBJ_INCREF(key);
        KOBJ_INCREF(val);

        obj->ents[ei].hash = hash;
        obj->ents[ei].key = key;
        obj->ents[ei].val = val;

        // add buckets if needed, which is when we are out of buckets, or
        //   when we are expanding past the size limit of the current bucket size
        if (bi < 0 || (obj->ents_len == U8_MAX-2 || obj->ents_len == U16_MAX-2 || obj->ents_len == U32_MAX-2)) {
            if (!my_resize(obj, (usize)(1 + (double)obj->ents_len / LOAD_NEW))) {
                return false;
            }

            ssize new_ei;
            if (!my_search(obj, key, hash, &bi, &new_ei)) return false;
            assert(bi >= 0);
            assert(new_ei == ei);
        }

        // set the bucket to point to the entry
        KDICT_PER_BUKS(obj, obj->ents_len, {
            BUKS[bi] = ei;
        });

        return true;
    } else {
        // found, so replace the value
        KOBJ_INCREF(val);
        KOBJ_DECREF(obj->ents[ei].val);
        obj->ents[ei].val = val;
        return true;
    }
}
