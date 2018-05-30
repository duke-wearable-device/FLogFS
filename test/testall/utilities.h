#ifndef __UTILITIES_H_INCLUDED
#define __UTILITIES_H_INCLUDED

#include <vector>
#include <string>
#include <cstdint>

bool initialize_and_open(bool truncate = true);

std::vector<std::string> generate_random_file_names(int32_t number);

std::vector<std::string> get_file_listing();

struct GeneratedFile {
    flog_write_file_t file;
    std::string name;
    size_t size;
    size_t written;
};

std::vector<GeneratedFile> write_files_randomly(std::vector<std::string> &names, uint8_t number_of_iterations, uint32_t min_size, uint32_t max_size);

#endif
