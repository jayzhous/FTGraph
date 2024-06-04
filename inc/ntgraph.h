#ifndef _NTGRAPH_INC_NTGRAPH_H_
#define _NTGRAPH_INC_NTGRAPH_H_

#include <cstring>
#include <map>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <set>
#include <atomic>
#include <climits>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <omp.h>


#ifndef NO_CONCURRENT
#include <tbb/parallel_for.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#endif

#include "apps/config.h"
#include "common/persistent_instructions.h"
#include "common/version_lock.h"
#include "inc/pmem_pool_manage.h"
#include "inc/vertex_index.h"
#include "inc/tree_node.h"


class ntgraph {
private:
    pmem_pool_allocator* nvm_alloc = nullptr;
    vertex_index vtx_index;
    vertex* vertices;
    char* node_start;
    

    vtxid_t num_vertices;
    bool is_directed; // 1 : directed, 0 : undirected
    bool is_weighted; // 1 : weighted, 0 : unweighted
    
public:

    
    class iterator2 {
    public:
        iterator2(ntgraph& g, vtxid_t v): graph(g){
            // v is a logical id
            vertex_index& vtx_index = graph.get_vertex_index();
            vertex* src_vertex = vtx_index.indexed_vertex(v);
            node* cur = (node*) graph.root_node_addr(v);
            
            edges = cur->get_edges();
            local_el = nullptr;

            local_size = cur->get_num_edges();
            local_idx = 0;

            ptr_size = 0;
            ptr_idx = -1;
            n_ptrs = nullptr;

            uint32_t num_nodes = src_vertex->get_num_nodes();
            if (num_nodes <= 1) {
                return;
            }

            if (cur->node_lv >= EL_START_LV - 1) {
                local_el = src_vertex->first_el;
                return;
            }

            // only have root node, n_ptrs = nullptr and ptr_size = 0
            ptr_size = num_nodes - 1;
            n_ptrs = (node**) malloc(sizeof(node*) * (num_nodes - 1));

            int front = -1;
            int rear = -1;

            for (int j = 0; j < cur->get_num_branch(); j++) {
                rear += 1;
                n_ptrs[rear] = graph.offset_to_node(cur->first_child_offset + (sizeof(node) * j));
            }
            
            while (front != rear) {
                front += 1;
                cur = n_ptrs[front];

                if (cur->is_offset_valid != 0) {
                    if (cur->node_lv >= EL_START_LV - 1) {
                        local_el = src_vertex->first_el;
                        ptr_size = rear + 1;
                        break;
                    } 
                    for (int j = 0; j < cur->get_num_branch(); j++) {
                        rear += 1;
                        if (rear == num_nodes - 1) {
                            fprintf(stderr, "Error: n_ptrs queue overflows, source vertex id = %u. \n", v);
                            exit(1);
                        }
                        n_ptrs[rear] = graph.offset_to_node(cur->first_child_offset + (sizeof(node) * j));
                    }
                }
            }

            // printf("num_nodes = %u, ptr_size = %u\n", num_nodes - 1, ptr_size);
            // for (int i = 0; i < ptr_size; i++) {
            //     printf("p=%lu,e=%u,node_lv=%d   ", n_ptrs[i], n_ptrs[i]->get_num_edges(), n_ptrs[i]->node_lv);
            // }
            // printf("\n\n");

        }
        inline void operator++ () {
            //when delete an edge, compaction is needed
            local_idx++;
        }
        inline edge* operator*() {
            // if (ptr_idx < ptr_size) {
            //     return &local_node->edges[local_idx];
            // } else {
            //     return &local_el->edges[local_idx];
            // }
            return &edges[local_idx];
        }
        bool done() {
            if (ptr_idx < ptr_size) {
                if (local_idx < local_size) {
                    return false;
                } else {
                    
                    // if (n_ptrs == nullptr) return true;

                    ptr_idx++;
                    if (ptr_idx < ptr_size) {
                        local_size = n_ptrs[ptr_idx]->get_num_edges();
                        edges = n_ptrs[ptr_idx]->get_edges();

                        while (local_size == 0) {
                            ptr_idx++;
                            if (ptr_idx == ptr_size) {

                                if (local_el != nullptr) {
                                    local_size = local_el->get_num_edges();
                                    edges = local_el->get_edges();
                                } else {
                                    return true;
                                }

                                break;
                            }

                            local_size = n_ptrs[ptr_idx]->get_num_edges();
                            edges = n_ptrs[ptr_idx]->get_edges();
                        }
                    } else if (local_el != nullptr) {
                        local_size = local_el->get_num_edges();
                        edges = local_el->get_edges();
                    } else {
                        return true;
                    }

                    local_idx = 0;
                    return false;
                }
            }
            

            if (local_idx < local_size) {
                return false;
            } else {
                local_el = local_el->next_el;
                if (local_el == nullptr) {
                    return true;
                } else {
                    edges = local_el->get_edges();
                    local_size = local_el->get_num_edges();
                    local_idx = 0;
                    return false;
                }
            }
            return true;
        }
        ~iterator2() {
            if (n_ptrs != nullptr) {
                free(n_ptrs);
            }
        }
    private:
        ntgraph& graph;

        uint32_t local_size;
        uint32_t local_idx;
        
        node** n_ptrs;
        int32_t ptr_size;
        int32_t ptr_idx;

        edgelist* local_el;

        edge* edges;
    };

    class sbtree_iterator {
    public:
        sbtree_iterator(ntgraph& g, vtxid_t v): graph(g), source(v) {
            // v is an internal logical id
            vertex_index& vtx_index = graph.get_vertex_index();

            node* cur = (node*) graph.root_node_addr(source);

            local_node = cur;
            local_idx = 0;

            ptr_size = 0;
            ptr_idx = 0;
            n_ptrs = nullptr;

            uint32_t num_nodes = vtx_index.get_num_nodes_l(source);
            if (num_nodes <= 1) {
                return;
            }
            ptr_size = num_nodes - 1;
            n_ptrs = (node**) malloc(sizeof(node*) * (ptr_size));

            int front = -1;
            int rear = -1;

            if (cur->is_offset_valid != 0) {
                for (int j = 0; j < cur->get_num_branch(); j++) {
                    rear += 1;
                    n_ptrs[rear] = graph.offset_to_node(cur->first_child_offset + (sizeof(node) * j));
                }
            }
            while (front != rear) {
                front += 1;
                cur = n_ptrs[front];
                if (cur->is_offset_valid != 0) {
                    for (int j = 0; j < cur->get_num_branch(); j++) {
                        rear += 1;
                        if (rear == ptr_size) {
                            fprintf(stderr, "Error: n_ptrs queue overflows, source vertex id = %u. \n", source);
                            exit(1);
                        }
                        n_ptrs[rear] = graph.offset_to_node(cur->first_child_offset + (sizeof(node) * j));
                    }
                }
            }

        }
        inline void operator++ () {
            //when delete an edge, compaction is needed
            local_idx++;
        }
        inline edge* operator*() {
            return &local_node->edges[local_idx];
        }
        bool done() {
            if (local_idx == local_node->get_num_edges()) {
                if (ptr_idx == ptr_size) {
                    return true;
                } else {
                    local_node = n_ptrs[ptr_idx];
                    ptr_idx++;
                    while (local_node->get_num_edges() == 0) {
                        if (ptr_idx == ptr_size) {
                            return true;
                        }
                        local_node = n_ptrs[ptr_idx];
                        ptr_idx++;
                    }
                    local_idx = 0;
                    return false;
                }
            }
            return false;
        }
        ~sbtree_iterator() {
            if (n_ptrs != nullptr) {
                free(n_ptrs);
            }
        }
    private:
        ntgraph& graph;
        vtxid_t source;
        uint32_t local_idx;
        node** n_ptrs;
        uint32_t ptr_size;
        uint32_t ptr_idx;
        node* local_node;
    };

    ntgraph(pmem_pool_allocator* _nvm_alloc, vtxid_t _num_vertices, bool _is_directed, bool _is_weighted) : 
        nvm_alloc(_nvm_alloc), 
        num_vertices(_num_vertices),
        is_directed(_is_directed),
        is_weighted(_is_weighted) {
        printf("ntgraph constructor called. \n");

        manage_meta_data* meta_data = nvm_alloc->get_meta_data();

        if (meta_data->get_offset() == sizeof(manage_meta_data)) {
            /* a new nvm memory pool */

            // preassign nvm memory for all vertices
            vertex* _vertices = (vertex*) nvm_alloc->alloc(sizeof(vertex) * num_vertices);

            vertices = _vertices;
            vtx_index.init(meta_data, _num_vertices, _vertices);
            assert(nvm_alloc->get_base() + meta_data->get_offset() == nvm_alloc->get_cur());
            

            uint64_t mod = (uint64_t) nvm_alloc->get_cur() % ALIGNMENT_SIZE;

            if (mod != 0) {
                nvm_alloc->alloc(ALIGNMENT_SIZE - mod);
                meta_data->set_alignment_size(ALIGNMENT_SIZE - mod);
            }

            nvm_alloc->set_node_start(nvm_alloc->get_cur());
            assert(isaligned_atline(nvm_alloc->get_node_start()) == 1);
            node_start = (char*) nvm_alloc->get_node_start();
            nvm_alloc->alloc(sizeof(node) * num_vertices);

            #pragma omp parallel for
            for (vtxid_t i = 0; i < num_vertices; i++) {
                node* n = (node*) (node_start + sizeof(node) * i);
                n->size_lv = 2;

                #ifndef NO_CONCURRENT
                n->lock.reset_version();
                #endif
            }
            meta_data->set_crashed(1);
        } else {
            // read from an old nvm file
            vertex* _vertices = (vertex*) (nvm_alloc->get_base() + sizeof(manage_meta_data));

            vtxid_t count = meta_data->get_count();
            vtx_index.init(meta_data, _num_vertices, _vertices, count);

            if (meta_data->has_crashed() == 1) {
                printf("the system has crashed, recovering ... \n");
                // recover degree ..
                // recover num of nodes for each vertex ..

            }
            if (vtx_index.count_vertex() > num_vertices) {
                vtx_index.set_high(num_vertices);
            }
            //re-establish the mapping relationship (external physical vertex id to internal logical vertex id)
            vtx_index.re_map();

            nvm_alloc->set_node_start((char*) vtx_index.get_vertex_ptr() + sizeof(vertex) * num_vertices + meta_data->get_alignment_size());
            node_start = (char*) nvm_alloc->get_node_start();
            
            #ifdef EDGELIST
            init_edgelist();
            #endif

        }
        printf("ntgraph initialization completed. \n");
    }

    ~ntgraph() {
        printf("ntgraph destructor called. \n");
        destructor();
    }

    void destructor() {
        manage_meta_data* meta_data = nvm_alloc->get_meta_data();
        meta_data->set_crashed(0);
        meta_data->set_count(vtx_index.count_vertex());

        #ifdef EDGELIST
        uint64_t bytes = vtx_index.get_num_elbytes();
        printf("DRAM usage: use %.4lf GB \n", ((double) bytes) / GB );
        vtx_index.del_edgelist();
        #endif

        nvm_alloc->print_usage();
        delete nvm_alloc;
    }

    void init_edgelist() {
        #pragma omp parallel for
        for (vtxid_t src = 0; src < num_vertices; src++) {
            
            index_t src_index = vtx_index.find_vertex(src);
            if (src_index < 0) continue;

            vertex* src_vertex = vtx_index.indexed_vertex(src_index);
            node* cur = (node*) root_node_addr(src_index);

            if (EL_START_LV == 0) {
                for (int j = 0; j < cur->level_to_size(); j++) {
                    if (cur->bitmap.get(j) == 1) {
                        src_vertex->insert_el(cur->edges[j]);
                    }
                }
            }

            uint32_t num_nodes = vtx_index.get_num_nodes(src);
            if (num_nodes <= 1) {
                continue;
            }
            uint32_t ptr_size = num_nodes - 1;
            node** queue = (node**) malloc(sizeof(node*) * (ptr_size));

            int front = -1;
            int rear = -1;
            for (int j = 0; j < cur->get_num_branch(); j++) {
                rear += 1;
                queue[rear] = offset_to_node(cur->first_child_offset + (sizeof(node) * j));
            }
            
            while (front != rear) {
                front += 1;
                cur = queue[front];
                if (cur->is_offset_valid != 0) {
                    for (int j = 0; j < cur->get_num_branch(); j++) {
                        rear += 1;
                        if (rear == ptr_size) {
                            fprintf(stderr, "Error: n_ptrs queue overflows, source vertex id = %u. \n", src);
                            exit(1);
                        }
                        queue[rear] = offset_to_node(cur->first_child_offset + (sizeof(node) * j));
                    }
                }
            }
            
            for (int i = 0; i < ptr_size; i++) {
                cur = queue[i];
                if (cur->node_lv >= EL_START_LV) {
                    for (int j = 0; j < cur->level_to_size(); j++) {
                        if (cur->bitmap.get(j) == 1) {
                            src_vertex->insert_el(cur->edges[j]);
                        }
                    }
                }
            }
            free(queue);
        }
    }

    inline vertex_index& get_vertex_index() {
        return vtx_index;
    }

    inline degree_t degree(const vtxid_t v) const {
        return vertices[v].degree;
    }

    inline vtxid_t get_num_vertices() {
        return num_vertices;
    }

    vtxid_t get_num_physical_vertices() {
        return vtx_index.get_num_physical_vertices();
    }

    pmem_pool_allocator* get_nvm_allocator() {
        assert(nvm_alloc != nullptr);
        return nvm_alloc;
    }

    inline bool is_directed_g() {
        return is_directed;
    }

    inline bool is_weighted_g() {
        return is_weighted;
    }


    template <class F, typename VS>
    void map_sparse(F &f, VS &output_vs, uint32_t self_index, bool output) {
        sbtree_iterator it(*this, self_index);
        while (!it.done()) {
            #ifdef WEIGHTED
            auto v = (*it)->id;
            auto w = (*it)->val;
            if (f.cond(v) == 1 && f.updateAtomic(self_index, v, w) == 1) {
            #else
            auto v = (*it)->id;
            if (f.cond(v) == 1 && f.updateAtomic(self_index, v) == 1) {
            #endif

                if (output) {
                    output_vs.insert_sparse(v);
                }
            }
            ++it;
        }
    }

    template <class F, typename VS>
    void map_dense_vs_all(F &f, VS &vs, VS &output_vs, uint32_t self_index, bool output) {
        sbtree_iterator it(*this, self_index);
        while (!it.done()) {
            #ifdef WEIGHTED
            auto v = (*it)->id;
            auto w = (*it)->val;
            if (f.update(v, self_index, w) == 1) {
            #else
            auto v = (*it)->id;
            if (f.update(v, self_index) == 1) {
            #endif

                if (output) {
                    output_vs.insert_dense(self_index);
                }
            }
            if (f.cond(self_index) == 0) {
                return;
            }
            ++it;
        }
    }

    template <class F, typename VS>
    void map_dense_vs_not_all(F &f, VS &vs, VS &output_vs, uint32_t self_index, bool output) {
        sbtree_iterator it(*this, self_index);
        while (!it.done()) {
            #ifdef WEIGHTED
            auto v = (*it)->id;
            auto w = (*it)->val;
            if (vs.has_dense_no_all(v) && f.update(v, self_index, w) == 1) {
            #else
            auto v = (*it)->id;
            if (vs.has_dense_no_all(v) && f.update(v, self_index) == 1) {
            #endif
                if (output) {
                    output_vs.insert_dense(self_index);
                }
            }
            if (f.cond(self_index) == 0) {
                return;
            }
            ++it;
        }
    }

    inline node* offset_to_node(offset_t offset) {
        return (node*) (node_start + offset);
    }

    inline char* offset_to_addr(offset_t offset) {
        return (char*) (node_start + offset);
    }


    inline char* root_node_addr(index_t src_index) {
        return node_start + sizeof(node) * src_index;
        //return nvm_alloc.node_start + sizeof(node) * (src_index * NUM_BRANCH + dst_vtx_id % NUM_BRANCH);
    }


    int find_edge_g(vtxid_t src_vtx_id, vtxid_t dst_vtx_id);
    int find_edge_t(node* root, vtxid_t dst_vtx_id);


    #ifndef NO_CONCURRENT
    int insert_edge_parallel(vtxid_t src_vtx_id, vtxid_t dst_vtx_id, weight_t val = 1);
    #else
    int insert_edge(vtxid_t src_vtx_id, vtxid_t dst_vtx_id, weight_t val = 1);
    #endif

    #ifndef NO_CONCURRENT
    int delete_edge_parallel(vtxid_t src_vtx_id, vtxid_t dst_vtx_id);
    #else
    int delete_edge(vtxid_t src_vtx_id, vtxid_t dst_vtx_id);
    #endif

    degree_t get_out_degree(vtxid_t src_vtx_id);
    degree_t get_out_degree_l(vtxid_t src_index);

    degree_t get_adj_edges(vtxid_t src_vtx_id, std::vector<edge>& adj_edges);
    degree_t get_adj_edges(vtxid_t src_vtx_id, edge* adj_edges);
    
    degree_t get_adj_edges_l(vtxid_t src_index, std::vector<edge>& adj_edges);
    degree_t get_adj_edges_l(vtxid_t src_index, edge* adj_edges);
    
};


#endif // _NTGRAPH_INC_NTGRAPH_H_