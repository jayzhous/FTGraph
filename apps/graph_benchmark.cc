#include <set>
#include <cstdlib>
#include <ctime>
#include <map>
#include <sys/time.h>


#include "inc/ntgraph.h"
#include "apps/ingestion.h"
// #include "apps/deletion.h"
#include "apps/one_hop.h"
#include "apps/two_hop.h"
#include "apps/bfs.h"
#include "apps/dijkstra.h"
#include "apps/bellman_ford.h"
#include "apps/cc.h"
#include "apps/page_rank.h"
#include "apps/degree.h"
#include "common/file_util.h"

#include "apps/bellman_ford_edgemap.h"
#include "apps/bfs_edgemap.h"
#include "apps/cc_edgemap.h"

#ifdef PCMTEST
#include "cpucounters.h"
#include "utils.h"
#endif

void load_file_to_vertices(ntgraph &g, std::string file_name, std::set<vtxid_t>& query_vtxids) {
    vertex_index& vtx_index = g.get_vertex_index();

    std::string path = vertex_file_directory + file_name;
    std::ifstream ifs;
    ifs.open(path, std::ios::in);
    if (!ifs.is_open()) {
        std::cout << "Error: failed to open file " << file_name << " (load_file_to_vertices)!" << std::endl;
        exit(1);
    }
    std::string line;
    while (getline(ifs, line)) {
        if (line == "" || line == "\n") {
            continue;
        }
        query_vtxids.insert(vtx_index.find_vertex(std::stoi(line)));
    }
    ifs.close();
}

void root_generator(ntgraph &g, vtxid_t query_count, std::set<vtxid_t>& query_vtxids) {
    srand((int)time(0));
    vertex_index& vtx_index = g.get_vertex_index();
    
    while (query_vtxids.size() < query_count) {
        vtxid_t vtxid = rand() % g.get_num_vertices();  // 0 ~ VERTEX_NUM - 1, physical id
        // vtxid is physical id
        if (vtx_index.find_vertex(vtxid) == -1) {
            continue;
        }
        degree_t d = g.degree(vtxid);
        if (d > 0){
            query_vtxids.insert(vtxid);
        }
    }
}

void dump_vertices_to_file(std::string file_name, std::set<vtxid_t>& query_vtxids) {
    std::string path = vertex_file_directory + file_name;
    std::string content;

    std::set<vtxid_t>::iterator iter;
    for(iter = query_vtxids.begin(); iter != query_vtxids.end(); iter++){
        content += std::to_string(*iter) + "\n";
    }
    write_file(path, content);
}



int main() {

    struct timeval _start_tv, _end_tv;
    long long total_us, single_us;

    #ifdef Mini                         
    vtxid_t num_vtxs = 14;      // max id + 1
    edgeid_t num_edges = 24;    // undirected: num_of_lines * 2, directed: num_of_line
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;
    std::string dataset = "../mini.txt";

    uint64_t size = 1L * 1024 * 1024 * 1024;
    std::string path = "Mini";
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    #endif
    

    #ifdef Graph500_25
    // 1_HOP : 16131024
    // 2_HOP : 18070047822
    // BFS : 103863060
    // SSSP :
        // dijkstra : 
        // bellman ford : 51179249
    // PR : 47102300.4776
    // CC: 16497599
    std::string file_name_1hop = "Graph500_25_1hop.txt";
    std::string file_name_2hop = "Graph500_25_2hop.txt";
    std::vector<vtxid_t> bfs_src_vtxids = {6, 7, 128};
    std::vector<vtxid_t> src_vtxids = {6};

    vtxid_t num_vtxs = 33554431;
    edgeid_t num_edges = 1047205662;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;

    std::string dataset = "graph500-25/graph500-25.e";
    std::string path = "Graph500_25";

    uint64_t size = 25L * 1024 * 1024 * 1024;
    
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    gettimeofday(&_start_tv, NULL);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    gettimeofday(&_end_tv, NULL);
    total_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
    printf("G25 graph recovery takes %lld ms. \n", total_us / 1000);
    #endif


    #ifdef Graph500_26
    // 1_HOP : 15952993
    // 2_HOP : 25256060265
    // BFS : 301351442
    // SSSP :
        // dijkstra : 
        // bellman ford : 94011452
    // PR : 93621105.7759
    // CC: 34314611
    std::string file_name_1hop = "Graph500_26_1hop.txt";
    std::string file_name_2hop = "Graph500_26_2hop.txt";
    std::vector<vtxid_t> bfs_src_vtxids = {7, 17, 132};
    std::vector<vtxid_t> src_vtxids = {17};

    vtxid_t num_vtxs = 67108863;
    edgeid_t num_edges = 2103845706;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;

    std::string dataset = "graph500-26/graph500-26.e";
    std::string path = "Graph500_26";

    uint64_t size = 50L * 1024 * 1024 * 1024;
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    gettimeofday(&_start_tv, NULL);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    gettimeofday(&_end_tv, NULL);
    total_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
    printf("G26 graph recovery takes %lld ms. \n", total_us / 1000);
    #endif
    
    #ifdef Hollywood_2009
    // 1_HOP : 26143958
    // 2_HOP : 2840577878
    // BFS : 12297344
    // SSSP :
        // bellman ford : 4546477
    // PR : 
    // CC: 44508
    std::string file_name_1hop = "Hollywood_2009_1hop.txt";
    std::string file_name_2hop = "Hollywood_2009_2hop.txt";
    std::vector<vtxid_t> bfs_src_vtxids = {152756, 421257, 934065};
    std::vector<vtxid_t> src_vtxids = {421257};

    vtxid_t num_vtxs = 1139905;
    edgeid_t num_edges = 115031232;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;
    std::string dataset = "hollywood-2009.mtx";

    uint64_t size = 6L * 1024 * 1024 * 1024;
    std::string path = "Hollywood_2009";
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    #endif

    #ifdef Orkut
    // 1_HOP : 19899118
    // 2_HOP : 961710516
    // BFS : 40259283
    // SSSP : 14197095
    // PR : 9388806.7822
    // CC: 186
    std::string file_name_1hop = "Orkut_1hop.txt";
    std::string file_name_2hop = "Orkut_2hop.txt";
    std::vector<vtxid_t> bfs_src_vtxids = {898608, 1119370, 2452559};    
    std::vector<vtxid_t> src_vtxids = {2921746};
    
    vtxid_t num_vtxs = 3072626;
    edgeid_t num_edges = 234370166;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;

    std::string dataset = "com-orkut.ungraph.txt";
    std::string path = "Orkut";

    // std::string dataset = "com-orkut.ungraph_shuffle.txt";
    // std::string path = "Orkut_shuffle";

    uint64_t size = 10L * 1024 * 1024 * 1024;
    
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    
    gettimeofday(&_start_tv, NULL);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    gettimeofday(&_end_tv, NULL);
    total_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
    printf("Orkut graph recovery takes %lld ms. \n", total_us / 1000);
    #endif

    #ifdef LiveJournal
    // 1_HOP : 4549932
    // 2_HOP : 68836391
    // BFS : 65152648
    // SSSP : 19751092
    // PR : 5265003.5405
    // CC: 38577
    std::string file_name_1hop = "LiveJournal_1hop.txt";
    std::string file_name_2hop = "LiveJournal_2hop.txt";
    std::vector<vtxid_t> bfs_src_vtxids = {380065, 1154699, 2900047};
    std::vector<vtxid_t> src_vtxids = {1713673};
                                                                               
    vtxid_t num_vtxs = 4036538;
    edgeid_t num_edges = 69362378;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;
    std::string dataset = "com-lj.ungraph.txt";

    uint64_t size = 4L * 1024 * 1024 * 1024;
    std::string path = "LiveJournal";
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);

    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    #endif

    #ifdef Twitter
    // 1_HOP : 14777679
    // 2_HOP : 199410716306
    // BFS : 407355724
    // SSSP :
        // bellman ford : 137468081
    // PR : 107464120.3224
    // CC : 19926185

    // Directed: 
    // 1_HOP : 8853632
    // 2_HOP : 7820918223
    // BFS : 407900218
    // SSSP :
        // bellman ford : 124945887
    // PR : 74586534.4539
    // CC : 19926185
    std::string file_name_1hop = "Twitter_1hop.txt";
    std::string file_name_2hop = "Twitter_2hop.txt";
    std::vector<vtxid_t> bfs_src_vtxids = {4106, 13009, 48960346};
    std::vector<vtxid_t> src_vtxids = {13009};

    vtxid_t num_vtxs = 61578414;
    // edgeid_t num_edges = 2936730364;
    edgeid_t num_edges = 1468365182;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;

    std::string dataset = "twitter_rv.net";
    std::string path = "Twitter";

    // std::string dataset = "twitter_rv_shuffle.net";
    // std::string path = "Twitter_undirected_shuffle";


    uint64_t size = 40L * 1024 * 1024 * 1024;
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);

    gettimeofday(&_start_tv, NULL);
    ntgraph graph(nvm_alloc, num_vtxs, 1, 0);
    gettimeofday(&_end_tv, NULL);
    total_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
    printf("Twitter graph recovery takes %lld ms. \n", total_us / 1000);
    #endif

    #ifdef PCMTEST
    using namespace pcm;
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
    auto before_state = getSystemCounterState();
    #endif

    test_ingestion_performance(graph, dataset, num_batches, num_edges);

    #ifdef PCMTEST
    auto after_sstate = getSystemCounterState();
    printf("ReadFromPMM: %.1f GB\n", (double) getBytesReadFromPMM(before_state, after_sstate) / 1073741824);
    printf("WrittenToPMM: %.1f GB\n", (double) getBytesWrittenToPMM(before_state, after_sstate) / 1073741824);

    printf("ReadFromMC: %.1f GB\n", (double) getBytesReadFromMC(before_state, after_sstate) / 1073741824);
    printf("WrittenToMC: %.1f GB\n", (double) getBytesWrittenToMC(before_state, after_sstate) / 1073741824);
    #endif


    #ifdef DELETION
    test_deletion_performance(graph, dataset, num_batches, num_edges);
    #endif
    

    #ifdef DEGREE_COUNTER
    avg_degree(graph);
    #endif




    printf("\n");

    /************************************ 1-hop ************************************/
    #ifdef ONE_HOP
    vtxid_t query_count_1hop = 1 << 18;
    // 1<<15=32768,1<<18=262144,1<<20=1048576
    std::set<vtxid_t> query_vtxids_1hop;

    #ifdef GENERATING
    root_generator(graph, query_count_1hop, query_vtxids_1hop);
    //printf("query_vtxids_1hop size = %ld\n", query_vtxids_1hop.size());
    assert(query_vtxids_1hop.size() == query_count_1hop);
    dump_vertices_to_file(file_name_1hop, query_vtxids_1hop);
    #endif

    query_vtxids_1hop.clear();
    load_file_to_vertices(graph, file_name_1hop, query_vtxids_1hop);
    
    
    
    gettimeofday(&_start_tv, NULL);
    degree_t total_degree_1hop = test_one_hop(graph, query_vtxids_1hop);
    gettimeofday(&_end_tv, NULL);

    total_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
    printf("one hop for %d vertices, sum of their 1-hop neighbors = %u, taken %lld ms. \n", query_count_1hop, total_degree_1hop, total_us / 1000);
    #endif


    /************************************ 2-hop ************************************/
    #ifdef TWO_HOP
    vtxid_t query_count_2hop = 1 << 15;
    // 1<<15=32768,1<<18=262144,1<<20=1048576
    std::set<vtxid_t> query_vtxids_2hop;

    #ifdef GENERATING
    root_generator(graph, query_count_2hop, query_vtxids_2hop);
    assert(query_vtxids_2hop.size() == query_count_2hop);
    dump_vertices_to_file(file_name_2hop, query_vtxids_2hop);
    #endif

    query_vtxids_2hop.clear();
    load_file_to_vertices(graph, file_name_2hop, query_vtxids_2hop);
    
    total_us = 0;
    for (int i = 0; i < ROUND; i++) {

        #ifdef USE_ITERATOR
        #ifndef EDGELIST
        gettimeofday(&_start_tv, NULL);
        uint64_t total_degree_2hop = test_two_hop3(graph, query_vtxids_2hop);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("two hop for %d vertices, sum of their 2-hop neighbors = %lu, taken %lld ms. \n", query_count_2hop, total_degree_2hop, single_us / 1000);
        #else
        gettimeofday(&_start_tv, NULL);
        uint64_t total_degree_2hop = test_two_hop5(graph, query_vtxids_2hop);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("two hop for %d vertices, sum of their 2-hop neighbors = %lu, taken %lld ms. \n", query_count_2hop, total_degree_2hop, single_us / 1000);
        #endif
        #else

        gettimeofday(&_start_tv, NULL);
        uint64_t total_degree_2hop = test_two_hop2(graph, query_vtxids_2hop);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("two hop for %d vertices, sum of their 2-hop neighbors = %lu, taken %lld ms. \n", query_count_2hop, total_degree_2hop, single_us / 1000);
        #endif

        total_us += single_us;
    }

    printf("two hop: taken %lld ms. \n\n", total_us / 1000 / ROUND);

    #endif


    /************************************ bfs ************************************/
    #ifdef BFS
    uint64_t total_count = 0;

    total_us = 0;
    for (int i = 0; i < ROUND; i++) {

        #ifdef USE_ITERATOR
        #ifndef EDGELIST
        gettimeofday(&_start_tv, NULL);
        for (vtxid_t i = 0; i < bfs_src_vtxids.size(); i++) {
            total_count += test_bfs3(graph, bfs_src_vtxids[i]);
        }
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("bfs for %ld source vertices, sum of depth = %lu, taken %lld ms. \n", bfs_src_vtxids.size(), total_count, single_us / 1000);
        #else
        total_count = 0;
        gettimeofday(&_start_tv, NULL);
        for (vtxid_t i = 0; i < bfs_src_vtxids.size(); i++) {
            total_count += test_bfs5(graph, bfs_src_vtxids[i]);
        }
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("bfs for %ld source vertices, sum of depth = %lu, taken %lld ms. \n", bfs_src_vtxids.size(), total_count, single_us / 1000);
        #endif
        #else

        total_count = 0;
        gettimeofday(&_start_tv, NULL);
        for (vtxid_t i = 0; i < bfs_src_vtxids.size(); i++) {
            total_count += test_bfs2(graph, bfs_src_vtxids[i]);
        }
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("bfs for %ld source vertices, sum of depth = %lu, taken %lld ms. \n", bfs_src_vtxids.size(), total_count, single_us / 1000);
        #endif

        #ifdef EDGEMAP
        total_count = 0;
        gettimeofday(&_start_tv, NULL);
        for (vtxid_t i = 0; i < bfs_src_vtxids.size(); i++) {
            total_count += BFS_with_edge_map(graph, bfs_src_vtxids[i]);
        }
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("bfs for %ld source vertices, sum of depth = %lu, taken %lld ms. \n", bfs_src_vtxids.size(), total_count, single_us / 1000);
        #endif

        total_us += single_us;
    }
    printf("bfs: taken %lld ms. \n\n", total_us / 1000 / ROUND);
    
    #endif

    /************************************ sssp ************************************/
    #ifdef SSSP
    int64_t total_dist = 0;
    bool has_negative_cycle;

    total_us = 0;
    for (int i = 0; i < ROUND; i++) {

        #ifdef USE_ITERATOR
        #ifndef EDGELIST
        total_dist = 0;
        gettimeofday(&_start_tv, NULL);
        for (vtxid_t i = 0; i < src_vtxids.size(); i++) {
            total_dist += bellman_ford3(graph, &has_negative_cycle, src_vtxids[i]);
        }
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("bellman ford for %ld source vertices, has_negative_cycle: %d, sum of shortest distance (just for verification) = %lu, taken %lld ms. \n", src_vtxids.size(), has_negative_cycle, total_dist, single_us / 1000);
        #else
        total_dist = 0;
        gettimeofday(&_start_tv, NULL);
        for (vtxid_t i = 0; i < src_vtxids.size(); i++) {
            total_dist += bellman_ford5(graph, &has_negative_cycle, src_vtxids[i]);
        }
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("bellman ford for %ld source vertices, has_negative_cycle: %d, sum of shortest distance (just for verification) = %lu, taken %lld ms. \n", src_vtxids.size(), has_negative_cycle, total_dist, single_us / 1000);
        #endif
        #else

        total_dist = 0;
        gettimeofday(&_start_tv, NULL);
        for (vtxid_t i = 0; i < src_vtxids.size(); i++) {
            total_dist += bellman_ford2(graph, &has_negative_cycle, src_vtxids[i]);
        }
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("bellman ford for %ld source vertices, has_negative_cycle: %d, sum of shortest distance (just for verification) = %lu, taken %lld ms. \n", src_vtxids.size(), has_negative_cycle, total_dist, single_us / 1000);
        #endif

        #ifdef EDGEMAP
        int32_t* result;
        gettimeofday(&_start_tv, NULL);
        for (vtxid_t i = 0; i < src_vtxids.size(); i++) {
            result = SSSP_BF(graph, src_vtxids[i]);
        }
        gettimeofday(&_end_tv, NULL);
        total_dist = 0;
        for (int i = 0; i < num_vtxs; i++) {
            if (result[i] != INT_MAX / 2) {
                total_dist += result[i];
            }
        }
        free(result);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("bellman ford for %ld source vertices, sum of shortest distance (just for verification) = %lu, taken %lld ms. \n", src_vtxids.size(), total_dist, single_us / 1000);
        #endif
        total_us += single_us;
    }
    printf("bf: taken %lld ms. \n\n", total_us / 1000 / ROUND);
    #endif


    /************************************ cc ************************************/
    #ifdef CC
    uint64_t ncc = 0;

    total_us = 0;
    for (int i = 0; i < ROUND; i++) {

        #ifdef USE_ITERATOR
        #ifndef EDGELIST
        gettimeofday(&_start_tv, NULL);
        ncc = test_cc3(graph);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("num of connected components = %lu, taken %lld ms. \n", ncc, single_us / 1000);
        #else
        gettimeofday(&_start_tv, NULL);
        ncc = test_cc5(graph);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("num of connected components = %lu, taken %lld ms. \n", ncc, single_us / 1000);
        #endif
        #else

        gettimeofday(&_start_tv, NULL);
        ncc = test_cc2(graph);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("num of connected components = %lu, taken %lld ms. \n", ncc, single_us / 1000);
        #endif

        #ifdef EDGEMAP
        gettimeofday(&_start_tv, NULL);
        auto cc_result = CC_T(graph);
        gettimeofday(&_end_tv, NULL);
        free(cc_result);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("cc: taken %lld ms. \n", single_us / 1000);
        #endif
        total_us += single_us;
    }
    printf("cc: taken %lld ms. \n\n", total_us / 1000 / ROUND);
    #endif

    /************************************ pr ************************************/
    #ifdef PR

    total_us = 0;
    for (int i = 0; i < ROUND; i++) {

        #ifdef USE_ITERATOR
        #ifndef EDGELIST
        gettimeofday(&_start_tv, NULL);
        page_rank3(graph, 10);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("page rank time: %lld ms. \n", single_us / 1000);
        #else
        gettimeofday(&_start_tv, NULL);
        page_rank5(graph, 10);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("page rank time: %lld ms. \n", single_us / 1000);
        #endif
        #else
        gettimeofday(&_start_tv, NULL);
        page_rank2(graph, 10);
        gettimeofday(&_end_tv, NULL);
        single_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
        printf("page rank time: %lld ms. \n", single_us / 1000);
        #endif
        total_us += single_us;
    }
    printf("pr: taken %lld ms. \n\n", total_us / 1000 / ROUND);

    #endif

    return 0;
}