#ifndef _INC_NTGRAPH_BTREE_H_
#define _INC_NTGRAPH_BTREE_H_

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

#include "common/btree.h"
#include "common/persistent_instructions.h"

struct vertex {  // sizeof = 32
    vtxid_t id;
    degree_t degree;
    uint32_t num_nodes;
    offset_t btree_root;

    vertex (vtxid_t id_) {
        id = id_;
    }

    inline void add_degree(int val = 1) {
        degree += val;
    }
    inline degree_t get_degree() {
        return degree;
    }
    inline uint32_t get_num_nodes() {
        return num_nodes;
    }
    inline void add_num_nodes(uint32_t val) {
        num_nodes += val;
    }
    inline void set_id(vtxid_t id_) {
        id = id_;
    }
    inline vtxid_t get_id() {
        return id;
    }
}; 

class vertex_index {
private:
    manage_meta_data* meta_data;
    vtxid_t high;
    vtxid_t num_vertices;
    vertex* vertices;
    std::vector<index_t> p_t_l_mapper;
public:
    void init(manage_meta_data* _meta_data, vtxid_t _num_vertices, vertex* _vertices, vtxid_t _high = 0) {
        meta_data = _meta_data;
        num_vertices = _num_vertices;
        vertices = _vertices;
        high = _high;

        p_t_l_mapper = std::vector<index_t>(num_vertices, -1);
    }

    inline vtxid_t get_num_vertices() {
        return num_vertices;
    }

    inline vtxid_t get_num_physical_vertices() {
        return p_t_l_mapper.size();
    }

    // return degree with physical id
    inline degree_t get_degree(vtxid_t p_id) {
        index_t index = p_t_l_mapper[p_id];
        if (index < 0) return 0;
        else return vertices[index].get_degree();
    }

    // return degree with logical id
    inline degree_t get_degree_l(vtxid_t index) {
        return vertices[index].get_degree();
    }

    inline uint32_t get_num_nodes(vtxid_t p_id) {
        index_t index = p_t_l_mapper[p_id];
        if (index < 0) return 0;
        else return vertices[index].get_num_nodes();
    }

    inline uint32_t get_num_nodes_l(vtxid_t index) {
        return vertices[index].get_num_nodes();
    }

    inline vtxid_t count_vertex() {
        return high;
    }

    inline void set_high(vtxid_t _high) {
        high = _high;
    }

    inline vertex* get_vertex_ptr() {
        return vertices;
    }

    void re_map() {
        for (unsigned int i = 0; i < high; i++) {
            p_t_l_mapper[vertices[i].get_id()] = i;
        }
    }

    inline index_t find_vertex(vtxid_t vertex_id) {
        return p_t_l_mapper[vertex_id];
    }

    // return real/physical id according to a logical id
    inline vtxid_t indexed_vtxid(index_t index) {
        assert(index >= 0);
        return vertices[index].get_id();
    }
    // return a vertex according to a logical id
    inline vertex* indexed_vertex(index_t index) {
        if (index >= 0) {
            return &vertices[index];
        } else {
            return nullptr;
        }
    }
    // return a vertex according to a external physical id
    inline vertex* get_vertex(vtxid_t vertex_id) {
        index_t logical_id = find_vertex(vertex_id);
        if (logical_id != -1) {
            return &vertices[logical_id];
        } else {
            return nullptr;
        }
    }
    inline vtxid_t alloc_entry() {
        return high++;
    }

    // return internal logical vertex id
    // deleting an vertex is not allowed, so when the variable count is persisted, the vertex inserted is also persisted
    vtxid_t insert_vertex(vtxid_t vertex_id) {
        index_t k = find_vertex(vertex_id);
        if (k == -1) {
            vtxid_t high = alloc_entry();
            if (high < num_vertices) {
                vertices[high].set_id(vertex_id);
                vertices[high].num_nodes = 0;
                vertices[high].btree_root = 0;

                #ifdef PERSISTED
                int num_vertices_per_line = CACHE_LINE_SIZE / sizeof(vertex);
                clwb((void*) (vertices + high / num_vertices_per_line * num_vertices_per_line));
                sfence();
                #endif

                p_t_l_mapper[vertex_id] = high;
                meta_data->set_count(high);
                return high;
            } else {
                fprintf(stderr, "Error: the vertex array compaction is needed.\n");
                exit(1);
            }
        }
        return k;
    }
    // return vertex pointer
    vertex* insert_vertex_p(vtxid_t vertex_id) {
        index_t k = find_vertex(vertex_id);
        if (k == -1) {
            vtxid_t high = alloc_entry();
            if (high < num_vertices) {
                vertices[high].set_id(vertex_id);
                vertices[high].num_nodes = 0;
                vertices[high].btree_root = 0;

                #ifdef PERSISTED
                int num_vertices_per_line = CACHE_LINE_SIZE / sizeof(vertex);
                clwb((void*) (vertices + high / num_vertices_per_line * num_vertices_per_line));
                sfence();
                #endif

                p_t_l_mapper[vertex_id] = high;
                meta_data->set_count(high);
                return &vertices[high];
            } else {
                fprintf(stderr, "Error: the number of vertices has reached %d and no more vertices can be added.\n", num_vertices);
                exit(1);
            }
        }
        return &vertices[k];
    }
};

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

            char* padding = (char*) nvm_alloc->alloc(sizeof(node));

            
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

        nvm_alloc->print_usage();
        delete nvm_alloc;
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

    const node* find(offset_t root_offset, vtxid_t id) {
        return (root_offset == 0) ? nullptr : offset_to_node(root_offset)->find(id);
    }

    int find_edge_g(vtxid_t src_vtx_id, vtxid_t dst_vtx_id) {
        index_t src_index = vtx_index.find_vertex(src_vtx_id);
        index_t dst_index = vtx_index.find_vertex(dst_vtx_id);
        if (src_index == -1 || dst_index == -1) {
            return 0;
        }

        vertex* src_vertex = vtx_index.indexed_vertex(src_index);
        const node* n = find(src_vertex->btree_root, dst_index);
        if (n == nullptr) return 0;
        return 1;
    }

    int insert_edge(vtxid_t src_vtx_id, vtxid_t dst_vtx_id, weight_t w = 1) {
        vtxid_t dst_index = vtx_index.insert_vertex(dst_vtx_id);
        vtxid_t src_index = vtx_index.insert_vertex(src_vtx_id);

        vertex* src_vertex = vtx_index.indexed_vertex(src_index);

        if (find(src_vertex->btree_root, dst_index) != nullptr) {
            return 0;
        }

        if (src_vertex->btree_root == 0) {

            char* child_addr = (char*) nvm_alloc->alloc(sizeof(node));
            offset_t _offset = child_addr - node_start;

            src_vertex->btree_root = _offset;

            node* root = (node*) child_addr;
            root->init(true);

            root->keys[0] = dst_index;
            root->offset = _offset;

#ifdef WEIGHTED
            root->weights[0] = w;
#endif

            root->num_keys = 1;
            return 1;
        } else {  
            if (offset_to_node(src_vertex->btree_root)->num_keys == MAX_KEYS) {
            
                char* child_addr = (char*) nvm_alloc->alloc(sizeof(node));
                offset_t _offset = child_addr - node_start;
                
                node* s = (node*) child_addr;
                
                s->init(false);
                s->children[0] = src_vertex->btree_root;
                s->offset = _offset;
                s->splitChild(0, offset_to_node(src_vertex->btree_root));

                
                int i = 0;
                if (s->keys[0] < dst_index)
                    i++;

                src_vertex->btree_root = _offset;
#ifdef WEIGHTED
                return offset_to_node(s->children[i])->insertNonFull(dst_index, w);
#else
                return offset_to_node(s->children[i])->insertNonFull(dst_index);
#endif
            }
            else  
#ifdef WEIGHTED
                return offset_to_node(src_vertex->btree_root)->insertNonFull(dst_index, w);
#else
                return offset_to_node(src_vertex->btree_root)->insertNonFull(dst_index);
#endif
        }

    }


    void traverse(vtxid_t src_vtx_id) {
        vtxid_t src_index = vtx_index.insert_vertex(src_vtx_id);
        vertex* src_vertex = vtx_index.indexed_vertex(src_index);

        if (src_vertex->btree_root != 0) offset_to_node(src_vertex->btree_root)->traverse();
    }

    uint64_t sum(vtxid_t src_vtx_id) {
        vtxid_t src_index = vtx_index.insert_vertex(src_vtx_id);
        vertex* src_vertex = vtx_index.indexed_vertex(src_index);

        if (src_vertex->btree_root != 0)
            return offset_to_node(src_vertex->btree_root)->sum();
        return 0;
    }

    uint32_t get_num_nodes(vtxid_t src_vtx_id) {
        vtxid_t src_index = vtx_index.insert_vertex(src_vtx_id);
        vertex* src_vertex = vtx_index.indexed_vertex(src_index);

        if (src_vertex->btree_root != 0) return offset_to_node(src_vertex->btree_root)->get_num_nodes();
        return 0;
    }

    const node* get_root(vtxid_t src_vtx_id) {
        vtxid_t src_index = vtx_index.insert_vertex(src_vtx_id);
        vertex* src_vertex = vtx_index.indexed_vertex(src_index);
        return offset_to_node(src_vertex->btree_root);
    }


    class btree_iterator {
    public:
        btree_iterator() {};
        btree_iterator(offset_t btree_root) {
            assert(btree_root != 0);
            it = offset_to_node(btree_root)->begin();
        }

#ifdef WEIGHTED
        std::pair<vtxid_t, weight_t> operator*(void) { return *it; }
#else
        vtxid_t operator*(void) { return *it; }
#endif
        void operator++(void) { ++it; }
        bool done(void) { return it.done(); }

    private:
        node::node_iterator it;
    };



};


#endif