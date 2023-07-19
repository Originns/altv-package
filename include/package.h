#ifndef PACKAGE_H
#define PACKAGE_H
#include <hashing.h>

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

package_file_t *package_create_file(package_t *package, const char *file_name);
package_file_t *package_open_file_idx(package_t *package, uint32_t index);
package_file_t *package_open_file_hash(package_t *package, altv_hash_t hash);
package_file_t *package_open_file_cstr(package_t *package, const char *file_name);

uint32_t package_file_seek(package_file_t *entry, uint32_t offset, int origin);
uint32_t package_file_read(package_file_t *entry, uint8_t *buffer, uint32_t size);
uint32_t package_file_write(package_file_t *entry, uint8_t *buffer, uint32_t size);
uint32_t package_file_tell(package_file_t *file);
uint32_t package_file_truncate(package_file_t *file, uint32_t size);
uint32_t package_file_size(package_file_t *file);
altv_hash_t package_file_hash(package_file_t *file);
const uint8_t *package_file_data(package_file_t *file);

#endif // PACKAGE_H
