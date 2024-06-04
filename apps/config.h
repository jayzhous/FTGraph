#ifndef _NTGRAPH_APPS_CONFIG_H_
#define _NTGRAPH_APPS_CONFIG_H_

#include <cstdint>
#include <string>

typedef uint32_t vtxid_t;
typedef uint32_t edgeid_t;
typedef uint32_t degree_t;
typedef int32_t weight_t;
typedef uint32_t batchid_t;
typedef uint64_t offset_t;
typedef int64_t index_t;
typedef uint32_t level_t;


/* 1. Dataset setting*/

// #define Hollywood_2009      //start_from_1
#define Orkut               //start_from_1
// #define Twitter             //start_from_1
// #define LiveJournal
// #define Graph500_25
// #define Graph500_26
// #define Mini
#define start_from_1
// #define WEIGHTED


/* 2. Print setting*/
// #define DEGREE_COUNTER
// #define PRINT_DETAILED_INFO
// #define CHECK
// #define PCMTEST



/* 3. Function setting*/

#define NO_CONCURRENT
// #define CLWB_SFENCE

// #define GENERATING
// #define DELETION
// #define EDGEMAP  // has bug !
// #define SEQUENTIAL_MEM
// #define NO_CHECK_BEFORE_INSERT
// #define EDGELIST
// #define USE_ITERATOR
// #define HUGE_PAGE // has bug !


/* 4. Query setting */
// #define ONE_HOP
// #define TWO_HOP
// #define BFS
// #define SSSP
// #define CC
// #define PR


namespace {
    static constexpr int      MAX_EL_SIZE     = 1024;
    static constexpr int      INI_EL_SIZE     = 64;
    static constexpr int      EL_START_LV     = 4;      // >= 1, adjacency list
    static constexpr int      START_LV        = 4;      // >= 1, quadtree
    
    static constexpr int      MAX_ENTRY_NUM   = 64;
    static constexpr int      MAX_THREAD_NUM  = 128;
    static constexpr int      ALIGNMENT_SIZE  = 256;
    static constexpr int      ROUND           = 1;
    static constexpr uint32_t BATCH_SIZE = 1000000;
    static constexpr uint64_t UINT64 = 18446744073709551615ULL;
    static constexpr uint64_t UINT32 = 4294967295ULL;
    static constexpr uint64_t UINT16 = 65535ULL;

    const std::string edge_file_directory = "/home/sungan/dataset/";
    const std::string vertex_file_directory = "/mnt/pmem1/sungan/dynamic_graph_230207/vertices/";
    // make sure all graph systems read the same vertices for 1 HOP and 2 HOP query

    const int THREAD_NUM = 16;
}

#endif //_NTGRAPH_APPS_CONFIG_H_