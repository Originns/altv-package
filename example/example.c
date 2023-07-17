#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#define mkdir(dir, mode) _mkdir(dir)
#define stat(path, buf) _stat(path, buf)
#define S_ISDIR(mode) (mode & _S_IFDIR)
struct _stat;
#else
#include <sys/stat.h>
#endif

#include <package.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    package_t pkg;
    struct stat info;
    package_file_t *file;

    if (package_create(&pkg))
    {
        printf("Failed to create package\n");
        return 1;
    }

    // add a file
    file = package_add_file(&pkg, "example.txt");
    if (file == NULL)
    {
        printf("Failed to add file\n");
        return 1;
    }

    // add data
    const char data[] = "i have no idea what i'm doing";
    if (package_file_write(file, (uint8_t *)data, strlen(data)) != strlen(data))
    {
        printf("Failed to write data\n");
        return 1;
    }

    // add another file
    file = package_add_file(&pkg, "client/index.js");
    if (file == NULL)
    {
        printf("Failed to add file\n");
        return 1;
    }

    // add data
    const char data2[] = "console.log('yup yep');";
    if (package_file_write(file, (uint8_t *)data2, strlen(data2)) != strlen(data2))
    {
        printf("Failed to write data\n");
        return 1;
    }

    // save the package
    if (package_save(&pkg, "example.resource"))
    {
        printf("Failed to save package\n");
        return 1;
    }

    // close the package
    package_close(&pkg);

    // open the package
    if (package_open(&pkg, "example.resource"))
    {
        printf("Failed to open package\n");
        return 1;
    }

    // create a dump dir
    if (stat("dump", &info) != 0 || !S_ISDIR(info.st_mode))
    {
        if (mkdir("dump", 0777))
        {
            printf("Failed to create dump directory\n");
            return 1;
        }
    }

    // extract the files
    for (uint32_t i = 0; i < pkg.num_entries; ++i)
    {
        file = package_get_file(&pkg, i);

        uint32_t size = package_file_size(file);
        altv_hash_t hash = package_file_hash(file);
        const uint8_t *data = package_file_data(file);

        // print file information
        printf("File %llx size: %u\n", hash.value, size);

        char file_name[256];
        sprintf(file_name, "dump/%llx.bin", hash.value);

        // open the file
        FILE *rfile = fopen(file_name, "wb");
        if (rfile == NULL)
        {
            printf("Failed to open file %s\n", file_name);
            return 1;
        }

        // write the data to the file
        if (fwrite(data, 1, size, rfile) != size)
        {
            printf("Failed to write file %s\n", file_name);
            return 1;
        }

        fclose(rfile);
    }

    // close the package
    package_close(&pkg);

    return 0;
}
