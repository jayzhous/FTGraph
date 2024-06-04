#ifndef _NTGRAPH_APPS_ONE_HOP_H_
#define _NTGRAPH_APPS_ONE_HOP_H_
#include "inc/ntgraph.h"

inline degree_t query_vtx_nebrs(ntgraph& g, vtxid_t id){
    std::vector<edge> adj_edges;
    g.get_adj_edges(id, adj_edges);
    return adj_edges.size();
}

inline degree_t query_vtx_nebrs3(ntgraph& g, vtxid_t id){
    ntgraph::sbtree_iterator iter(g, id);
    degree_t count = 0;
    while (!iter.done()) {
        count += 1;
        ++iter;
    }
    return count;
}


degree_t test_one_hop(ntgraph& g, std::set<vtxid_t>& query_vtxids) {
    
    int64_t total_degree = 0;

    std::set<vtxid_t>::iterator iter;
    for(iter = query_vtxids.begin(); iter != query_vtxids.end(); iter++){
        total_degree += query_vtx_nebrs3(g, *iter); // 3
    }
    return total_degree;
}

#endif //_NTGRAPH_APPS_ONE_HOP_H_