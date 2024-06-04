#ifndef _NTGRAPH_INC_VERTEX_INDEX_H_
#define _NTGRAPH_INC_VERTEX_INDEX_H_

#include <atomic>
#include <vector>
#include <sys/mman.h>

#include "inc/tree_node.h"

inline edge* huge_page_alloc(uint64_t size) {
    #ifndef HUGE_PAGE
    edge* edges = (edge*) mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0 );
    #else
    edge* edges = (edge*) mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB, -1, 0 );
    if (MAP_FAILED == edges) {
        printf("Huge page alloc failed for edge list.\n");
        exit(1);
    }
    #endif
    return edges;
}

struct vertex {  // sizeof = 32
    vtxid_t id;
    #ifdef NO_CONCURRENT
    degree_t degree;
    uint32_t num_nodes;
    #else
    std::atomic<degree_t> degree{0};
    std::atomic<uint32_t> num_nodes{0};
    #endif
    
    edgelist* first_el;
    edgelist* cur_el;

    vertex (vtxid_t id_) {
        id = id_;
    }

    inline void reset() {
        first_el = nullptr;
        cur_el = nullptr;
    }

    inline void add_degree(int val = 1) {
        #ifdef NO_CONCURRENT
        degree += val;
        #else
        degree.fetch_add(val);
        #endif
        // needn't persist the degree, it can be recalculated after a crash
    }
    inline degree_t get_degree() {
        return degree;
    }
    inline uint32_t get_num_nodes() {
        return num_nodes;
    }
    inline void add_num_nodes(uint32_t val) {
        #ifdef NO_CONCURRENT
        num_nodes += val;
        #else
        num_nodes.fetch_add(val);
        #endif
        // needn't persist the num_nodes, it can be recalculated after a crash
    }

    inline void set_id(vtxid_t id_) {
        id = id_;
    }
    inline vtxid_t get_id() {
        return id;
    }

    void insert_el(vtxid_t dst, weight_t val = 1) {
        if (cur_el == nullptr) {
            cur_el = (edgelist*) malloc(sizeof(edgelist));
            first_el = cur_el;
            cur_el->reset();
            // assert(cur_el->high == 0);

            edge* edges = huge_page_alloc(sizeof(edge) * cur_el->capacity);

            cur_el->set_edges(edges);
        }
        
        edgelist* el = cur_el;
        if (!el->is_full()) {

            #ifdef WEIGHTED
            el->append(dst, val);
            #else
            el->append(dst);
            #endif
            
        } else if (el->capacity < MAX_EL_SIZE) {
            el->capacity *= 2;

            edge* edges = huge_page_alloc(sizeof(edge) * el->capacity);

            // memory copy
            memcpy(edges, el->edges, sizeof(edge) * el->high);

            munmap(el->edges, sizeof(edge) * el->high);

            el->set_edges(edges);

            #ifdef WEIGHTED
            el->append(dst, val);
            #else
            el->append(dst);
            #endif

        } else {
            edgelist* new_el = (edgelist*) malloc(sizeof(edgelist));
            new_el->reset();
            edge* edges = huge_page_alloc(sizeof(edge) * new_el->capacity);
            new_el->set_edges(edges);

            #ifdef WEIGHTED
            new_el->append(dst, val);
            #else
            new_el->append(dst);
            #endif

            cur_el->next_el = new_el;
            cur_el = new_el;
        }
    }

    void insert_el(edge e) {
        if (cur_el == nullptr) {
            cur_el = (edgelist*) malloc(sizeof(edgelist));
            first_el = cur_el;
            cur_el->reset();

            edge* edges = huge_page_alloc(sizeof(edge) * cur_el->capacity);
            cur_el->set_edges(edges);
        }
        
        edgelist* el = cur_el;
        if (!el->is_full()) {
            el->append(e);
        } else if (el->capacity < MAX_EL_SIZE) {
            el->capacity *= 2;
            edge* edges = huge_page_alloc(sizeof(edge) * el->capacity);
            // memory copy
            memcpy(edges, el->edges, sizeof(edge) * el->high);

            munmap(el->edges, sizeof(edge) * el->high);

            el->set_edges(edges);
            el->append(e);
        } else {
            edgelist* new_el = (edgelist*) malloc(sizeof(edgelist));
            new_el->reset();

            edge* edges = huge_page_alloc(sizeof(edge) * new_el->capacity);
            new_el->set_edges(edges);
            new_el->append(e);
            // el->set_next(new_el);
            cur_el->next_el = new_el;
            cur_el = new_el;
        }
    }

    void print_el() {
        edgelist* cur = first_el;
        printf("edgelist print info of vertex %u: \n", id);
        while (cur != nullptr) {
            for (uint32_t i = 0; i < cur->get_num_edges(); i++) {
                printf("%u ", cur->edges[i].id);
            }
            cur = cur->next_el;
        }
        printf("\n");
    }
}; 

class vertex_index {
private:
    manage_meta_data* meta_data;

    #ifdef NO_CONCURRENT
    vtxid_t high;
    #else
    std::atomic<vtxid_t> high{0};
    #endif

    vtxid_t num_vertices;
    // vertex array stored on NVM
    vertex* vertices;

    #ifdef NO_CONCURRENT
    // external physical vertex id to internal logical id, stored on DRAM, 
    std::vector<index_t> p_t_l_mapper;
    #else
    tbb::concurrent_vector<index_t> p_t_l_mapper;
    std::vector<std::atomic_flag> flags;
    #endif

public:
    void init(manage_meta_data* _meta_data, vtxid_t _num_vertices, vertex* _vertices, vtxid_t _high = 0) {
        meta_data = _meta_data;
        num_vertices = _num_vertices;
        vertices = _vertices;
        high = _high;

        #ifdef NO_CONCURRENT
        p_t_l_mapper = std::vector<index_t>(num_vertices, -1);
        #else
        p_t_l_mapper = tbb::concurrent_vector<index_t>(num_vertices, -1);
        flags = std::vector<std::atomic_flag>(num_vertices);
        #endif
    }

    #ifndef NO_CONCURRENT
    // lock with physical id
    inline void lock(vtxid_t vertex_id) {
        while (flags[vertex_id].test_and_set(std::memory_order_acquire));
    }
    inline void unlock(vtxid_t vertex_id) {
        flags[vertex_id].clear(std::memory_order_release);
    }
    #endif

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
        #ifdef NO_CONCURRENT
        high = _high;
        #else
        high.store(_high);
        #endif
    }

    inline vertex* get_vertex_ptr() {
        return vertices;
    }

    void re_map() {
        for (unsigned int i = 0; i < high; i++) {
            vertices[i].reset();
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
        #ifdef NO_CONCURRENT
        return high++;
        #else
        return high.fetch_add(1);
        #endif
    }

    uint64_t get_num_elbytes() {
        edgelist* cur;
        uint64_t ne = 0;
        for (vtxid_t i = 0; i < num_vertices; i++) {
            index_t id = p_t_l_mapper[i];
            if (id < 0) continue;
            cur = vertices[id].first_el;
            while (cur != nullptr) {
                ne += cur->capacity;
                cur = cur->next_el;
            }
        }
        return ne * sizeof(edge);
    }

    void del_edgelist() {
        edgelist* next = nullptr;
        edgelist* cur;
        for (vtxid_t i = 0; i < num_vertices; i++) {
            index_t id = p_t_l_mapper[i];
            if (id < 0) continue;
            cur = vertices[id].first_el;
            
            while (cur != nullptr) {
                next = cur->next_el;

                munmap(cur->edges, sizeof(edge) * cur->capacity);
                free(cur);
                cur = next;
            }
        }
    }

    #ifdef NO_CONCURRENT
    // return internal logical vertex id
    // deleting an vertex is not allowed, so when the variable count is persisted, the vertex inserted is also persisted
    vtxid_t insert_vertex(vtxid_t vertex_id) {
        index_t k = find_vertex(vertex_id);
        if (k == -1) {
            vtxid_t high = alloc_entry();
            if (high < num_vertices) {
                vertices[high].set_id(vertex_id);
                vertices[high].num_nodes = 1;

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
                vertices[high].num_nodes = 1;

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
    #else
    vtxid_t insert_vertex_parallel(vtxid_t vertex_id) {
        // return logical id
        index_t k = find_vertex(vertex_id);

        if (k == -1) {
            //vertex_array_ptr->lock(&va_mtx);
            lock(vertex_id);
            index_t i = find_vertex(vertex_id);
            //lock the vertex
            if (i == -1) {
                vtxid_t high = alloc_entry();
                if (high >= num_vertices) {
                    unlock(vertex_id);
                    fprintf(stderr, "Error: the number of vertices has reached %d and no more vertices can be added.\n", num_vertices);
                    exit(1);
                }
                p_t_l_mapper[vertex_id] = high;
                unlock(vertex_id);
                vertices[high].set_id(vertex_id);
                vertices[high].num_nodes = 1;

                meta_data->set_count(high);

                return high;
            } else {
                unlock(vertex_id);
                return i;
            }
        }
        return k;
    }
    #endif
};


#endif // _NTGRAPH_INC_VERTEX_INDEX_H_