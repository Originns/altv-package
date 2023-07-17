#ifndef CRYPT_H
#define CRYPT_H
#include <hashing.h>

// struct for the file entry
typedef struct
{
    altv_hash_t hash;
    uint32_t offset;
    uint32_t size;
} file_entry_t;

file_entry_t *xor_file_entry(uint8_t *entry_data, uint32_t index, uint32_t entry_count, uint32_t file_size);
void xor_file_data(file_entry_t *entry, uint8_t *buffer);

#endif // CRYPT_H
