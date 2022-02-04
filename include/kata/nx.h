/* kata/nx.h - Kata's NumeriX module, which is a runtime and math library
 *
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_NX_H
#define KATA_NX_H

#ifndef KATA_API_H
  #include <kata/api.h>
#endif

////////////////////////////////////////////////////////////////////////////////

// maximum rank of a tensor
// TODO: some other libraries choose 16 or even as low as 6
#define NX_RANK_MAX    8

////////////////////////////////////////////////////////////////////////////////

// kinds of datatypes
// NOTE: some names are reserved with leading '_'. don't use these
enum {
    // void/undef type
    NXDTYPE_VOID       = 0,

    NXDTYPE_U8         = 1,
    NXDTYPE_U16        = 2,
    NXDTYPE_U32        = 3,
    NXDTYPE_U64        = 4,
    NXDTYPE_U128       = 5,
    _NXDTYPE_U256      = 6,
    _NXDTYPE_U512      = 7,
    _NXDTYPE_U1024     = 8,

    NXDTYPE_S8         = 9,
    NXDTYPE_S16        = 10,
    NXDTYPE_S32        = 11,
    NXDTYPE_S64        = 12,
    NXDTYPE_S128       = 13,
    _NXDTYPE_S256      = 14,
    _NXDTYPE_S512      = 15,
    _NXDTYPE_S1024     = 16,

    NXDTYPE_F8         = 17,
    NXDTYPE_F16        = 18,
    NXDTYPE_F32        = 19,
    NXDTYPE_F64        = 20,
    NXDTYPE_F128       = 21,
    _NXDTYPE_F256      = 22,
    _NXDTYPE_F512      = 23,
    _NXDTYPE_F1024     = 24,

    // structure types
    NXDTYPE_STRUCT     = 101,
    NXDTYPE_UNION      = 102,

    // reference types
    NXDTYPE_PTR        = 201,

};


// datatype structure, which describes a particular kind of data
typedef struct nx_dtype {

    // the printable/referable name of the datatype
    kstr name;

    // the kind of datatype, which can be a builtin format/datatype
    // NOTE: see 'NXDTYPE_*'
    s32 kind;

    // the size of the datatype in bytes
    // TODO: support variable sizes with negatives?
    ssize sz;

}* nx_dtype;


// environment object, which is basically an execution context but it can be
//   on other hardware, such as a GPU/TPU/accelerator
// TODO: FPGA support?
typedef struct nx_env {
    
    // the printable/referable name of the environment
    kstr name;



}* nx_env;

typedef struct nx_arena {

}* nx_arena;


// NumeriX tensor handle
typedef struct nx_tensor {

    // the raw data pointer, which is the location of the 0th element in all dimensions
    u8* data;

    // the rank (i.e. number of dimensions) of the tensor
    u8 rank;

    // the shape of the tensor, which is the number of elements in each dimension
    // for example, a tensor with rank 2 and shape (3, 4) has 12 elements
    usize shape[NX_RANK_MAX];

    // the stride of the tensor, which is the number of elements between each element in a given dimension
    ssize stride[NX_RANK_MAX];

}* nx_tensor;


////////////////////////////////////////////////////////////////////////////////


// global types
KATA_API ktype
Kio_bufio
;


#endif // KATA_IO_H
