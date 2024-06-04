#ifndef _NTGRAPH_APPS_TWO_HOP_H_
#define _NTGRAPH_APPS_TWO_HOP_H_
#include "inc/ntgraph.h"


uint64_t test_two_hop(ntgraph& g, std::set<vtxid_t>& query_vtxids) {
    uint64_t total_degree = 0;
    std::set<vtxid_t>::iterator iter;
    for(iter = query_vtxids.begin(); iter != query_vtxids.end(); iter++){
        std::vector<edge> adj_edges;
        g.get_adj_edges(*iter, adj_edges);
        /*2-hop*/
        for (vtxid_t i = 0; i < adj_edges.size(); i++) {
            std::vector<edge> two_hop_adj_edges;
            g.get_adj_edges(adj_edges[i].id, two_hop_adj_edges);
            total_degree += two_hop_adj_edges.size();
        }
    }
    return total_degree;
}

uint64_t test_two_hop2(ntgraph& g, std::set<vtxid_t>& query_vtxids) {
    vertex_index& vtx_index = g.get_vertex_index();
    uint64_t total_degree = 0;

    degree_t nebr_count, nebr_count2, local_degree, local_degree2;
    edge* adj_edges;
    edge* adj_edges2;

    std::set<vtxid_t>::iterator iter;
    for(iter = query_vtxids.begin(); iter != query_vtxids.end(); iter++){

        local_degree = vtx_index.get_degree_l(*iter);
        adj_edges = new edge[local_degree];
        nebr_count = g.get_adj_edges_l(*iter, adj_edges);
        /*2-hop*/
        for (vtxid_t i = 0; i < nebr_count; i++) {
            local_degree2 = vtx_index.get_degree_l(adj_edges[i].id);
            adj_edges2 = new edge[local_degree2];
            nebr_count2 = g.get_adj_edges_l(adj_edges[i].id, adj_edges2);
            total_degree += nebr_count2;
            delete [] adj_edges2;
        }
        delete [] adj_edges;
    }
    return total_degree;
}

uint64_t test_two_hop3(ntgraph& g, std::set<vtxid_t>& query_vtxids) {
    uint64_t total_degree = 0;
    std::set<vtxid_t>::iterator iter;
    for(iter = query_vtxids.begin(); iter != query_vtxids.end(); iter++){
        ntgraph::sbtree_iterator e_iter(g, *iter);
        while (!e_iter.done()) {
            ntgraph::sbtree_iterator e_iter2(g, (*e_iter)->id);
            while (!e_iter2.done()) {
                total_degree += 1;
                ++e_iter2;
            }
            ++e_iter;
        }
    }
    return total_degree;
}



uint64_t test_two_hop5(ntgraph& g, std::set<vtxid_t>& query_vtxids) {
    vertex_index& vtx_index = g.get_vertex_index();
    uint64_t total_degree = 0;
    std::set<vtxid_t>::iterator iter;
    for(iter = query_vtxids.begin(); iter != query_vtxids.end(); iter++){
        ntgraph::iterator2 e_iter(g, *iter); 
        while (!e_iter.done()) {
            ntgraph::iterator2 e_iter2(g, (*e_iter)->id);
            while (!e_iter2.done()) {
                total_degree += 1;
                ++e_iter2;
            }
            ++e_iter;
        }
    }
    return total_degree;
}



#endif //_NTGRAPH_APPS_TWO_HOP_H_