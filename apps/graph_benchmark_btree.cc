#include <set>
#include <cstdlib>
#include <ctime>
#include <map>
#include <sys/time.h>

#include "inc/ntgraph_btree.h"
#include "apps/ingestion.h"

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

int main() {

    #ifdef LiveJournal
    vtxid_t num_vtxs = 4036538;
    edgeid_t num_edges = 69362378;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;
    std::string dataset = "com-lj.ungraph.txt";
    std::string path = "../lj_canbedeleted";
    uint64_t size = 8L * 1024 * 1024 * 1024;
    nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    #endif

    #ifdef Orkut
    vtxid_t num_vtxs = 3072626;
    edgeid_t num_edges = 234370166;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;
    std::string dataset = "com-orkut.ungraph.txt";
    std::string path = "../ok_canbedeleted";
    uint64_t size = 15L * 1024 * 1024 * 1024;
    nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    #endif

    #ifdef Twitter
    vtxid_t num_vtxs = 61578414;
    edgeid_t num_edges = 1468365182;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;
    std::string dataset = "twitter_rv.net";
    std::string path = "../tt_canbedeleted";
    uint64_t size = 88L * 1024 * 1024 * 1024;
    nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    ntgraph graph(nvm_alloc, num_vtxs, 1, 0);
    #endif

    #ifdef Graph500_25
    vtxid_t num_vtxs = 33554431;
    edgeid_t num_edges = 1047205662;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;
    std::string dataset = "graph500-25/graph500-25.e";
    std::string path = "../g25_canbedeleted";
    uint64_t size = 60L * 1024 * 1024 * 1024;
    nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    #endif


    #ifdef Graph500_26
    vtxid_t num_vtxs = 67108863;
    edgeid_t num_edges = 2103845706;
    batchid_t num_batches = num_edges / BATCH_SIZE + 1;
    std::string dataset = "graph500-26/graph500-26.e";
    std::string path = "../g26_canbedeleted";
    uint64_t size = 120L * 1024 * 1024 * 1024;
    nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);
    #endif

    node_start = (char*) nvm_alloc->get_node_start();

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

}

// g++ graph_benchmark_btree.cc -I/mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/ -I/usr/local/pcm-202311/src -L/usr/local/pcm-202311/build/lib -Wl,-rpath=/usr/local/pcm-202311/build/lib -lpcm -lpthread