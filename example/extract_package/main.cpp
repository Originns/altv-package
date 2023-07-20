#include <iostream>
#include <string>
#include <filesystem>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <map>

extern "C" 
{ 
#include <package.h> 
}

namespace fs = std::filesystem;

bool extract_package(fs::path input_file, fs::path output_dir)
{
    package_t package;
    if (package_open(&package, input_file.string().c_str()))
    {
        std::cerr << "Error: Failed to open " << input_file.filename() << '\n';
        return false;
    }

    std::string package_name = input_file.filename().string();
    // strip the extension
    package_name = package_name.substr(0, package_name.find_last_of('.'));

    auto package_dir = output_dir / package_name;

    // create the package directory
    if (!fs::exists(package_dir))
    {
        fs::create_directories(package_dir);
    }

    for (uint32_t i = 0; i < package.num_entries; i++)
    {
        package_file_t *entry = package_open_file_idx(&package, i);

        // file name is the entry hash as a hex string
        std::stringstream ss;
        ss << std::hex << package_file_hash(entry).value << ".bin";
        std::string file_name = ss.str();

        // create the file
        std::ofstream file(package_dir / file_name, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Error: Failed to create " << file_name << '\n';
            package_close(&package);
            return false;
        }

        // write the file data
        file.write((const char *)package_file_data(entry), package_file_size(entry));
        file.close();
    }

    package_close(&package);

    return true;
}

int main(int argc, char **argv)
{
    std::map<std::string, std::string> args;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if ((arg == "-i") || (arg == "-o"))
        {
            if (i + 1 < argc)
            {
                args[arg] = argv[++i];
            }
            else
            {
                std::cerr << "Error: " << arg << " requires a value.\n";
                return 1;
            }
        }
        else
        {
            std::cerr << "Error: Unknown option " << arg << '\n';
            return 1;
        }
    }

    if (args.find("-i") == args.end() || args.find("-o") == args.end())
    {
        std::cerr << "Usage: " << argv[0] << " -i <input_file/dir> -o <output_dir>\n";
        return 1;
    }

    // if the output directory does not exist, create it
    fs::path input_dir = args["-i"];
    fs::path output_dir = args["-o"];

    if (!fs::exists(output_dir))
    {
        fs::create_directories(output_dir);
    }

    if (fs::is_directory(input_dir))
    {
        for (auto &p : fs::directory_iterator(input_dir))
        {
            if (p.path().extension() == ".resource")
            {
                std::cout << "Extracting " << p.path().filename() << "...\n";

                extract_package(p.path(), output_dir);
            }
        }
    }
    else if (fs::is_regular_file(input_dir))
    {
        std::cout << "Extracting " << input_dir.filename() << "...\n";
        extract_package(input_dir, output_dir);
    }
    else
    {
        std::cerr << "Error: " << input_dir << " is not a valid file or directory.\n";
        return 1;
    }

    std::cout << "Done.\n";

    return 0;
}
