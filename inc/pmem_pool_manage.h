#ifndef _NTGRAPH_INC_PMEM_POOL_MANAGE_H_
#define _NTGRAPH_INC_PMEM_POOL_MANAGE_H_

// #define PERSISTED

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstddef>
#include <cassert>
#include <mutex>

#include <string.h>

#include "common/file_util.h"

#ifndef KB
#define KB      (1024)
#endif

#ifndef MB
#define MB      (1024*KB)
#endif

#ifndef GB
#define GB      (1024*MB)
#endif

#define PAGE_SIZE          (4*KB)

class manage_meta_data {
private:
    uint64_t offset;
    uint64_t node_alignment_size;

    #ifdef NO_CONCURRENT
    uint32_t count;
    #else
    std::atomic<uint32_t> count{0};
    #endif

    

    uint32_t crashed;
    char padding[40];

public:
    inline uint32_t has_crashed() {
        return crashed;
    }
    inline void set_crashed(uint32_t val) {
        crashed = val;
    }
    inline uint32_t get_count() {
        return count;
    }
    inline void set_count(uint32_t _count) {
        
        #ifdef NO_CONCURRENT
        count = _count;
        #else
        count.store(_count);
        #endif

        #ifdef PERSISTED
        clwb((void*) this);
        sfence();
        #endif
    }
    inline void set_offset(uint64_t _offset) {
        offset = _offset;
    }
    inline void update_offset(uint64_t _offset) {
        offset += _offset;
    }
    inline uint64_t get_offset() {
        return offset;
    }
    inline void set_alignment_size(uint64_t _alignment_size) {
        node_alignment_size = _alignment_size;
    }
    inline uint64_t get_alignment_size() {
        return node_alignment_size;
    }
};

class pmem_pool_allocator {
public:

    char* pmem_pool_file;
    uint64_t pmem_pool_size;

    char* pmem_pool_start;
    char* pmem_pool_cur;
    char* pmem_pool_end;
    char* node_start;
    char* tlsmanagers;
    int fd;

    #ifndef NO_CONCURRENT
    std::mutex mtx;
    #endif

    manage_meta_data* meta_data;

    pmem_pool_allocator(char* path, uint64_t size) : pmem_pool_file(path), pmem_pool_size(size) {
        printf("pmem pool manager constructor called. \n");
        init();
    }
    ~pmem_pool_allocator() {
        printf("pmem pool manager destructor called. \n");
        unmap();
    }

    inline manage_meta_data* get_meta_data() {
        return meta_data;
    }

    void init() {
        if (pmem_pool_size < 1024L) {
            fprintf(stderr, "Error: %ld bytes is too small\n", pmem_pool_size);
            exit(1);
        }

        if (file_exists(pmem_pool_file) != 0) {
            char cmd[1024];
            sprintf(cmd, "touch %s", pmem_pool_file);
            if (system(cmd) == -1) {
                fprintf(stderr, "An error occurred while executing the command %s. \n", cmd);
                exit(1);
            }
        }

        fd = open(pmem_pool_file, O_RDWR);
        if (fd < 0){
            fprintf(stderr, "Error: failed to open nvm file\n");
            exit(1);
        }
        if (ftruncate(fd, pmem_pool_size) < 0){
            fprintf(stderr, "Error: failed to truncate file\n");
            exit(1);
        }
        pmem_pool_start = (char*) mmap(0, pmem_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        //pmem_pool_start = (char*) mmap(0, pmem_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE | MAP_SYNC, fd, 0);


        pmem_pool_end = pmem_pool_start + pmem_pool_size;

        //memset(pmem_pool_start, 0, pmem_pool_size);
        
        meta_data = (manage_meta_data*) pmem_pool_start;

        if (meta_data->get_offset() == 0) {
            pmem_pool_cur = pmem_pool_start + sizeof(manage_meta_data);
            meta_data->set_offset(sizeof(manage_meta_data));
        } else {
            pmem_pool_cur = pmem_pool_start + meta_data->get_offset();
        }
    }
    void unmap() {
        munmap((void*) pmem_pool_start, pmem_pool_size);
        close(fd);
    }
    inline char* get_base() {
        return pmem_pool_start;
    }
    inline char* get_cur() {
        return pmem_pool_cur;
    }

    inline char* get_node_start() {
        return node_start;
    }

    void* alloc(uint64_t size) {
        if (pmem_pool_cur + size <= pmem_pool_end) {

            #ifndef NO_CONCURRENT
            mtx.lock();
            #endif

            char* p = pmem_pool_cur;
            pmem_pool_cur += size;

            meta_data->update_offset(size);

            #ifdef PERSISTED
            clwb((void*) meta_data);
            sfence();
            #endif

            #ifndef NO_CONCURRENT
            mtx.unlock();
            #endif

            return (void*) p;
        }
        fprintf(stderr, "%s alloc - run out of memory!\n", pmem_pool_file);
        exit(1);
    }

    inline void set_tlsmanagers(char* _tlsmanagers) {
        tlsmanagers = _tlsmanagers;
    }

    inline char* get_tlsmanagers() {
        return tlsmanagers;
    }

    inline void set_node_start(char* _node_start) {
        node_start = _node_start;
    }

    void print_usage() {
        long long  used = pmem_pool_cur - pmem_pool_start;
        printf("NVM usage: total %.2lf GB, use %.2lf GB \n", ((double) pmem_pool_size) / GB, ((double) used) / GB );
    }

};


#endif // _NTGRAPH_INC_PMEM_POOL_MANAGE_H_