#ifndef PACKAGE_H
#define PACKAGE_H
#include <hashing.h>

#define PACKAGE_MAGIC 0x544c41 // "ALT"

// enum for seekorigin
typedef enum
{
    PKG_SEEK_SET,
    PKG_SEEK_CUR,
    PKG_SEEK_END
} seekorigin_t;

typedef struct package_file_t package_file_t;

typedef struct
{
    uint32_t num_entries;
    package_file_t **entries;
} package_t;

uint32_t package_open(package_t *package, const char *package_path);
uint32_t package_create(package_t *package);
uint32_t package_save(package_t *package, const char *path);
void package_close(package_t *package);
uint32_t package_open_file_hash(package_t *package, package_file_t **entry, altv_hash_t hash);
uint32_t package_open_file_cstr(package_t *package, package_file_t **entry, const char *file_name);
package_file_t *package_add_file(package_t *package, const char *file_name);
package_file_t *package_get_file(package_t *package, uint32_t index);

uint32_t package_file_seek(package_file_t *entry, uint32_t offset, seekorigin_t origin);
uint32_t package_file_read(package_file_t *entry, uint8_t *buffer, uint32_t size);
uint32_t package_file_write(package_file_t *entry, uint8_t *buffer, uint32_t size);
uint32_t package_file_tell(package_file_t *file);
uint32_t package_file_truncate(package_file_t *file, uint32_t size);
uint32_t package_file_size(package_file_t *file);
altv_hash_t package_file_hash(package_file_t *file);
const uint8_t *package_file_data(package_file_t *file);

#endif // PACKAGE_H
