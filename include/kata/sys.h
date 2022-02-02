/* kata/sys.h - Kata's system interface (replacing the old 'os' library)
 *
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_SYS_H
#define KATA_SYS_H


// system's raw IO interface, through C-style libraries only currently
typedef struct ksys_rawio {

    // for systems with a file descriptor
    s32 fd_;

}* ksys_rawio;

// make a new rawio with a file descriptor
KATA_API ksys_rawio
ksys_rawio_newd(s32 fd_);


// ksys_open() flags
// TODO: https://linux.die.net/man/3/open
enum {
    // no flags, which is an invalid combination
    KSYS_OPEN_NONE   = 0x00,

    // open for reading
    KSYS_OPEN_R      = 0x01,
    // open for writing
    KSYS_OPEN_W      = 0x02,

    // open, appending to the end of the file
    KSYS_OPEN_A      = 0x10,
    // open, creating if not exist
    KSYS_OPEN_C      = 0x20,
    // open, exclusive mode (which fails if the file already exists)
    KSYS_OPEN_E      = 0x40,

};

// open for reading and writing
#define KSYS_OPEN_RW (KSYS_OPEN_R | KSYS_OPEN_W)

// open a file/resource for reading/writing
// TODO: default mode, or allow it to be passed in? or just have a different function
// NOTE: see 'KSYS_OPEN_*' for 'flags'
KATA_API kobj
ksys_open(const char* path, u32 flags);

// sys module globals
KATA_API ksys_rawio
Ksys_stdout,
Ksys_stderr,
Ksys_stdin
;

KATA_API ktype
Ksys_rawio
;

#endif // KATA_SYS_H
