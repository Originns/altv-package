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
    package_t pkg;
    package_file_t *file;
    struct stat info;
    uint32_t i;
    uint32_t size;
    altv_hash_t hash;
    const uint8_t *file_data;
    char file_name[256];
    FILE *rfile;

    const char data[] = "i have no idea what i'm doing";
    const char data2[] = "console.log('yup yep');";

    (void)argc;
    (void)argv;

    if (package_create(&pkg))
    {
        printf("Failed to create package\n");
        return 1;
    }

    // create a file
    file = package_create_file(&pkg, "example.txt");
    if (file == NULL)
    {
        printf("Failed to add file\n");
        package_close(&pkg);
        return 1;
    }

    // add data
    if (package_file_write(file, (uint8_t *)data, sizeof(data) - 1) != sizeof(data) - 1)
    {
        printf("Failed to write data\n");
        package_close(&pkg);
        return 1;
    }

    // create another file
    file = package_create_file(&pkg, "client/index.js");
    if (file == NULL)
    {
        printf("Failed to add file\n");
        package_close(&pkg);
        return 1;
    }

    // add data
    if (package_file_write(file, (uint8_t *)data2, sizeof(data2) - 1) != sizeof(data2) - 1)
    {
        printf("Failed to write data\n");
        package_close(&pkg);
        return 1;
    }

    // save the package
    if (package_save(&pkg, "example.resource"))
    {
        printf("Failed to save package\n");
        package_close(&pkg);
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
            package_close(&pkg);
            return 1;
        }
    }

    // extract the files
    for (i = 0; i < pkg.num_entries; ++i)
    {
        file = package_open_file_idx(&pkg, i);
        size = package_file_size(file);
        hash = package_file_hash(file);
        file_data = package_file_data(file);

        // print file information
        printf("File %llx size: %u\n", hash.value, size);

        sprintf(file_name, "dump/%llx.bin", hash.value);

        // open the file
        rfile = fopen(file_name, "wb");
        if (rfile == NULL)
        {
            printf("Failed to open file %s\n", file_name);
            package_close(&pkg);
            return 1;
        }

        // write the data to the file
        if (fwrite(file_data, 1, size, rfile) != size)
        {
            printf("Failed to write file %s\n", file_name);
            fclose(rfile);
            package_close(&pkg);
            return 1;
        }

        fclose(rfile);
    }

    // close the package
    package_close(&pkg);

    return 0;
}
