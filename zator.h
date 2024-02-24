#ifndef __ZATOR_H__
#define __ZATOR_H__

#ifndef __KERNEL__
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#else
#include <linux/string.h>
#include <linux/types.h>
#define assert(x) //WARN((x))
#endif

//required for pointer arithmetics
typedef long long z_ptr_t;
typedef unsigned long long zu_ptr_t;

_Static_assert(sizeof(void*) == sizeof(z_ptr_t), "z_ptr_t size is not the size of void*");
_Static_assert(sizeof(void*) == sizeof(zu_ptr_t), "z_ptr_t size is not the size of void*");

static inline void z_serialize_ptr(void* data, void** srcptr) 
{
#if defined(DEBUG)
    assert(data && srcptr);
    assert(*srcptr);
    assert((zu_ptr_t) data > (zu_ptr_t) srcptr); // data lies past-struct
#endif
    *srcptr = (void*) ((z_ptr_t) data - (z_ptr_t) srcptr);
}

static inline void z_deserialize_ptr(void** srcptr) 
{
#if defined(DEBUG)
    assert(srcptr);
    assert(*srcptr);
#endif
    *srcptr =  (void*) ((z_ptr_t) srcptr + (z_ptr_t) *srcptr);
}

// Assuming that src will be serialized pointer 
static inline void* z_serialize_copy(void* dst, void** src, unsigned long length)
{
#if defined(DEBUG)
    assert(dst && src);
    assert(*src);
#endif

    memcpy(dst, *src, length);
    z_serialize_ptr(dst, src);

    return (void*) (((zu_ptr_t) dst) + length);
}

//source is one-dimensional pointer to source structure
//destination will be updated appropriately
#define ZSERIALIZE(destination, source, length)                                                 \
    do {                                                                                        \
        if (source && length) {                                                                 \
            memcpy(destination, source, length);                                                \
            destination += length;                                                              \
        }

#define Z_SERIALIZE_PTR(destination, source, length)                                            \
    ({                                                                                          \
        if (*source && length) {                                                                \
            destination = z_serialize_copy((void*) destination, (void**) source, length);       \
        }                                                                                       \
    })

#define Z_SERIALIZE_STR(destination, source)                                                    \
    ({                                                                                          \
        if (*source)                                                                            \
            Z_SERIALIZE_PTR(destination, source, strlen(*(source)) + 1);                        \
    })

#define ZEZILAIRES()                                                                            \
    } while(0)

#define Z_DESERIALIZE(source)                                                                   \
    ({                                                                                          \
        if (source && *source)                                                                  \
            z_deserialize_ptr((void**) source);                                                 \
    })

///////////////////////////////////////////////////////////////
// EASIER API FOR STRUCTURES AND SEPARATE BUFFERS
///////////////////////////////////////////////////////////////

//source is one-dimensional pointer, totallen should be set to 0
#define Z_SERIALIZE_STRUCT(destination, source, total_len)                                      \
    _Z_SERIALIZE_STRUCT(destination, source, 0, total_len)

#define Z_SERIALIZE_STRUCT_DRY(destination, source, total_len)                                  \
    _Z_SERIALIZE_STRUCT(destination, source, 1, total_len)

// tmp_source - serialization context, copy of source pointer
// len - pointer to total_len, which will recieve final length of the serialized structure
// dry - if true, only length will be updated. memory in destination is not touched
// tmp_dest - pointer to the destination address
#define _Z_SERIALIZE_STRUCT(destination, source, dryrun, total_len)                             \
    ({                                                                                          \
        typeof(source) __tmp_source = dryrun ? source : (typeof(source)) destination;           \
        typeof(total_len)* __len = &(total_len);                                                \
        int __dry = (int) dryrun;                                                               \
        char* __tmp_dest = (char*) (destination);                                               \
        *__len += sizeof(*(source));                                                            \
        ZSERIALIZE(__tmp_dest, source, __dry ? 0 : sizeof(*source))

#define Z_SERIALIZE_STRUCT_PTR_FIELD(field, length)                                             \
    ({                                                                                          \
        *__len += length;                                                                       \
        Z_SERIALIZE_PTR(__tmp_dest, &__tmp_source->field, __dry ? 0 : length);                  \
    })

#define Z_SERIALIZE_STRUCT_STR_FIELD(field)                                                     \
    ({                                                                                          \
        Z_SERIALIZE_STRUCT_PTR_FIELD(field,                                                     \
            __tmp_source->field ? strlen(__tmp_source->field) + 1 : 0);                         \
    })

#define Z_SERIALIZE_STRUCT_END()                                                                \
        ZEZILAIRES();                                                                           \
        __tmp_source;                                                                           \
    })

// field is one-dimensional pointer in a struct
#define Z_SERIALIZE_STRUCT_NESTED(field)                                                        \
    _Z_SERIALIZE_STRUCT_NESTED(field, typeof(*field))

// if there are more than one nested structure, perhaps, __nested_xxx_##level can help ?
#define _Z_SERIALIZE_STRUCT_NESTED(field, type)                                                 \
    ({                                                                                          \
        if (__tmp_source->field) {                                                              \
        type** __nested_source = (type**) &__tmp_source->field;                                 \
        typeof(__tmp_dest) __nested_dest = __tmp_dest;                                          \
        typeof(__len) __nested_len = __len;                                                     \
        typeof(__dry) __nested_dry = __dry;                                                     \
        _Z_SERIALIZE_STRUCT(__nested_dest, *__nested_source, __nested_dry, *__nested_len);      \
        if (!__dry) z_serialize_ptr(__nested_dest, (void**) __nested_source)

#define Z_SERIALIZE_STRUCT_NESTED_END() Z_SERIALIZE_STRUCT_END(); } })

#define Z_DESERIALIZE_STRUCT(source)                                                            \
    ({                                                                                          \
        typeof(source) __tmp_source = source

#define Z_DESERIALIZE_STRUCT_FIELD(field)                                                       \
    if (__tmp_source && __tmp_source->field)                                                    \
        Z_DESERIALIZE(&__tmp_source->field);

#define Z_DESERIALIZE_STRUCT_END()                                                              \
    })

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////

#endif // __ZATOR_H__
