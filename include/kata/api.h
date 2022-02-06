/* kata/api.h - Kata's C API standard, which defines the stable interface to Kata
 *
 * TODO: introduce a 'kio_bufio' for buffered IO primitives?
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_API_H
#define KATA_API_H

////////////////////////////////////////////////////////////////////////////////

/// C Config ///

//#define _GNU_SOURCE
//#define _POSIX_C_SOURCE 200112L

// C standard
#include <stdbool.h>
#include <stdint.h>

#include <stdarg.h>

#include <stdlib.h>
#include <stdio.h>

#include <assert.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <float.h>


#include <math.h>


////////////////////////////////////////////////////////////////////////////////

/// Kata Config ///

// Kata bit size, measured as number of bits used to represent any of:
//   * a pointer address
//   * a pointer index
//   * a size of memory
//
// This setting changes based on the platform, for example:
//   * on most modern day PCs, this should be 64 (for example, most native Linux/Windows/MacOS)
//   * on the web platform, this should be 32 (since things are sandboxed in WASM)
//   * on arduino/tiny builds this could possibly be 16 (depends on the architecture)
//
// For the most part, should always be 64, 32, or 16 though
#if UINTPTR_MAX == 0xffffffffffffffff
    #define KATA_BITS 64
#elif UINTPTR_MAX == 0xffffffff
    #define KATA_BITS 32
#else
    #error "Unexpected/unsupported platform (when testing UINTPTR_MAX, was not 32bit or 64bit)"
#endif

// Kata API function decorator
#define KATA_API extern


// libbf library, for 'bf_t' and arbitrary precision math
#include <kata/libbf.h>

////////////////////////////////////////////////////////////////////////////////

/// Kata Standard Types ///

// TODO: f16
// TODO: u128, s128, f128
// TODO: detect float/double/long double sizes

// unsigned integer types
typedef uint8_t        u8;
typedef uint16_t       u16;
typedef uint32_t       u32;
typedef uint64_t       u64;

// signed integer types
typedef int8_t         s8;
typedef int16_t        s16;
typedef int32_t        s32;
typedef int64_t        s64;


#define U8_MAX         UINT8_MAX
#define S8_MAX         INT8_MAX
#define U16_MAX        UINT16_MAX
#define S16_MAX        INT16_MAX
#define U32_MAX        UINT32_MAX
#define S32_MAX        INT32_MAX
#define U64_MAX        UINT64_MAX
#define S64_MAX        INT64_MAX




// floating point types
typedef float          f32;
typedef double         f64;

#define F32_DIG        FLT_DIG
#define F64_DIG        DBL_DIG

#define F64_INF        INFINITY
#define F64_NAN        NAN

// math constants
#define F64_TAU        6.2831853071795864769252867665590057683943387987502116419498891846156328125724179972560696506842341359
#define F64_PI         3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679
#define F64_E          2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274
#define F64_LN2        0.69314718055994530941723212145817656807550013436025525412068000949339362196969471560586332699641868754
#define F64_LN10       2.302585092994045684017991454684364207601101488628772976033327900967572609677352480235997205089598298



// set configuration depending on platform settings
#if KATA_BITS == 16
    typedef u16        usize;
    typedef s16        ssize;
#elif KATA_BITS == 32
    typedef u32        usize;
    typedef s32        ssize;
#elif KATA_BITS == 64
    typedef u64        usize;
    typedef s64        ssize;
#else
    #error "KATA_BITS must be 16, 32, or 64"
#endif


// Kata error code (see 'keno' typedef)
enum {

    // success/no error
    KENO_OK = 0,

    // Kata general/misc error
    KENO_ERR = -1,

    // out of memory (OOM)
    KENO_ERR_OOM = -20,

};

// Kata error code type
// (see 'KENO_*')
typedef ssize          keno;


/// Kata Types ///

// Kata object reference
typedef void*          kobj;

// extra size required for an object's meta
// NOTE: should be same as sizeof(ktype) == sizeof(usize)
#define KOBJ_EXTRA sizeof(struct kobj_meta)

// macro to retrieve the type of a Kata object
#define KOBJ_TYPE(_obj_) KOBJ_META(_obj_)->type

// macro to retrieve the reference count of a Kata object as an lval
// NOTE: this should really only be used internally
// TODO: this assumes sizeof(usize)==sizeof(ktype)
// TODO: what to do if building without ARC/refcounting?
#define KOBJ_REFC(_obj_) KOBJ_META(_obj_)->refc

// increment reference count
#define KOBJ_INCREF(obj_) do { \
    KOBJ_REFC(obj_)++; \
} while(0)

// decrement reference count
#define KOBJ_DECREF(obj_) do { \
    kobj obj__ = (kobj)(obj_); \
    if (!--KOBJ_REFC(obj__)) { \
        kobj_free(obj__); \
    }; \
} while(0)



// Kata integer, which is an arbitrary precision whole number
typedef struct kint {

    // numeric data, via libbf
    bf_t val;

}* kint;

// Kata float, which is an arbitrary precision floating point
// TODO: add context/precision?
typedef struct kfloat {

    // numeric data, via libbf
    bf_t val;

}* kfloat;


// Kata complex number, which has a real and imaginary part
// NOTE: even though they are marked as 'KOBJ', they cannot be
//         reassigned (since numbers are immutable)
typedef struct kcomplex {

    // real part
    kfloat re;

    // imaginary part
    kfloat im;

}* kcomplex;


// Kata bytestring
typedef struct kbytes {

    // length in bytes and characters
    usize lenb;

    // hash of the bytes, according to 'kmem_hash(lenb, data)'
    usize hash;

    // the raw data, allocated as part of the structure
    u8 data[0];

}* kbytes;

// Kata string, which is UTF-8
typedef struct kstr {

    // length in bytes and characters
    usize lenb, lenc;

    // hash of the string, according to 'kmem_hash(lenb, data)'
    usize hash;

    // UTF-8 encoded data, with NUL-terminator, stored
    //   as part of the string
    u8 data[0];

}* kstr;

// Kata tuple, which is basically an immutable array
typedef struct ktuple {
    
    // length of 'data', in elements
    usize len;

    // array of object references, which should be allocated
    //   as the structure
    kobj data[0];

}* ktuple;


typedef struct kbuffer {

    // length and capacity of 'data', in bytes
    usize len, cap;

    // current position, in bytes
    usize pos;

    // byte data of the buffer
    u8* data;

}* kbuffer;


// Kata list, which is an ordered collection of Kata objects
typedef struct klist {

    // length and capacity of 'data', in elements
    usize len, cap;

    // array of object references
    kobj* data;

}* klist;

// Kata dictionary entry, which is a key-value pair
typedef struct kdict_entry {

    // key and value of the entry
    kobj key, val;

    // hash(key)
    usize hash;

}* kdict_entry;


// Kata dictionary, a hash-table/dictionary/associative array of Kata objects
typedef struct kdict {

    // array of buckets
    // NOTE: the type depends on the length, see 'KDICT_PER_BUKS(...)'
    void* buks;

    // the length of buckets, in elements
    // NOTE: the type depends on the length, see 'KDICT_PER_BUKS(...)'
    usize buks_len;

    // the capacity of the buckets, in BYTES (NOT ELEMENTS)
    usize buks_cap;


    // array of entry structures, indexed by 'buks'
    struct kdict_entry* entrys;

    // the length and capacity of 'entrys', in elements
    usize entrys_len, entrys_cap;

}* kdict;

// perform some code per-each bucket type
#define KDICT_PER_BUKS(obj_, ...) do { \
    struct kdict* obj__ = (obj_); \
    if (obj__->buks_len <= U8_MAX - 2) { \
        u8* BUKS = obj__->buks; \
        __VA_ARGS__ \
    } else if (obj__->buks_len <= U16_MAX - 2) { \
        u16* BUKS = obj__->buks; \
        __VA_ARGS__ \
    } else if (obj__->buks_len <= U32_MAX - 2) { \
        u32* BUKS = obj__->buks; \
        __VA_ARGS__ \
    } else { \
        u64* BUKS = obj__->buks; \
        __VA_ARGS__ \
    } \
} while (0)


////////////////////////////////////////////////////////////////////////////////


// make a new integer with the given value
KATA_API kint
kint_new(const char* val, s32 base);
KATA_API kint
kint_newu(u64 val);
KATA_API kint
kint_news(s64 val);


// make a new str (or return an interned one)
// NOTE: pass 'lenb=-1' to indicate that the string is NUL-terminated
KATA_API kstr
kstr_new(ssize lenb, const char* data);


// make new tuple
KATA_API ktuple
ktuple_new(usize len, kobj* data);

// make new tuple, absorbing references from 'data'
KATA_API ktuple
ktuple_newz(usize len, kobj* data);


// make a new buffer
KATA_API kbuffer
kbuffer_new(usize len, const u8* data);

// init a buffer with the given data
KATA_API keno
kbuffer_init(struct kbuffer* obj, usize len, const u8* data);

// done with buffer, free resources
KATA_API void
kbuffer_done(struct kbuffer* obj);

// push extra data onto the buffer
KATA_API keno
kbuffer_push(struct kbuffer* obj, usize len, const u8* data);
// pop off the last 'len' bytes from the buffer
KATA_API keno
kbuffer_pop(struct kbuffer* obj, usize len);


// make new list
KATA_API klist
klist_new(usize len, kobj* data);

// make new list, absorbing references from 'data'
KATA_API klist
klist_newz(usize len, kobj* data);


// init list struct with a length and array of objects
// NOTE: 'data' must be legal objects
KATA_API keno
klist_init(struct klist* obj, usize len, kobj* data);

// done with list, free resources
KATA_API void
klist_done(struct klist* obj);

// push 'val' to the end of the list
KATA_API keno
klist_push(struct klist* obj, kobj val);

// push 'vals' to the end of the list
KATA_API keno
klist_pushn(struct klist* obj, usize len, kobj* vals);
// push 'vals' to the end of the list, absorbing references
KATA_API keno
klist_pushz(struct klist* obj, usize len, kobj* vals);


// pop an object from the end of the list
KATA_API kobj
klist_pop(struct klist* obj);
KATA_API bool
klist_popu(struct klist* obj);


// initializers for key and value (see KDICT_IKV(...))
struct kdict_ikv {
    
    // C-style string of the key
    const char* key;

    // value to be inserted
    kobj val;

};

// list of initializers and value (always include trailing ',')
// example: KDICT_IKV({"abc": abc}, {"def", some_res()},)
#define KDICT_IKV(...) ((struct kdict_ikv[]){ __VA_ARGS__ { NULL, NULL } })


// make a new dict
KATA_API kdict
kdict_new(struct kdict_ikv* ikv);

// init a dict struct with the given C-style initializers (or, NULL for empty)
KATA_API keno
kdict_init(struct kdict* obj, struct kdict_ikv* ikv);

// done with dict, free resources
KATA_API void
kdict_done(struct kdict* obj);

// get the value of the given key, or NULL if not found
KATA_API keno
kdict_get(struct kdict* obj, kobj key, kobj* val);
KATA_API keno
kdict_geth(struct kdict* obj, kobj key, usize hash, kobj* val);

// set the value of the given key to 'val'
KATA_API keno
kdict_set(struct kdict* obj, kobj key, kobj val);
KATA_API keno
kdict_seth(struct kdict* obj, kobj key, usize hash, kobj val);


////////////////////////////////////////////////////////////////////////////////

// implementation details/internals

enum {
    // none kind (i.e. forward declaration?)
    KFUNC_NONE   = 0x00,

    // C-style function being wrapped
    // TODO: differentiate from FFI functions?
    KFUNC_CFUNC  = 0x01,

    // KataBC (bytecode) function
    KFUNC_BFUNC  = 0x02,

};

// function parameters for a C-style function
// NOTE: you should always use the macro, and not rely on this!
#define KCFUNC_PARAMS kobj kfn, s32 nargs, kobj* vargs

// C-style function type
typedef kobj (*kcfunc)(KCFUNC_PARAMS);

// callable first-class function type
typedef struct kfunc {

    // the global name of the function
    // TODO: should C functions be in namespace 'c.name_of_symbol'?
    kstr name;

    // documentation string
    kstr docs;

    // the kind of funct
    kobj fn;

    // what kind of function is it
    // see 'KFUNC_*'
    u32 kind;

    // if 'kind & KFUNC_CFUNC'
    kcfunc cfunc_;

    // if 'kind & KFUNC_BFUNC'
    struct {

        // the actual encoded bytecode, as an array of bytes
        kbuffer data;

        // list of constant value references, typically shared with
        //   other functions
        klist vals;

        // position in 'data' of the start of the function
        u32 pos;

    } bfunc_;

}* kfunc;

// make a new C-style function wrapper
KATA_API kobj
kfunc_new(kcfunc cfunc, const char* name, const char* docs);

// Kata type, which is like a type/class/struct in other languages, it defines
//   a datatype and associated functions/attributes/methods for that type
// TODO: differentiate 
typedef struct ktype {

    /// Type Attributes ///

    // name of the type
    kstr name;

    // documentation string
    kstr docs;

    // the size of the type, which can be one of:
    //   like int: the size, in bytes of a single element (for statically sized types)
    //   like (obj)->int: a function that returns the size, in bytes for a given object
    //                      (for dynamically sized types, which is uncommon)
    s32 sz;


    /// Type Functions ///

    /// Lifetime Functions: init/done and new/del ///
    //
    // this is a simple explanation of how objects are managed in Kata, and how
    //   library authors should use them. generally, for best performance, the
    //   memory management functions should be implemented in C, JIT compiled, or
    //   otherwise optimized (since they run so many times)
    //
    // new/del are the functions used to create a new object and delete it,
    //   once the object is unreachable. these may internally call other functions
    //   to delete specific resources for that type
    //
    // init/done are used to initialize and finalize an object, and are called
    //   on stack allocated (or flatbuffer allocated, arena allocated, etc) instances. 
    //   these functions should NOT free the pointer that the object is stored at
    //

    // <type>.__new(tp, *args)-><type>
    //   allocation function that takes

    // <type>.__del(tp, obj)->void
    //   deallocation function that deletes the object itself

    kobj fn_new, fn_del;


    // <type>.__init(tp, obj, *args)->void
    //   initialization function that takes an allocated (but undefined) address of
    //   an object and initializes according to the arguments given
    // NOTE: may be NULL, if the type only uses 'fn_new' or doesn't need to be initialized

    // <type>.__done(tp, obj)->void
    //   deinitialization function that clears out any memory/references associated with 'obj'

    kobj fn_init, fn_done;

}* ktype;

// helper macro to declare a statically allocated type
#define KTYPE_DECL(name_) static u8 name_##_[sizeof(struct kobj_meta) + sizeof(struct ktype)]; \
    ktype name_ = (ktype)(name_##_ + sizeof(struct kobj_meta)); \

// initialize a type with the given information
KATA_API void
ktype_init(ktype tp, s32 sz, const char* name, const char* docs);

// meta data structucture stored before an object in memory
struct kobj_meta {

    // type and reference count
    ktype type;

    // reference count
    usize refc;

};

// get a reference to the meta for an object
#define KOBJ_META(obj_) (((struct kobj_meta*)(obj_))-1)

// given a meta pointer, get the object pointer
#define KOBJ_UNMETA(meta_) ((kobj)(((struct kobj_meta*)(meta_))+1))


// Kata frame, which is a activation/function record 
struct kframe {

    // the function being called
    kobj fn;

    // number of arguments passed to the function
    s32 nargs;
    // the arguments being passed to the function
    kobj* args;

    // the arguments as a list, if already made
    // NOTE: this may be NULL!
    klist args_list_;
    // the arguments as a tuple, if already made
    // NOTE: this may be NULL!
    ktuple args_tuple_;

};

// Kata thread, which is a first-class object containing runtime information
typedef struct kthread {

    // object stack, manipulated by bytecodes
    klist stk_obj;

    // data stack, manipulated by bytecodes/optimized compiled code
    kbuffer stk_data;


    // length and capacity of the 'stk_frames' array, in elements
    usize stk_frame_len, stk_frame_cap;

    // array of frames, which are pushed/popped by bytecodes
    struct kframe* stk_frame;

}* kthread;


// get the current thread that is executing
// NOTE: do NOT decref this! it doesn't return an extra reference!
KATA_API kthread
kthread_get();

// add a new frame
KATA_API keno
kthread_push_frame(struct kthread* obj, kobj fn, usize nargs, kobj* args);

// pop the current frame
KATA_API keno
kthread_pop_frame(struct kthread* obj);


////////////////////////////////////////////////////////////////////////////////

// globals, which start with 'K' (capital)

KATA_API ktype
Kint,
Kfloat,
Kstr,
Kbytes,
Ktuple,

Kbuffer,
Klist,
Kdict,
Kdict_entry,

Kfunc,
Ktype,
Kthread
;

// context for all of libbf
KATA_API bf_context_t
Kbf_ctx;


////////////////////////////////////////////////////////////////////////////////

// C-API

// try to initialize the kata library
// NOTE: if 'fail_on_err' is given, this will halt the program on error
// (this is wanted on some utilities, but if you are embedding kata
//   you should handle the return value yourself)
KATA_API keno
kinit(bool fail_on_err);

// exit the program, with a return code
KATA_API void
kexit(keno rc);

// make an empty object, and initialize its meta for a given type
// NOTE: the resulting object will be uninitialized (the metadata will be, though)
KATA_API kobj
kobj_make(ktype tp);

// free an object, according to its type constructor
// NOTE: do not call this directly, use 'KOBJ_DECREF()' instead
KATA_API void
kobj_free(kobj obj);

// get the value of an object converted to a particular C-style value
// NOTE: returns 0 on success, <0 on failure, and >0 with metadata
KATA_API keno
kobj_getu(kobj obj, u64* out);
KATA_API keno
kobj_gets(kobj obj, s64* out);
KATA_API keno
kobj_getf(kobj obj, f64* out);
KATA_API keno
kobj_getc(kobj obj, f64* outre, f64* outim); // complex numbers


// call 'fn(*args)', return a reference to the result or NULL if an exception
//   was thrown
KATA_API kobj
kcall(kobj fn, usize nargs, kobj* args);

// quick call, which may be optimized with short circuiting paths when
//   debugging is not important
KATA_API kobj
kqcall(kobj fn, usize nargs, kobj* args);

// read a sequence of bytes from a IO-like object
// NOTE: number of characters read returned, or <0 on error
KATA_API ssize
kread(kobj io, usize len, void* data);

// write a sequence of bytes to a IO-like object
// NOTE: number of characters written returned, or <0 on error
KATA_API ssize
kwrite(kobj io, usize len, const void* data);


// writing some primitive datatypes to an 'io'
KATA_API ssize
kwriteu(kobj io, u64 val, s8 base, s32 width);
KATA_API ssize
kwrites(kobj io, s64 val, s8 base, s32 width);
KATA_API ssize
kwritef(kobj io, f64 val, s8 base, s32 width, s32 prec);

// writing different serialization formats of 'obj' to 'io'
// for example, %B, %S, %R
KATA_API ssize
kwriteB(kobj io, kobj obj);
KATA_API ssize
kwriteS(kobj io, kobj obj);
KATA_API ssize
kwriteR(kobj io, kobj obj);

// do a C-style printf to an IO-like object, where the following are respected:
//   %u: u64 value
//   %s: s64 value
//   %f: f64 value
//   %O: kobj value, outputs '<TYPE @ ADDR>'
//   %B: kobj value, outputs 'bytes(val)'
//   %S: kobj value, outputs 'str(val)'
//   %R: kobj value, outputs 'repr(val)'
KATA_API ssize
kprintf(kobj io, const char* fmt, ...);
KATA_API ssize
kprintfv(kobj io, const char* fmt, va_list args);

////////////////////////////////////////////////////////////////////////////////


// standard modules
#include <kata/mem.h>
#include <kata/sys.h>


#endif // KATA_API_H
