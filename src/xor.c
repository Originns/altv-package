#include "xor.h"
#include "table.h"

file_entry_t *xor_file_entry(uint8_t *entry_data, uint32_t index, uint32_t entry_count, uint32_t file_size)
{
    uint32_t key;
    uint32_t i;

    key = entry_count * file_size;
    for (i = 0; i < sizeof(file_entry_t); ++i)
    {
        entry_data[i] ^= g_xor_table[(key ^ (i + index * sizeof(file_entry_t))) % XOR_TABLE_SIZE];
    }

    return (file_entry_t *)entry_data;
}

void xor_file_data(file_entry_t *entry, uint8_t *buffer)
{
    uint32_t key;
    uint32_t i;

    key = entry->offset * (entry->hash.joaat ^ entry->hash.murmur3);
    for (i = 0; i < entry->size; ++i)
    {
        buffer[i] ^= key ^ g_xor_table[i % XOR_TABLE_SIZE];
    }
}
