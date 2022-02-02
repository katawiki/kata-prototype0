/* kata/io.h - Kata's IO interface
 *
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_IO_H
#define KATA_IO_H


////////////////////////////////////////////////////////////////////////////////


// kio_bufio, Kata IO primitives/buffering utilities
typedef struct kio_bufio {

    // IO-like object, which is called on operations
    kobj io;

    // the capacity and length of the buffer, currently
    usize cap, len;

    // the data, which may or may not be allocated
    // for example, often a stack allocation is used
    u8* data;

}* kio_bufio;

// make a new IO-like object from the given io, capacity, and data
KATA_API void
kio_bufio_init(struct kio_bufio* obj, kobj io, usize cap, u8* data);

// done with kio_bufio, so flush and free the data
KATA_API void
kio_bufio_done(struct kio_bufio* obj);

// buffered read primitive
KATA_API ssize
kio_bufio_read(struct kio_bufio* obj, usize len, u8* data);

// buffered write primitive
KATA_API ssize
kio_bufio_write(struct kio_bufio* obj, usize len, const u8* data);
KATA_API ssize
kio_bufio_writex(struct kio_bufio* obj, usize len, const u8* data, s32 width);
KATA_API ssize
kio_bufio_writeu64(struct kio_bufio* obj, u64 val, s32 base, s32 width);
KATA_API ssize
kio_bufio_writes64(struct kio_bufio* obj, s64 val, s32 base, s32 width);
KATA_API ssize
kio_bufio_writef64(struct kio_bufio* obj, f64 val, s32 base, s32 width, s32 prec);
KATA_API ssize
kio_bufio_writeb(struct kio_bufio* obj, kobj val);
KATA_API ssize
kio_bufio_writes(struct kio_bufio* obj, kobj val);
KATA_API ssize
kio_bufio_writer(struct kio_bufio* obj, kobj val);

////////////////////////////////////////////////////////////////////////////////


// global types
KATA_API ktype
Kio_bufio
;


#endif // KATA_IO_H
