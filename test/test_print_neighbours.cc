#include "inc/ntgraph.h"

int main() {
    // std::string path = "Orkut";
    // uint64_t size = 8L * 1024 * 1024 * 1024;
    // pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    // vtxid_t num_vtxs = 3072626;
    // ntgraph graph(nvm_alloc, num_vtxs, 0, 0);


    std::string path = "Twitter";
    uint64_t size = 40L * 1024 * 1024 * 1024;
    pmem_pool_allocator* nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);
    vtxid_t num_vtxs = 61578414;
    ntgraph graph(nvm_alloc, num_vtxs, 1, 0);

    
    /********************************** print neighbours **********************************/
    vtxid_t src = 59535096;  // degree = 1272
    uint32_t count = 0;
    vertex_index& vtx_index = graph.get_vertex_index();
    printf("degree = %u, num_nodes = %u\n", graph.get_out_degree(src), vtx_index.get_num_nodes(src));

    index_t src_index = vtx_index.find_vertex(src);
    vertex* src_vertex = vtx_index.indexed_vertex(src_index);
    node* cur = (node*) graph.root_node_addr(src_index);

    // edge* edges = cur->get_edges();
    // for (vtxid_t i = 0; i < cur->get_num_edges(); i++) {
    //     printf("%u ", vtx_index.indexed_vtxid(edges[i].id));
    // }
    // printf("\n");

    uint32_t num_nodes = vtx_index.get_num_nodes(src);
    node** n_ptrs = (node**) malloc(sizeof(node*) * (num_nodes - 1));
    

    int front = -1;
    int rear = -1;

    if (cur->node_lv < EL_START_LV - 1) {
        for (int j = 0; j < cur->get_num_branch(); j++) {
            rear += 1;
            n_ptrs[rear] = graph.offset_to_node(cur->first_child_offset + (sizeof(node) * j));
        }
    }

    uint32_t ne_tree = cur->get_num_edges();
    uint32_t ne_el = 0;
    uint32_t n_el = 0;
    
    while (front != rear) {
        front += 1;
        cur = n_ptrs[front];

        ne_tree += cur->get_num_edges();

        if (cur->is_offset_valid != 0) {
            if (cur->node_lv >= EL_START_LV - 1) { 
                continue;
            } 
            for (int j = 0; j < cur->get_num_branch(); j++) {
                rear += 1;
                if (rear == num_nodes - 1) {
                    fprintf(stderr, "Error: n_ptrs queue overflows, source vertex id = %u. \n", src);
                    exit(1);
                }
                n_ptrs[rear] = graph.offset_to_node(cur->first_child_offset + (sizeof(node) * j));
            }
        }
    }

    edgelist* el = src_vertex->first_el;
    while (el != nullptr) {
        n_el += 1;
        ne_el += el->get_num_edges();
        printf("el: %u\n", el->get_num_edges());
        el = el->next_el;
    }
    printf("num of edges : %u\n", ne_tree + ne_el);
    printf("num of edges in the tree: %u\n", ne_tree);
    printf("num of edges in the edgelist: %u\n", ne_el);
    printf("num of edgelist: %u\n", n_el);


    return 0;
}