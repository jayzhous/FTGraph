#include "inc/ntgraph.h"

int main() {
    std::string path = "Orkut";
    uint64_t size = 3L * 1024 * 1024 * 1024;
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    vtxid_t num_vtxs = 3072626;
    ntgraph graph(nvm_alloc, num_vtxs, 0, 0);

    // std::string path = "Twitter";
    // uint64_t size = 40L * 1024 * 1024 * 1024;
    // pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    // vtxid_t num_vtxs = 61578414;
    // ntgraph graph(nvm_alloc, num_vtxs, 1, 0);

    // std::string path = "Graph500_25";
    // uint64_t size = 25L * 1024 * 1024 * 1024;
    // pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    // vtxid_t num_vtxs = 33554431;
    // ntgraph graph(nvm_alloc, num_vtxs, 0, 0);

    vertex_index& vtx_index = graph.get_vertex_index();

    // assert all edges in the dataset have been inserted into the graph
    /********************************** print neighbours **********************************/
    uint32_t count = 0;

    vtxid_t src = 1;
    
    printf("degree = %u, num_nodes = %u\n", graph.get_out_degree(src), vtx_index.get_num_nodes(src));

    std::vector<edge> adj_edges;
    graph.get_adj_edges(src, adj_edges);
    for (int i = 0; i < adj_edges.size(); i++) {
        printf("%u  ", vtx_index.indexed_vtxid(adj_edges[i].id));
        count += 1;
    }
    printf("\ncount = %u\n\n", count);

    // ntgraph::iterator2 iter(graph, vtx_index.find_vertex(src));
    // count = 0;
    // while (!iter.done()) {
    //     // printf("%u  ", vtx_index.indexed_vtxid((*iter)->id));
    //     ++iter;
    //     count += 1;
    // }
    // printf("\ncount = %u\n\n", count);

    // vtxid_t id;
    // for (vtxid_t i = 0; i < graph.get_num_vertices(); i++) {
    //     if (vtx_index.find_vertex(i) == -1) continue;
    //     ntgraph::sbtree_iterator iter2(graph, vtx_index.find_vertex(i));
    //     count = 0;
    //     while (!iter2.done()) {
    //         // printf("%u  ", (*iter2)->id);
    //         ++iter2;
    //         count += 1;
    //     }
    //     if (count != vtx_index.get_degree(i)) {
    //         printf("vertex = %u, count = %u, degree = %u\n", i, count, vtx_index.get_degree(i));
    //         // assert(count == vtx_index.get_degree(i));
    //     }
    //     if (count > 3000 && count < 4000) {
    //         id = i;
    //     }
    // }
    // printf("vid = %u, degree = %u\n", id, vtx_index.get_degree(id));
    
    return 0;
}