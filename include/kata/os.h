/* kata/os.h - Kata's operating system module
 *
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_OS_H
#define KATA_OS_H


// OS's raw IO interface, through C-style libraries only currently
typedef struct kos_rawio {

    // for systems with a file descriptor
    s32 fd_;

}* kos_rawio;

// make a new rawio with a file descriptor
KATA_API kos_rawio
kos_rawio_newd(s32 fd_);


// kos_open() flags
// TODO: https://linux.die.net/man/3/open
enum {
    // no flags, which is an invalid combination
    KOS_OPEN_NONE      = 0x00,

    // open for reading
    KOS_OPEN_R         = 0x01,
    // open for writing
    KOS_OPEN_W         = 0x02,

    // open, appending to the end of the file
    KOS_OPEN_A         = 0x10,
    // open, creating if not exist
    KOS_OPEN_C         = 0x20,
    // open, exclusive mode (which fails if the file already exists)
    KOS_OPEN_E         = 0x40,

};

// open for reading and writing
#define KOS_OPEN_RW (KOS_OPEN_R | KOS_OPEN_W)

// open a file/resource for reading/writing
// TODO: default mode, or allow it to be passed in? or just have a different function
// NOTE: see 'KOS_OPEN_*' for 'flags'
KATA_API kobj
kos_open(const char* path, u32 flags);

// os module globals
KATA_API kos_rawio
Kos_stdout,
Kos_stderr,
Kos_stdin
;

KATA_API ktype
Kos_rawio
;

#endif // KATA_OS_H
