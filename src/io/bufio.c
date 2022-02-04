/* src/io/bufio.c - implementation of kio_bufio
 *
 * TODO: actually buffer
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>



/// C API ///

struct ktype Kio_bufio_, *Kio_bufio = &Kio_bufio_;

// digit string
static const char* digits = "0123456789ABCDEF";


KATA_API void
kio_bufio_init(struct kio_bufio* obj, kobj io, usize cap, u8* data) {
    obj->io = io;
    KOBJ_INCREF(io);
    obj->cap = cap;
    obj->len = 0;
    obj->data = data;
}

KATA_API void
kio_bufio_done(struct kio_bufio* obj) {
    KOBJ_DECREF(obj->io);
}

KATA_API ssize
kio_bufio_read(struct kio_bufio* obj, usize len, u8* data) {
    return kread(obj->io, len, data);
}

KATA_API ssize
kio_bufio_write(struct kio_bufio* obj, usize len, const u8* data) {
    return kio_bufio_writex(obj, len, data, -1);
}

KATA_API ssize
kio_bufio_writex(struct kio_bufio* obj, usize len, const u8* data, s32 width) {
    // TODO: actually buffer
    return kwrite(obj->io, len, data);
}

KATA_API ssize
kio_bufio_writeu64(struct kio_bufio* obj, u64 val, s32 base, s32 width) {
    assert(2 <= base && base <= 16);

    u8 tmp[128];
    ssize i = 0;
    bool is_neg = val < 0;
    if (is_neg) tmp[i++] = '-';

    // emit digits
    do {
        // current digit
        s32 cdig = val % base;
        if (cdig < 0) cdig = -cdig;

        // now, extract and emit digit
        val /= base;
        tmp[i++] = digits[cdig];
    } while (val > 0);

    // now, reverse buffer
    ssize j, k;
    for (j = is_neg?1:0, k = i-1; j < k; j++, k--) {
        u8 t = tmp[j];
        tmp[j] = tmp[k];
        tmp[k] = t;
    }

    // pad with zeros
    // TODO: make more efficient
    if (width > 0) {
        for (j = i; j < width; j++) {
            if (!kio_bufio_write(obj, 1, "0")) return -1;
        }
        i += width;
    }

    return i;
}

KATA_API ssize
kio_bufio_writes64(struct kio_bufio* obj, s64 val, s32 base, s32 width) {
    assert(2 <= base && base <= 16);

    u8 tmp[128];
    ssize i = 0;
    bool is_neg = val < 0;
    if (is_neg) tmp[i++] = '-';

    // emit digits
    do {
        // current digit
        s32 cdig = val % base;
        if (cdig < 0) cdig = -cdig;

        // now, extract and emit digit
        val /= base;
        tmp[i++] = digits[cdig];
    } while (val > 0);

    // now, reverse buffer
    ssize j, k;
    for (j = is_neg?1:0, k = i-1; j < k; j++, k--) {
        u8 t = tmp[j];
        tmp[j] = tmp[k];
        tmp[k] = t;
    }

    // pad with zeros
    // TODO: make more efficient
    if (width > 0) {
        for (j = i; j < width; j++) {
            if (!kio_bufio_write(obj, 1, "0")) return -1;
        }
        i += width;
    }

    return i;
}

KATA_API ssize
kio_bufio_writef64(struct kio_bufio* obj, f64 val, s32 base, s32 width, s32 prec) {

    // temporary buffer
    char buf[256];
    size_t res = snprintf(buf, sizeof(buf), "%.*lf", (int)F64_DIG, (double)val);
    assert(res < sizeof(buf) - 1);

    // remove trailing zeros
    // TODO: make more efficient
    // TODO: this doesn't work with hex
    while (res > 2 && (buf[res-1] == '0' && ('0' <= buf[res-2] && buf[res-2] <= '9'))) {
        res--;
    }

    // write directly
    return kio_bufio_write(obj, res, (u8*)buf);
}


KATA_API ssize
kio_bufio_writeb(struct kio_bufio* obj, kobj val) {
    ktype tp = KOBJ_TYPE(val);
    if (tp == Kstr) {
        return kio_bufio_write(obj, ((kstr)val)->lenb, ((kstr)val)->data);
    } else {
        // TODO: give error
        return -1;
    }
}

KATA_API ssize
kio_bufio_writes(struct kio_bufio* obj, kobj val) {
    ktype tp = KOBJ_TYPE(val);
    if (tp == Kstr) {
        return kio_bufio_write(obj, ((kstr)val)->lenb, ((kstr)val)->data);
    } else {
        // TODO: give error
        return -1;
    }
}

KATA_API ssize
kio_bufio_writer(struct kio_bufio* obj, kobj val) {
    ktype tp = KOBJ_TYPE(val);
    if (tp == Kstr) {
        return kio_bufio_write(obj, ((kstr)val)->lenb, ((kstr)val)->data);
    } else if (tp == Kint) {
        // TODO: faster ways to dump?
        size_t len;
        char* data = bf_ftoa(&len, &((kint)val)->val, 10, 0, BF_FTOA_FORMAT_FRAC);
        return kio_bufio_write(obj, len, data);
    } else {
        // TODO: give error
        return -1;
    }
}

KATA_API void
kinit_io_bufio() {

}
