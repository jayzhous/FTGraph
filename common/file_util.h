#ifndef _NTGRAPH_COMMON_FILE_UTIL_H_
#define _NTGRAPH_COMMON_FILE_UTIL_H_

#include <fstream>
#include <iostream>

#include <unistd.h>
#include <stdlib.h>

static void append_file(const std::string file, const std::string content) {
    std::ofstream ofs;
    ofs.open(file, std::ios::out | std::ios::app);
    if (!ofs.is_open()) {
        std::cout << "Error: 文件创建或打开失败!" << std::endl;
        exit(1);
    }
    ofs << content << std::endl;
    ofs.close();
}

static void write_file(const std::string file, const std::string content) {
    std::ofstream ofs;
    ofs.open(file, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        std::cout << "Error: 文件创建或打开失败!" << std::endl;
        exit(1);
    }
    ofs << content << std::endl;
    ofs.close();
}

static inline int file_exists(const char* file) {
    return access(file, F_OK);
}

#endif // _NTGRAPH_COMMON_WRITE_FILE_H_