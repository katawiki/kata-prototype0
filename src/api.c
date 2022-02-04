/* src/api.c - implementation of the Kata C API functions
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>

bf_context_t
Kbf_ctx;

// internal allocation function for libbf
static void*
Kbf_ctx_realloc_(void *opaque, void *ptr, size_t sz) {
    void* res = ptr;
    if (!kmem_grow(&res, sz)){ 
        fprintf(stderr, "libbf: out of memory\n");
        kexit(KENO_ERR_OOM);
        return NULL;
    }
    return res;
}


KATA_API keno
kinit(bool fail_on_err) {
    Kint->sz = sizeof(struct kint);
    Ksys_rawio->sz = sizeof(struct ksys_rawio);

    bf_context_init(&Kbf_ctx, Kbf_ctx_realloc_, NULL);

    kinit_sys();
    kinit_io();

    return true;
}

KATA_API void
kexit(keno rc) {
    if (rc == 0) {
        exit(0);
    } else {
        // TODO: print return code
        fprintf(stderr, "kexit: rc=%i\n", (int)rc);
        exit(1);
    }
}

KATA_API kobj
kobj_alloc(ktype tp) {
    assert(tp != NULL);
    struct kobj_meta* meta = kmem_make(sizeof(struct kobj_meta) + tp->sz);
    if (!meta) return NULL;

    // initialize object meta
    meta->type = tp;
    meta->refc = 1;

    return KOBJ_UNMETA(meta);
}

KATA_API void
kobj_free(kobj obj) {
    ktype type = KOBJ_TYPE(obj);
    kobj res;

    if (type->fn_done) {
        // call uninitializer
        res = kcall(type->fn_done, 1, &obj);
        if (!res) kexit(-1);
        KOBJ_DECREF(res);
    }

    if (type->fn_del) {
        // call deleter
        res = kcall(type->fn_del, 1, &obj);
        if (!res) kexit(-1);
        KOBJ_DECREF(res);
    } else {
        // use default delete, which is to delete the memory
        kmem_free(KOBJ_META(obj));
    }
}

KATA_API keno
kobj_getu(kobj obj, u64* out) {
    ktype tp = KOBJ_TYPE(obj);
    if (tp == Kint) {

    } else {
        
    }


}

KATA_API keno
kobj_gets(kobj obj, s64* out) {

}

KATA_API keno
kobj_getf(kobj obj, f64* out) {

}

KATA_API keno
kobj_getc(kobj obj, f64* outre, f64* outim) {

}


KATA_API kobj
kcall(kobj fn, usize nargs, kobj* args) {
    
    // get the current thread structure
    kthread thd = kthread_get();

    // enter the function
    if (kthread_push_frame(thd, fn, nargs, args) < 0) {
        return NULL;
    }

    // run the function, depending on the type
    // NOTE: these are some hard-coded common cases
    ktype tp = KOBJ_TYPE(fn);
    if (tp == Kfunc) {

    }
}

KATA_API ssize
kread(kobj io, usize len, u8* data) {
    ktype tp = KOBJ_TYPE(io);
    if (tp == Kbuffer) {
        // buffer read
        kbuffer tio = (kbuffer)io;

        // get possible bytes to read
        ssize rsz = tio->len - tio->pos;
        // clamp to what's requested
        if (rsz > len) rsz = len;

        // copy and shift position
        memcpy(data, tio->data + tio->pos, rsz);
        tio->pos += rsz;

        return rsz;
    } else if (tp == Ksys_rawio) {
        // C-style read
        ksys_rawio tio = (ksys_rawio)io;
        ssize rsz = read(tio->fd_, data, len);
        if (rsz < 0) return -1;

        return rsz;
    } else {
        // TODO: throw error?
        return -1;
    }
}

KATA_API ssize
kwrite(kobj io, usize len, const u8* data) {
    ktype tp = KOBJ_TYPE(io);
    if (tp == Kbuffer) {
        // buffer read
        kbuffer tio = (kbuffer)io;

        // get possible bytes to read
        ssize rsz = tio->cap - tio->pos;
        // clamp to what's requested
        if (rsz > len) rsz = len;
        if (rsz < len) {
            // not enough, so grow buffer
            if (!kmem_growx((void**)&tio->data, &tio->cap, tio->pos + len)) {
                return -1;
            }
            rsz = len;
        }
        // should have enough space now
        assert(rsz == len);
        // copy and shift position
        memcpy(tio->data + tio->pos, data, rsz);
        tio->pos += rsz;

        return rsz;
    } else if (tp == Ksys_rawio) {
        // C-style write
        ksys_rawio tio = (ksys_rawio)io;
        ssize rsz = write(tio->fd_, data, len);
        if (rsz < 0) return -1;

        return rsz;
    } else {
        // TODO: throw error?
        return -1;
    }
}

KATA_API ssize
kprintf(kobj io, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ssize res = kprintfv(io, fmt, args);
    va_end(args);
    return res;
}


// buffer size to use internally
#define KPRINTF_BUFSIZ 1024

KATA_API ssize
kprintfv(kobj io, const char* fmt, va_list args) {
    const char* ofmt = fmt;
    char c;

    // buffer all output smaller than this
    u8 buf[KPRINTF_BUFSIZ];
    ssize rsz = 0, sz;

    // create temporary buffer to emit to
    struct kio_bufio bio;
    kio_bufio_init(&bio, io, KPRINTF_BUFSIZ, buf);

    while (*fmt) {
        // capture current format start
        const char* cfmt = fmt;

        // skip all non-control character
        while (*fmt && *fmt != '%') fmt++;

        // attempt to write to buffered io
        sz = kio_bufio_write(&bio, (usize)(fmt - cfmt), (const u8*)cfmt);
        if (sz < 0) return sz;
        rsz += sz;


        if (*fmt == '%') {
            // have a format specifier
            fmt++;
            // width and precision
            s32 width = -1, prec = -1;
            bool havePlus = false, haveMinus = false, haveSpace = false, haveZero = false;

            while ((c = *fmt) == '+' || c == '-' || c == ' ' || c == '0') {
                if (c == '+' || c == ' ' || c == '-') {
                    if (havePlus || haveMinus || haveSpace) {
                        fprintf(stderr, "invalid C-style printf format (have multiple +/-/' ') %s\n", ofmt);
                        kexit(1);
                    }
                    if (c == '+') havePlus = true;
                    if (c == ' ') haveSpace = true;
                    if (c == '-') haveMinus = true;
                } else if (c == '0') {
                    if (haveZero) {
                        fprintf(stderr, "invalid C-style printf format ('0' given multiple times): %s \n", ofmt);
                        kexit(1);
                    }
                    haveZero = true;
                }
                fmt++;
            }

            // get width
            // <width>
            if (*fmt == '*') {
                // -2 means to read in the argument
                width = -2;
                fmt++;
            } else if ('0' <= (c = *fmt) && c <= '9') {
                width = 0;
                while ((c = *fmt) && ('0' <= c && c <= '9')) {
                    width = 10 * width + (c - '0');
                    fmt++;
                }
            }

            // get prec
            // .<prec>
            if ((c = *fmt) == '.') {
                fmt++;
                if ((c = *fmt) == '*') {
                    // -2 means to read in the argument
                    prec = -2;
                    fmt++;
                } else if ('0' <= (c = *fmt) && c <= '9') {
                    prec = 0;
                    while ((c = *fmt) && ('0' <= c && c <= '9')) {
                        prec = 10 * prec + (c - '0');
                        fmt++;
                    }
                }
            }
            
            if ((c = *fmt++) == '%') {
                // literal '%' character
                sz = kio_bufio_write(&bio, 1, (const u8*)"%");
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 'u') {
                u64 val = va_arg(args, u64);
                sz = kio_bufio_writeu64(&bio, val, 10, width);
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 's') {
                s64 val = va_arg(args, s64);
                sz = kio_bufio_writes64(&bio, val, 10, width);
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 'f') {
                f64 val = va_arg(args, f64);
                sz = kio_bufio_writef64(&bio, val, 10, width, prec);
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 'O') {
                kobj val = va_arg(args, void*);
                ktype tp = KOBJ_TYPE(val);

                sz = kio_bufio_write(&bio, 1, "<");
                if (sz < 0) return sz;
                rsz += sz;
                
                sz = kio_bufio_write(&bio, tp->name->lenb, tp->name->data);
                if (sz < 0) return sz;
                rsz += sz;

                sz = kio_bufio_write(&bio, 5, " @ 0x");
                if (sz < 0) return sz;
                rsz += sz;

                sz = kio_bufio_writeu64(&bio, (u64)val, 16, -1);
                if (sz < 0) return sz;
                rsz += sz;

                sz = kio_bufio_write(&bio, 1, ">");
                if (sz < 0) return sz;
                rsz += sz;

            } else if (c == 'B') {
                kobj val = va_arg(args, void*);

                sz = kio_bufio_writeb(&bio, val);
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 'S') {
                kobj val = va_arg(args, void*);
                sz = kio_bufio_writes(&bio, val);
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 'R') {
                kobj val = va_arg(args, void*);

                sz = kio_bufio_writer(&bio, val);
                if (sz < 0) return sz;
                rsz += sz;
            } else {
                fprintf(stderr, "invalid C-style format string: %s\n", ofmt);
                kexit(1);
            }
        }
    }

    // flush io
    kio_bufio_done(&bio);
    return rsz;
}
