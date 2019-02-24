#ifndef MKPATH_H
#define MKPATH_H

#include <sys/stat.h>
#ifdef _MSC_VER
using mode_t = int;
#endif // WIN32

#include <utility>
#include <string>

int mkpath(const char *path, mode_t mode=0777);

enum FileType {
    DOES_NOT_EXIST,
    DIRECTORY,
    REGULAR_FILE,
    OTHER,
    ERROR
};

FileType get_file_type(const char* path);

std::pair<std::string, std::string> dir_and_base_name(const char* name);

#endif // MKPATH_H