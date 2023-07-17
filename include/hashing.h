#ifndef HASHING_H
#define HASHING_H
#include <stddef.h>
#include <stdint.h>

typedef union
{
    uint64_t value;
    struct
    {
        uint32_t joaat;
        uint32_t murmur3;
    };
} altv_hash_t;

altv_hash_t altv_hash(const char *key, uint32_t len);

#endif // HASHING_H
