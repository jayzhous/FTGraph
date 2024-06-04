#ifndef _NTGRAPH_APPS_BFS_EDGEMAP_H_
#define _NTGRAPH_APPS_BFS_EDGEMAP_H_

#include "apps/map.h"

struct BFS_F {
    int32_t* Parents;
    BFS_F(int32_t* _Parents) : Parents(_Parents) {}
    inline bool update(uint32_t s, uint32_t d) { //Update
        if(Parents[d] == -1) { Parents[d] = s; return 1; }
        else return 0;
    }
    inline bool updateAtomic(uint32_t s, uint32_t d){ //atomic version of Update
        return __sync_bool_compare_and_swap(&Parents[d],-1,s);
    }
    //cond function checks if vertex has been visited yet
    inline bool cond(uint32_t d) { return (Parents[d] == -1); } 
};

/**
 * define the frontier of BFS: the set of vertices that have been visited by runners but have yet to have runners leave them. 
 */

uint64_t BFS_with_edge_map(ntgraph &G, uint32_t src) {
    long start = src;
    long n = G.get_num_vertices();
    //creates Parents array, initialized to all -1, except for start
    int32_t* Parents = (int32_t *) malloc(n * sizeof(uint32_t));
    parallel_for(long i=0;i<n;i++) Parents[i] = -1;
    Parents[start] = start;
    VertexSubset frontier = VertexSubset(start, n); //creates initial frontier
    while (frontier.not_empty()) { //loop until frontier is empty
        // printf("frontier size  %lu\n", frontier.get_n());
        // dense only
        // edgeMap(G, frontier, BFS_F(Parents), true, INT_MAX);    
        VertexSubset next_frontier = edgeMap(G, frontier, BFS_F(Parents), true); 
        frontier.del();
        frontier = next_frontier;
    }
    frontier.del();


    std::vector<uint32_t> depths(n, UINT32_MAX);
    for (uint32_t j = 0; j < n; j++) {
        uint32_t current_depth = 0;
        int32_t current_parent = j;
        if (Parents[j] < 0) {
            continue;
        }
        while (current_parent != Parents[current_parent]) {
            current_depth += 1;
            current_parent = Parents[current_parent];
        }
        depths[j] = current_depth;
    }
    uint64_t total_depth = 0;
    // write out to file
    for (int i = 0; i < n; i++) {
        if (depths[i] != UINT32_MAX) {
            total_depth += depths[i];
        }
    }
    free(Parents);
    return total_depth;
}



#endif