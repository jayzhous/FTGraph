#ifndef _NTGRAPH_COMMON_PERSISTENT_INSTRUCTIONS_H_
#define _NTGRAPH_COMMON_PERSISTENT_INSTRUCTIONS_H_

#include <sys/mman.h>

#define CACHE_LINE_SIZE    64
#define NODE_LINE_NUM      6


#define getline_(addr) (((unsigned long long)(addr)) & (~(unsigned long long)(CACHE_LINE_SIZE - 1)))
#define isaligned_atline(addr) (!(((unsigned long long)(addr)) & (unsigned long long)(CACHE_LINE_SIZE-1)))

static inline void clwb(void* addr) {
    asm volatile("clwb %0": :"m"(*((char*) addr)));
}

static inline void clwb2(void* start, void* end) {
    clwb(start);
    if(getline_(start) != getline_(end)) {
        clwb(end);
    }
}

static inline void clwbmore(void* start, void* end) {
    unsigned long long start_line = getline_(start);
    unsigned long long end_line = getline_(end);
    do {
        clwb((char*) start_line);
        start_line += CACHE_LINE_SIZE;
    } while (start_line <= end_line);
}

static inline void sfence(void) {
    asm volatile("sfence"); 
}


#ifndef NO_PREFETCH
#define prefetcht0(mem_var)     __asm__ __volatile__ ("prefetcht0 %0": :"m"(mem_var))
#define prefetchnta(mem_var)    __asm__ __volatile__ ("prefetchnta %0": :"m"(mem_var))
#else
#define prefetcht0(mem_var)
#define prefetchnta(mem_var)
#endif


static void inline NODE_PREF(void* addr) {
    prefetcht0(*((char*) addr));

    #if NODE_LINE_NUM >= 2
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE));
    #endif

    #if NODE_LINE_NUM >= 3
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 2));
    #endif
    
    #if NODE_LINE_NUM >= 4
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 3));
    #endif

    #if NODE_LINE_NUM >= 5
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 4));
    #endif

    #if NODE_LINE_NUM >= 6
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 5));
    #endif

    #if NODE_LINE_NUM >= 7
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 6));
    #endif

    #if NODE_LINE_NUM >= 8
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 7));
    #endif

    #if NODE_LINE_NUM >= 9
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 8));
    #endif

    #if NODE_LINE_NUM >= 10
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 9));
    #endif

    #if NODE_LINE_NUM >= 11
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 10));
    #endif

    #if NODE_LINE_NUM >= 12
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 11));
    #endif

    #if NODE_LINE_NUM >= 13
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 12));
    #endif

    #if NODE_LINE_NUM >= 14
    prefetcht0(*((char*) addr + CACHE_LINE_SIZE * 13));
    #endif

}


static void inline MADVICE(void* addr, size_t len) {
    madvise(addr, len, MADV_NORMAL);
    // MADV_NORMAL, MADV_SEQUENTIAL
}



#endif // _NTGRAPH_COMMON_PERSISTENT_INSTRUCTIONS_H_