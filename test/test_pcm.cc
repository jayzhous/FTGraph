#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>  
#include <unistd.h> 

#include "cpucounters.h"
#include "utils.h"

using namespace std;
using namespace pcm;

const char* file_name = "/mnt/pmem0/sungan/canbedeleted";
// const char* file_name = "/home/sungan/canbedeleted";
uint64_t file_size = 1024 * 1024 * 1024;


int fd;
int* arr = nullptr;

void pm_io_write_test() {
    // fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | O_DIRECT);
    fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (arr == nullptr) {
        arr = (int*) malloc(file_size);
    }
    int num_ints = file_size / 4;
    for (int i = 0; i < num_ints; i++) {
        arr[i] = i;
    }

    ssize_t bytes = write(fd, arr, file_size);
    if (bytes == -1) {
        perror("Error writing to file");
    }

    close(fd);
    free(arr);
    arr = nullptr;

}

void pm_io_read_test() {

    if (arr == nullptr) {
        arr = (int*) malloc(file_size);
    }

    // fd = open(file_name, O_RDONLY | O_DIRECT);
    fd = open(file_name, O_RDONLY);

    ssize_t bytes = read(fd, arr, file_size);
    if (bytes == -1) {
        perror("Error reading file");
        close(fd);
        free(arr);
        arr = nullptr;
        return;
    }

    int a;
    int num_ints = file_size / 4;
    for (int i = 0; i < num_ints; i++) {
        a = arr[i];
    }


    close(fd);
    free(arr);
    arr = nullptr;
}

void pm_mmap_write_test() {
    if (access(file_name, F_OK) != 0) {
        char cmd[1024];
        sprintf(cmd, "touch %s", file_name);
        if (system(cmd) == -1) {
            fprintf(stderr, "An error occurred while executing the command %s. \n", cmd);
            exit(1);
        }
    }
    fd = open(file_name, O_RDWR);
    ftruncate(fd, file_size);

    char* pmem_pool_start = (char*) mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    int num_ints = file_size / 4;

    for (int i = 0; i < num_ints; i++) {
        int* p = (int*)(pmem_pool_start + 4 * i);
        *p = i;
    }

    munmap((void*) pmem_pool_start, file_size);
    close(fd);
}
void pm_mmap_read_test() {
    if (access(file_name, F_OK) != 0) {
        fprintf(stderr, "Error: failed to find the file!");
        exit(1);
    }
    fd = open(file_name, O_RDWR);
    int a;

    char* pmem_pool_start = (char*) mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int num_ints = file_size / 4;

    for (int i = 0; i < num_ints; i++) {
        int* p = (int*)(pmem_pool_start + 4 * i);
        a = *p;
    }

    munmap((void*) pmem_pool_start, file_size);
    close(fd);
}


void dram_write_test() {
    if (arr == nullptr) {
        arr = (int*) malloc(file_size);
    }
    int num_ints = file_size / 4;
    for (int i = 0; i < num_ints; i++) {
        arr[i] = i;
    }
}
void dram_read_test() {
    int a;
    if (arr != nullptr) {
        int num_ints = file_size / 4;
        for (int i = 0; i < num_ints; i++) {
            a = arr[i];
        }
        free(arr);
        arr = nullptr;
    }
}

int main() {

    struct timeval _start_tv, _end_tv;
    long long total_us, single_us;

    set_signal_handlers();
    PCM *m = PCM::getInstance();
    auto status = m->program();
    if (status != PCM::Success) {
        std::cout << "Error opening PCM: " << status << std::endl;
        if (status == PCM::PMUBusy)
            m->resetPMU();
        else
            exit(0);
    }
    // print_cpu_details();

    gettimeofday(&_start_tv, NULL);

    auto before_state = getSystemCounterState();


    pm_mmap_write_test();
    pm_mmap_read_test();

    // pm_io_write_test();
    // pm_io_read_test();

    // dram_write_test();
    // dram_read_test();

    auto after_sstate = getSystemCounterState();

    gettimeofday(&_end_tv, NULL);

    total_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
    printf("taken %lld ms. \n", total_us / 1000);

    cout << "ReadFromPMM: " << getBytesReadFromPMM(before_state, after_sstate) / 1048576 << " MB" << endl;
    cout << "WrittenToPMM: " << getBytesWrittenToPMM(before_state, after_sstate) / 1048576 << " MB" << endl;

    cout << "ReadFromMC: " << getBytesReadFromMC(before_state, after_sstate) / 1048576 << " MB" << endl;
    cout << "WrittenToMC: " << getBytesWrittenToMC(before_state, after_sstate) / 1048576 << " MB" << endl;

    return 0;
}

// g++ test_pcm.cpp -I/home/sungan/Halo-main -L/home/sungan/Halo-main/pcm -lPCM -lpthread
// g++ test_pcm.cpp -I/usr/local/pcm-202311/src -L/usr/local/pcm-202311/build/lib -Wl,-rpath=/usr/local/pcm-202311/build/lib -lpcm -lpthread