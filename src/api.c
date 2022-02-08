/* src/api.c - implementation of the Kata C API functions
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/impl.h>


// digit string for integer to string conversions
static const char* digits = "0123456789ABCDEF";


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
    Kfloat->sz = sizeof(struct kfloat);
    Ktuple->sz = sizeof(struct ktuple);

    Klist->sz = sizeof(struct klist);

    Ksys_rawio->sz = sizeof(struct ksys_rawio);


    bf_context_init(&Kbf_ctx, Kbf_ctx_realloc_, NULL);

    kinit_mem();
    kinit_sys();
    kinit_ks();

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
kobj_make(ktype tp) {
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
    kobj res = NULL;
    if (tp == Kfunc) {
        kfunc kfn = (kfunc)fn;
        if (kfn->kind & KFUNC_CFUNC) {
            res = kfn->cfunc_(kfn, nargs, args);
        } else {
            kexit(-1);
            return NULL;
        }
    }

    return res;
}

KATA_API kobj
kqcall(kobj fn, usize nargs, kobj* args) {
    return kcall(fn, nargs, args);
}


KATA_API ssize
kread(kobj io, usize len, void* data) {
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
kwrite(kobj io, usize len, const void* data) {
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
        kexit(-1);
        return -1;
    }
}

KATA_API ssize
kwriteu(kobj io, u64 val, s8 base, s32 width) {
    assert(base >= 2);
    assert(base <= 16);

    // temporary buffer we populate up to 'i'
    u8 tmp[100];
    ssize i = 0;

    // strip out any negative sign
    bool is_neg = val < 0;
    if (is_neg) tmp[i++] = '-';

    // emit digits
    do {
        // calculate current digit
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

    // TODO: zero pad
    assert(width == 0);
    return kwrite(io, i, tmp);
}

KATA_API ssize
kwrites(kobj io, s64 val, s8 base, s32 width) {
    assert(2 <= base && base <= 16);

    // temporary buffer we populate up to 'i'
    u8 tmp[100];
    ssize i = 0;

    // strip out any negative sign
    bool is_neg = val < 0;
    if (is_neg) tmp[i++] = '-';

    // emit digits
    do {
        // calculate current digit
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

    // TODO: zero pad
    assert(width == 0);
    return kwrite(io, i, tmp);
}

KATA_API ssize
kwritef(kobj io, f64 val, s8 base, s32 width, s32 prec) {

    // temporary buffer
    char buf[100];
    // TODO: solution without libc's snprintf?
    size_t len = snprintf(buf, sizeof(buf), "%.*lf", (int)F64_DIG, (double)val);
    assert(len < sizeof(buf) - 1);

    // remove trailing zeros
    // TODO: make more efficient
    // TODO: this doesn't work with hex
    while (len > 2 && buf[len-1] == '0') {
        // inspect previous character before trailing zero
        char c = buf[len-2];

        // if is any valid digit
        if ('0' <= c && c <= '9') {
        } else if ('a' <= c && c <= 'f') {
        } else if ('A' <= c && c <= 'F') {
        } else {
            // not a valid digit, so we can't continue striping them off
            break;
        }

        len--;
    }

    assert(width == 0);
    return kwrite(io, len, buf);
}

KATA_API ssize
kwriteB(kobj io, kobj obj) {
    ktype tp = KOBJ_TYPE(obj);
    if (tp == Kstr) {
        return kwrite(io, ((kstr)obj)->lenb, ((kstr)obj)->data);
    } else {
        // TODO
        kexit(-1);
        return -1;
    }
}

KATA_API ssize
kwriteS(kobj io, kobj obj) {
    ktype tp = KOBJ_TYPE(obj);
    if (tp == Kstr) {
        return kwrite(io, ((kstr)obj)->lenb, ((kstr)obj)->data);
    } else if (tp == Kint || tp == Kfloat || tp == Ktuple || tp == Klist) {
        return kwriteR(io, obj);
    } else {
        // TODO
        kexit(-1);
        return -1;
    }
}

KATA_API ssize
kwriteR(kobj io, kobj obj) {
    ktype tp = KOBJ_TYPE(obj);
    if (tp == Kstr) {
        return kwrite(io, ((kstr)obj)->lenb, ((kstr)obj)->data);
    } else if (tp == Kint) {
        // TODO: faster ways to dump?
        size_t len;
        char* data = bf_ftoa(&len, &((kint)obj)->val, 10, 0, BF_FTOA_FORMAT_FRAC);
        ssize res = kwrite(io, len, data);
        bf_free(&Kbf_ctx, data);
        return res;
    } else if (tp == Kfloat) {
        // TODO: faster ways to dump?
        size_t len;
        // A few options for modes:
        // BF_FTOA_FORMAT_FREE: free format, which is exact
        // BF_FTOA_FORMAT_FREE_MIN: like above, but lowest number of digits
        //
        // should 'prec' be taken from the float, the current value, or infinite always?
        //
        // TODO: is 'len * bits_per_limb' correct? I assume it must be....
        s64 prec = ((kfloat)obj)->val.len * LIMB_BITS;
        char* data = bf_ftoa(&len, &((kfloat)obj)->val, 10, prec, BF_FTOA_FORMAT_FREE_MIN);
        ssize res = kwrite(io, len, data);
        bf_free(&Kbf_ctx, data);
        return res;

    } else if (tp == Klist) {
        // TODO: faster ways to dump?
        ssize rsz = 0, sz = kwrite(io, 1, "[");
        if (sz < 0) return sz;
        rsz += sz;

        // emit list children
        klist l = (klist)obj;
        usize i;
        for (i = 0; i < l->len; ++i) {
            if (i > 0) {
                sz = kwrite(io, 2, ", ");
                if (sz < 0) return sz;
                rsz += sz;
            }
        
            sz = kwriteR(io, l->data[i]);
            if (sz < 0) return sz;
            rsz += sz;
        }

        sz = kwrite(io, 1, "]");
        if (sz < 0) return sz;
        rsz += sz;
        return rsz;

    } else if (tp == Ktuple) {
        ssize rsz = 0, sz = kwrite(io, 1, "(");
        if (sz < 0) return sz;
        rsz += sz;

        // emit tuple children
        ktuple t = (ktuple)obj;
        usize i;
        for (i = 0; i < t->len; ++i) {
            if (i > 0) {
                sz = kwrite(io, 2, ", ");
                if (sz < 0) return sz;
                rsz += sz;
            }
        
            sz = kwriteR(io, t->data[i]);
            if (sz < 0) return sz;
            rsz += sz;
        }
        // to differentiate from a (...) grouping
        if (t->len == 1) {
            sz = kwrite(io, 2, ", ");
            if (sz < 0) return sz;
            rsz += sz;
        }

        sz = kwrite(io, 1, ")");
        if (sz < 0) return sz;
        rsz += sz;
        return rsz;

    } else {
        // TODO
        kexit(-1);
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

KATA_API ssize
kprintfv(kobj io, const char* fmt, va_list args) {
    const char* ofmt = fmt;
    char c;

    // total size and temp var
    ssize rsz = 0, sz;

    while (*fmt) {
        // capture current format start
        const char* cfmt = fmt;

        // skip all non-control character
        while (*fmt && *fmt != '%') fmt++;

        // attempt to write to buffered io
        sz = kwrite(io, (usize)(fmt - cfmt), cfmt);
        if (sz < 0) return sz;
        rsz += sz;

        if (*fmt == '%') {
            // have a format specifier
            fmt++;
            // width and precision
            s32 width = 0, prec = 0;
            bool havePlus = false, haveMinus = false, haveSpace = false, haveZero = false;

            while ((c = *fmt) == '+' || c == '-' || c == ' ' || c == '0') {
                if (c == '+' || c == ' ' || c == '-') {
                    if (havePlus || haveMinus || haveSpace) {
                        fprintf(stderr, "\nERROR: invalid C-style printf format (have multiple +/-/' ') %s\n", ofmt);
                        kexit(1);
                    }
                    if (c == '+') havePlus = true;
                    if (c == ' ') haveSpace = true;
                    if (c == '-') haveMinus = true;
                } else if (c == '0') {
                    if (haveZero) {
                        fprintf(stderr, "\nERROR: invalid C-style printf format ('0' given multiple times): %s \n", ofmt);
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
                sz = kwrite(io, 1, "%");
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 'u') {
                u64 val = va_arg(args, u64);
                sz = kwriteu(io, val, 10, width);
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 's') {
                s64 val = va_arg(args, s64);
                sz = kwrites(io, val, 10, width);
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 'f') {
                f64 val = va_arg(args, f64);
                sz = kwritef(io, val, 10, width, prec);
                if (sz < 0) return sz;
                rsz += sz;
            } else if (c == 'p') {
                void* val = va_arg(args, void*);
                sz = kwrite(io, 2, "0x");
                if (sz < 0) return sz;
                rsz += sz;
                
                sz = kwriteu(io, (u64)val, 16, prec);
                if (sz < 0) return sz;
                rsz += sz;

            } else if (c == 'O') {
                kobj val = va_arg(args, void*);
                ktype tp = KOBJ_TYPE(val);

                sz = kwrite(io, 1, "<");
                if (sz < 0) return sz;
                rsz += sz;
                
                sz = kwrite(io, tp->name->lenb, tp->name->data);
                if (sz < 0) return sz;
                rsz += sz;

                sz = kwrite(io, 5, " @ 0x");
                if (sz < 0) return sz;
                rsz += sz;

                sz = kwriteu(io, (u64)val, 16, -1);
                if (sz < 0) return sz;
                rsz += sz;

                sz = kwrite(io, 1, ">");
                if (sz < 0) return sz;
                rsz += sz;

            } else if (c == 'B') {
                kobj val = va_arg(args, void*);

                sz = kwriteB(io, val);
                if (sz < 0) return sz;
                rsz += sz;

            } else if (c == 'S') {
                kobj val = va_arg(args, void*);

                sz = kwriteS(io, val);
                if (sz < 0) return sz;
                rsz += sz;
            
            } else if (c == 'R') {
                kobj val = va_arg(args, void*);

                sz = kwriteR(io, val);
                if (sz < 0) return sz;
                rsz += sz;

            } else {
                fprintf(stderr, "\nERROR: invalid C-style format string: %s\n", ofmt);
                kexit(1);
            }
        }
    }

    return rsz;
}
