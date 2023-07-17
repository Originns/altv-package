#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <package.h>

#include "xor.h"

struct package_file_t
{
    file_entry_t entry;
    uint32_t pos;
    uint8_t *data;
};

uint32_t package_open(package_t *package, const char *package_path)
{
    uint32_t magic;
    uint32_t entry_table_size;
    uint8_t *file_data;
    uint32_t data_offset;
    uint32_t file_size;
    FILE *package_file;

    if (package == NULL)
    {
        return 1;
    }

    // init the package
    memset(package, 0, sizeof(package_t));

    // open the file
    package_file = fopen(package_path, "rb");
    if (package_file == NULL)
    {
        // File not found
        printf("File not found\n");
        return 1;
    }

    // get the file size
    fseek(package_file, 0, SEEK_END);
    file_size = ftell(package_file);
    fseek(package_file, 0, SEEK_SET);

    // read the magic
    if (fread(&magic, 4, 1, package_file) != 1 || magic != PACKAGE_MAGIC)
    {
        // Not a package file
        // printf("Not a package file\n");
        package_close(package);
        return 1;
    }

    // read the entry count
    if (fseek(package_file, file_size - 4, SEEK_SET))
    {
        // Package file corrupted
        // printf("Package file corrupted #1\n");
        package_close(package);
        return 1;
    }

    if (fread(&package->num_entries, 4, 1, package_file) != 1)
    {
        // Package file corrupted
        // printf("Package file corrupted #2\n");
        package_close(package);
        return 1;
    }

    // read the entry table
    entry_table_size = sizeof(file_entry_t) * package->num_entries;
    data_offset = file_size - entry_table_size - 4;
    if (fseek(package_file, data_offset, SEEK_SET))
    {
        // Package file corrupted
        // printf("Package file corrupted #3\n");
        package_close(package);
        return 1;
    }

    package->entries = (package_file_t *)malloc(sizeof(package_file_t) * package->num_entries);
    if (package->entries == NULL)
    {
        // Out of memory
        // printf("Out of memory\n");
        package_close(package);
        return 1;
    }

    for (uint32_t i = 0; i != package->num_entries; ++i)
    {
        if (fread(&package->entries[i].entry, 0x10, 1, package_file) != 1)
        {
            // Package file corrupted
            // printf("Package file corrupted #4\n");
            package_close(package);
            return 1;
        }

        xor_file_entry((uint8_t *)&package->entries[i].entry, i, package->num_entries, file_size);

        package->entries[i].data = NULL;
    }
    // validate the entries
    for (uint32_t i = 0; i != package->num_entries; ++i)
    {
        if (package->entries[i].entry.offset + package->entries[i].entry.size > data_offset)
        {
            // Package file corrupted
            // printf("Package file corrupted #5\n");
            package_close(package);
            return 1;
        }

        file_data = (uint8_t *)malloc(package->entries[i].entry.size);
        if (file_data == NULL)
        {
            // Out of memory
            // printf("Out of memory #1\n");
            package_close(package);
            return 1;
        }

        // read the file data
        if (fseek(package_file, package->entries[i].entry.offset, SEEK_SET))
        {
            // Package file corrupted
            // printf("Package file corrupted #6\n");
            package_close(package);
            return 1;
        }

        if (fread(file_data, 1, package->entries[i].entry.size, package_file) != package->entries[i].entry.size)
        {
            // Package file corrupted
            // printf("Package file corrupted #7\n");
            package_close(package);
            return 1;
        }

        xor_file_data(&package->entries[i].entry, file_data);

        package->entries[i].data = file_data;
    }

    return 0;
}

uint32_t package_create(package_t *package)
{
    // create a new package
    memset(package, 0, sizeof(package_t));

    // init the package package
    package->num_entries = 0;
    package->entries = NULL;

    return 0;
}

// save a package to disk
uint32_t package_save(package_t *package, const char *path)
{
    uint32_t entry_table_size;
    uint32_t data_offset;
    uint32_t file_size;
    FILE *package_file;

    // open the file
    package_file = fopen(path, "wb");
    if (package_file == NULL)
    {
        // File not found
        return 1;
    }

    // calculate the file size
    entry_table_size = sizeof(file_entry_t) * package->num_entries;

    // calculate the data offset
    data_offset = 4;
    for (uint32_t i = 0; i != package->num_entries; ++i)
    {
        data_offset += package->entries[i].entry.size;
    }

    // calculate the file size
    file_size = data_offset + entry_table_size + 4;

    // write the magic
    uint32_t magic = PACKAGE_MAGIC;
    if (fwrite(&magic, 4, 1, package_file) != 1)
    {
        // Failed to write to file
        return 1;
    }

    // write the file data
    for (uint32_t i = 0; i != package->num_entries; ++i)
    {
        package_file_t *file = &package->entries[i];
        file->entry.offset = ftell(package_file);
        if (file->data)
        {
            xor_file_data(&file->entry, file->data);
            if (fwrite(file->data, 1, file->entry.size, package_file) != file->entry.size)
            {
                // Failed to write to file
                return 1;
            }
            xor_file_data(&file->entry, file->data);
        }
    }

    // write the entry table
    for (uint32_t i = 0; i != package->num_entries; ++i)
    {
        package_file_t *file = &package->entries[i];
        xor_file_entry((uint8_t *)&file->entry, i, package->num_entries, file_size);
        if (fwrite(&file->entry, 0x10, 1, package_file) != 1)
        {
            // Failed to write to file
            return 1;
        }

        xor_file_entry((uint8_t *)&file->entry, i, package->num_entries, file_size);
    }

    // write the entry count
    if (fwrite(&package->num_entries, 4, 1, package_file) != 1)
    {
        // Failed to write to file
        return 1;
    }

    fclose(package_file);

    return 0;
}

void package_close(package_t *package)
{

    if (package->entries)
    {
        for (uint32_t i = 0; i != package->num_entries; ++i)
        {
            if (package->entries[i].data)
                free(package->entries[i].data);
        }

        free(package->entries);
    }
}

package_file_t *package_get_file(package_t *package, uint32_t index)
{
    if (index >= package->num_entries)
    {
        return NULL;
    }

    return &package->entries[index];
}

uint32_t package_open_file_hash(package_t *package, package_file_t **entry, altv_hash_t hash)
{
    if (package->num_entries == 0)
    {
        return 1;
    }

    for (uint32_t i = 0; i < package->num_entries; ++i)
    {
        if (package->entries[i].entry.hash.value == hash.value)
        {
            *entry = &package->entries[i];
            return 0;
        }
    }

    return 1;
}

uint32_t package_open_file_cstr(package_t *package, package_file_t **entry, const char *file_name)
{
    return package_open_file_hash(package, entry, altv_hash(file_name, strlen(file_name)));
}

// functions to add or remove files from the package
package_file_t *package_add_file(package_t *package, const char *file_name)
{
    // if there are no entries, allocate the entries
    if (package->entries == NULL)
    {
        package->entries = (package_file_t *)malloc(sizeof(package_file_t));
        if (package->entries == NULL)
        {
            return 0;
        }
    }
    else
    {
        // realloc the entries
        package_file_t *new_entries = (package_file_t *)realloc(package->entries, sizeof(package_file_t) * (package->num_entries + 1));
        if (new_entries == NULL)
        {
            return 0;
        }

        package->entries = new_entries;
    }

    // set the new entry
    package_file_t *entry = &package->entries[package->num_entries];

    entry->entry.hash = altv_hash(file_name, strlen(file_name));

    // increase the entry count
    ++package->num_entries;

    // return the entry
    return entry;
}

uint32_t package_file_seek(package_file_t *entry, uint32_t offset, seekorigin_t origin)
{
    switch (origin)
    {
    case PKG_SEEK_SET:
        entry->pos = offset;
        break;
    case PKG_SEEK_CUR:
        entry->pos += offset;
        break;
    case PKG_SEEK_END:
        entry->pos = entry->entry.size - offset;
        break;
    default:
        return 1;
    }

    return 0;
}

uint32_t package_file_read(package_file_t *entry, uint8_t *buffer, uint32_t size)
{
    // the file is smaller than the requested size
    if (entry->pos + size > entry->entry.size)
    {
        if (entry->pos >= entry->entry.size)
        {
            return 0;
        }

        size = entry->entry.size - entry->pos;
    }

    memcpy(buffer, &entry->data[entry->pos], size);

    entry->pos += size;

    return size;
}

uint32_t package_file_write(package_file_t *entry, uint8_t *buffer, uint32_t size)
{
    if (entry->pos + size > entry->entry.size)
    {
        // if there is no data, allocate the data
        if (entry->data == NULL)
        {
            entry->data = (uint8_t *)malloc(entry->pos + size);
            if (entry->data == NULL)
            {
                return 0;
            }
        }
        else
        {
            // realloc the data
            uint8_t *new_data = (uint8_t *)realloc(entry->data, entry->pos + size);
            if (new_data == NULL)
            {
                return 0;
            }

            entry->data = new_data;
        }

        entry->entry.size = entry->pos + size;
    }

    memcpy(&entry->data[entry->pos], buffer, size);

    entry->pos += size;

    return size;
}

uint32_t package_file_tell(package_file_t *file)
{
    return file->pos;
}

uint32_t package_file_truncate(package_file_t *file, uint32_t size)
{
    if (size > file->entry.size)
    {
        return 0;
    }

    file->entry.size = size;

    // only realloc if the size is less than 1/4 of the current size
    if (size < file->entry.size / 4)
    {
        uint8_t *new_data = (uint8_t *)realloc(file->data, size);
        if (new_data == NULL)
        {
            return 0;
        }

        file->data = new_data;
    }

    return size;
}

uint32_t package_file_size(package_file_t *file)
{
    return file->entry.size;
}

altv_hash_t package_file_hash(package_file_t *file)
{
    return file->entry.hash;
}

const uint8_t *package_file_data(package_file_t *file)
{
    return file->data;
}
