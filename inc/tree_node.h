#ifndef _NTGRAPH_INC_TREE_NODE_H_
#define _NTGRAPH_INC_TREE_NODE_H_

#include <immintrin.h>

#include "apps/config.h"
#include "common/bitmap_util.h"
#include "common/hash.h"

struct edge {
    vtxid_t id;
    #ifdef WEIGHTED
    weight_t val;
    #endif
    edge() {}

    #ifdef WEIGHTED
    edge(vtxid_t _id, weight_t val): id(_id), val(_val) {}
    #else
    edge(vtxid_t _id): id(_id) {}
    #endif
    
};


struct edgelist {
    edgelist* next_el;
    uint32_t high;
    uint32_t capacity;
    edge* edges;

    inline void reset() {
        edges = nullptr;
        next_el = nullptr;
        high = 0;
        capacity = INI_EL_SIZE;
    }

    inline void set_edges(edge* e) {
        edges = e;
    }

    inline edge* get_edges() {
        return edges;
    }

    #ifdef WEIGHTED
    inline void append(vtxid_t dst, weight_t val) {
        edges[high].id = dst;
        edges[high].val = val;
        high += 1;
    }
    
    #else
    inline void append(vtxid_t dst) {
        edges[high].id = dst;
        high += 1;
    }
    #endif

    inline void append(edge e) {
        edges[high] = e;
        high += 1;
    }

    inline uint32_t get_num_edges() {
        return high;
    }

    // inline void set_next(edgelist* _next) {
    //     next_el = _next;
    // }

    inline bool is_full() {
        return high < capacity ? false : true;
    }

    void print_nebrs() {
        for (uint32_t i = 0; i < high; i++) {
            printf("%u, ", edges[i].id);
        }
        printf("size = %u\n", high);
    }
};


// tree node
struct node {
    offset_t first_child_offset = 0;            // 8B

    #ifndef NO_CONCURRENT
    level_t node_lv = 0;                        // 4B
    level_t size_lv = 0;                        // 4B
    atomic_bitset bitmap;                       // 8B
    atomic_bitset locate_map;                   // 8B
    version_lock lock;                          // 8B
    int is_offset_valid = 0;                    // 4B
    char padding[20];                           // 20B
    #else
    level_t node_lv = 0;                        // 4B
    level_t size_lv = 0;                        // 4B
    bitset bitmap;                              // 8B
    int is_offset_valid = 0;                    // 4B
    char padding[36];                           // 36B
    #endif


    unsigned char fgpt[MAX_ENTRY_NUM];         // 64B
    
    edge edges[MAX_ENTRY_NUM];

                                                
    void reset() {
        size_lv = 0;
        node_lv = 0;
        first_child_offset = 0;
        is_offset_valid = 0;
        
        #ifndef NO_CONCURRENT
        locate_map.reset();
        #endif

        bitmap.reset();
    }

    inline int level_to_size() {
        if (size_lv == 0) return 16;
        else if (size_lv == 1) return 32;
        else if (size_lv == 2) return 64;
        else {
            fprintf(stderr, "Error: unexpected size_lv (level_to_size). \n");
            exit(1);
        }
    }

    inline void set_child_offset(offset_t _offset) {
        first_child_offset = _offset;
    }

    inline edge* get_edges() {
        return (edge*) (&edges);
    }

    inline offset_t get_child_offset() {
        return first_child_offset;
    }
    inline degree_t get_num_edges() {
        return bitmap.get_num_1bit();
    }

    inline char* child_node_addr(char* node_start, vtxid_t index) {
        // 0 <= index < NUM_BRANCH
        //assert(first_child_offset != 0);
        return node_start + first_child_offset + (sizeof(node) * index);
    }

    #ifndef NO_CONCURRENT
    inline version_t get_read_lock() {
        return lock.read_lock();
    }
    inline bool release_read_lock(version_t old_version) {
        return lock.read_unlock(old_version);
    }
    inline void get_write_lock() {
        while (!lock.write_lock()) {};
    }
    inline void release_write_lock() {
        lock.write_unlock();
    }
    #endif


    int find_key(vtxid_t key) {
        unsigned char key_hash = hashcode(key);
        if (size_lv == 0) {
            __m128i fgpt_16B = _mm_loadu_si128((const __m128i*) fgpt);    //sse2
            __m128i key_16B = _mm_set1_epi8((char) key_hash);
            __m128i cmp_res = _mm_cmpeq_epi8(fgpt_16B, key_16B);
            unsigned int mask = (unsigned int) _mm_movemask_epi8(cmp_res);
            mask = mask & bitmap.to_uint16();
            while (mask) {
                int k = __builtin_ffs(mask) - 1;
                if (edges[k].id == key) {
                    return k;
                }
                mask &= ~(0x1 << k);  
            }
        } else if (size_lv == 1) {
            __m256i fgpt_32B = _mm256_loadu_si256((const __m256i*) fgpt);  //avx2
            __m256i key_32B = _mm256_set1_epi8((char) key_hash);
            __m256i cmp_res = _mm256_cmpeq_epi8(fgpt_32B, key_32B);
            unsigned int mask = (unsigned int) _mm256_movemask_epi8(cmp_res);
            mask = mask & bitmap.to_uint32();
            while (mask) {
                int k = __builtin_ffs(mask) - 1;
                if (edges[k].id == key) {
                    return k;
                }
                mask &= ~(0x1 << k);  
            }
        } else if (size_lv == 2) {
            __m512i fgpt_64B = _mm512_loadu_si512((const __m512i*) fgpt);  //avx512
            __m512i key_64B = _mm512_set1_epi8((char) key_hash);
            uint64_t mask = _mm512_cmpeq_epi8_mask(fgpt_64B, key_64B);

            mask = mask & bitmap.to_uint64();
            while (mask) {
                int k = __builtin_ffsll(mask) - 1;
                if (edges[k].id == key) {
                    return k;
                }
                mask &= ~(0x1ull << k);  
            }
            
        }
        return -1;
    }

    inline int first_zero() {
        return bitmap.first_zero();
    }

    inline int get_num_branch() {

        // if (node_lv >= 0 && node_lv <= 3) {  // >= 4 为 4 叉
        //     return 2;
        // }
        // return 4;

        // if (node_lv >= 0 && node_lv <= 4) {  // >= 5 为 4 叉
        //     return 2;
        // }
        // return 4;
        
        // if (node_lv >= 0 && node_lv <= 2) {  // >= 3 为 4 叉
        //     return 2;
        // }
        // return 4;
        

        // if (node_lv >= 0 && node_lv <= 1) { // >= 2 为 4 叉
        //     return 2;
        // }
        // return 4;


        // if (node_lv == 0) {  // >= 1 为 4 叉
        //     return 2;
        // }
        // return 4;


        // return 2; // 全部 2 叉树
        return 4; // 全部 4 叉树  // >= 0 为 4 叉
        
    }

    inline unsigned int index_branch(vtxid_t vtx_id) {
        
        // int start = 0;
        // int num_bits = 0;
        // if (node_lv >= 0 && node_lv <= 2) {
        //     start = node_lv;
        //     num_bits = 1;
        // } else if (node_lv >= 3) {
        //     start = 3 + (node_lv - 3) * 2;
        //     num_bits = 2;
        // }
        // return get_bits(vtx_id, start, num_bits);


        // int start = 0;
        // int num_bits = 0;
        // if (node_lv >= 0 && node_lv <= 3) {
        //     start = node_lv;
        //     num_bits = 1;
        // } else if (node_lv >= 4) {
        //     start = 4 + (node_lv - 4) * 2;
        //     num_bits = 2;
        // }
        // return get_bits(vtx_id, start, num_bits);


        // int start = 0;
        // int num_bits = 0;
        // if (node_lv >= 0 && node_lv <= 4) {
        //     start = node_lv;
        //     num_bits = 1;
        // } else if (node_lv >= 5) {
        //     start = 5 + (node_lv - 5) * 2;
        //     num_bits = 2;
        // }
        // return get_bits(vtx_id, start, num_bits);

        
        return get_bits(vtx_id, node_lv * 2, 2);  // 全部 4 叉树  // >= 0 为 4 叉

        
        // int start = 0;
        // int num_bits = 0;
        // if (node_lv >= 0 && node_lv <= 1) {
        //     start = node_lv;
        //     num_bits = 1;
        // } else if (node_lv >= 2) {
        //     start = 2 + (node_lv - 2) * 2;
        //     num_bits = 2;
        // }
        // return get_bits(vtx_id, start, num_bits);
        

        
        // int start = 0;
        // int num_bits = 0;
        // if (node_lv == 0) {
        //     start = node_lv;
        //     num_bits = 1;
        // } else if (node_lv >= 1) {
        //     start = 1 + (node_lv - 1) * 2;
        //     num_bits = 2;
        // }
        // return get_bits(vtx_id, start, num_bits);
        


        // return get_bits(vtx_id, node_lv, 1);  // 全部 2 叉树
    }

    char* index_branch_addr(char* node_start, vtxid_t vtx_id) {
        unsigned int index = index_branch(vtx_id);
        return first_child_offset + (sizeof(node) * index) + node_start;
    }

    #ifndef NO_CONCURRENT
    inline int locate_entry() {
        return locate_map.set_first_zero();
    }
    inline int try_locate_entry(uint64_t old_bm, bool& changed) {
        // only old_bm = locate_map, set first zero of locate_map = 1
        return locate_map.test_and_set_first_zero(old_bm, changed);
    }
    #endif

    inline void set_entry(int pos) {
        bitmap.set(pos);
    }

    inline void scale_up(level_t _lv) {
        #ifndef NO_CONCURRENT
        get_write_lock();
        if (size_lv == _lv) {
            release_write_lock();
            return;
        }
        #endif

        size_lv = _lv;

        #ifdef CLWB_SFENCE
        clwb(this);
        sfence();
        #endif


        #ifndef NO_CONCURRENT
        release_write_lock();
        #endif

    }

    bool branch_out(pmem_pool_allocator* nvm_alloc) {
        
        
        #ifndef NO_CONCURRENT
        get_write_lock();
        if (is_offset_valid != 0) {
            release_write_lock();
            return false;
        }
        #endif
        
        #ifdef SEQUENTIAL_MEM
        if (node_lv == 0) {
            
            char* first_child = (char*) nvm_alloc->alloc(sizeof(node) * 14);
            first_child_offset = first_child - nvm_alloc->get_node_start();
            
            node* child;
            for (int i = 0; i < 2; i++) {
                child = (node*) (first_child + sizeof(node) * i);
                child->node_lv = node_lv + 1;
                child->set_child_offset(first_child_offset + sizeof(node) * 2 * (i + 1));
                child->is_offset_valid = 0;
                child->size_lv = 2;

                #ifndef NO_CONCURRENT
                child->lock.reset_version();
                #endif

                #ifdef CLWB_SFENCE
                clwb((void*) child);
                sfence();
                #endif
            }

            for (int i = 0; i < 4; i++) {
                child = (node*) (first_child + sizeof(node) * (i + 2));
                child->node_lv = node_lv + 2;
                child->set_child_offset(first_child_offset + sizeof(node) * (6 + i * 2));
                child->is_offset_valid = 0;
                child->size_lv = 2;

                #ifndef NO_CONCURRENT
                child->lock.reset_version();
                #endif

                #ifdef CLWB_SFENCE
                clwb((void*) child);
                sfence();
                #endif
            }

            for (int i = 0; i < 8; i++) {
                child = (node*) (first_child + sizeof(node) * (i + 6));
                child->node_lv = node_lv + 3;
                child->is_offset_valid = 0;
                child->size_lv = 2;

                #ifndef NO_CONCURRENT
                child->lock.reset_version();
                #endif

                #ifdef CLWB_SFENCE
                clwb((void*) child);
                sfence();
                #endif
            }

            is_offset_valid = 1;
            #ifdef CLWB_SFENCE
            clwb(this);
            sfence();
            #endif

        } else if (node_lv == 1 || node_lv == 2) { // 1 <= nodel_lv <= 2
            is_offset_valid = 1;

            #ifdef CLWB_SFENCE
            clwb(this);
            sfence();
            #endif

        } else {
            
            char* first_child = (char*) nvm_alloc->alloc(sizeof(node) * 4);

            node* child;
            for (int i = 0; i < 4; i++) {
                child = (node*) (first_child + sizeof(node) * i);
                child->node_lv = node_lv + 1;
                child->is_offset_valid = 0;
                child->size_lv = 2;

                #ifndef NO_CONCURRENT
                child->lock.reset_version();
                #endif

                #ifdef CLWB_SFENCE
                clwb((void*) child);
                sfence();
                #endif
            }

            first_child_offset = first_child - nvm_alloc->get_node_start();
            is_offset_valid = 1;

            #ifdef CLWB_SFENCE
            clwb(this);
            sfence();
            #endif

        }
        #else
        int num_branch = get_num_branch();
        char* first_child = (char*) nvm_alloc->alloc(sizeof(node) * num_branch);

        node* child;
        for (int i = 0; i < num_branch; i++) {
            child = (node*) (first_child + sizeof(node) * i);
            child->node_lv = node_lv + 1;
            child->is_offset_valid = 0;
            child->size_lv = 2;

            #ifndef NO_CONCURRENT
            child->lock.reset_version();
            #endif

            #ifdef CLWB_SFENCE
            clwb((void*) child);
            sfence();
            #endif
        }

        first_child_offset = first_child - nvm_alloc->get_node_start();
        is_offset_valid = 1;

        #ifdef CLWB_SFENCE
        clwb(this);
        sfence();
        #endif

        #endif


        #ifndef NO_CONCURRENT
        release_write_lock();
        #endif

        return true;
    }

};


#endif