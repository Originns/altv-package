#include <stddef.h>
#include <ctype.h>

#include <hashing.h>

// https://github.com/jwerle/murmurhash.c/blob/master/murmurhash.c
uint32_t package_murmurhash3(const char *key, uint32_t len)
{
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m = 5;
    uint32_t n = 0xe6546b64;
    uint32_t h = 0;
    uint32_t k = 0;
    uint8_t *d = (uint8_t *)key;
    const uint32_t *chunks = NULL;
    const uint8_t *tail = NULL;
    int i = 0;
    int l = len / 4;

    chunks = (const uint32_t *)(d + l * 4);
    tail = (const uint8_t *)(d + l * 4);

    for (i = -l; i != 0; ++i)
    {
        k = chunks[i];

        // convert to lower case
        k = tolower(k & 0xFF) |
            (tolower((k >> 8) & 0xFF) << 8) |
            (tolower((k >> 16) & 0xFF) << 16) |
            (tolower((k >> 24) & 0xFF) << 24);

        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        h ^= k;
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
    }

    k = 0;

    switch (len & 3)
    {
    case 3:
        k ^= (tail[2] << 16);
    case 2:
        k ^= (tail[1] << 8);
    case 1:
        k ^= tail[0];
        k = tolower(k & 0xFF) |
            (tolower((k >> 8) & 0xFF) << 8) |
            (tolower((k >> 16) & 0xFF) << 16);
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
        h ^= k;
    }

    h ^= len;
    h ^= (h >> 16);
    h *= 0x85ebca6b;
    h ^= (h >> 13);
    h *= 0xc2b2ae35;
    h ^= (h >> 16);

    return h;
}

uint32_t package_joaat(const char *key, uint32_t len)
{
    uint32_t hash;
    uint32_t i;

    for (i = 0, hash = 0; i < len; ++i)
    {
        hash += tolower(key[i]);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    if (hash < 2)
    {
        hash += 2;
    }

    return hash;
}

altv_hash_t altv_hash(const char *key, uint32_t len)
{
    return (altv_hash_t){
        .joaat = package_joaat(key, len),
        .murmur3 = package_murmurhash3(key, len)};
}
