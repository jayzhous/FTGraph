#ifndef _NTGRAPH_APPS_SSSP_H_
#define _NTGRAPH_APPS_SSSP_H_

#include "inc/ntgraph.h"
/*
sssp for directed or undirected graph
*/
#define INF 1000000

weight_t dijkstra(ntgraph& g, vtxid_t source) {
    vtxid_t n = g.get_num_vertices();
    bool is_weighted = g.is_weighted_g();
    vertex_index& vtx_index = g.get_vertex_index();


    std::vector<int> final(n, 0);
    std::vector<weight_t> dist(n, INF);
    // src_vtx_id is physical id
    final[source] = 1;

    std::vector<edge> adj_edges;
    g.get_adj_edges(source, adj_edges);

    vtxid_t vid;

    for (vtxid_t i = 0; i < adj_edges.size(); ++i) {
        vid = adj_edges[i].id;
        // vid is physical id
        #ifdef WEIGHTED
            dist[vid] = adj_edges[i].val;
        #else
            dist[vid] = 1;
        #endif
    }

    dist[source] = 0;

    vtxid_t count = 1;
    while (count <= n - 2) {
        adj_edges.clear();

        int64_t next = -1;
        weight_t d = INF;

        for (vtxid_t i = 0; i < n; i++) {
            if (final[i] == 0 && dist[i] < d) {
                d = dist[i];
                next = i;
            }
        }
        if (next == -1) break;
        final[next] = 1;
        if (vtx_index.find_vertex(next) == -1) {
            count += 1;
            continue;
        }
        g.get_adj_edges(next, adj_edges);
        weight_t val;
        vtxid_t vid;

        for (vtxid_t i = 0; i < adj_edges.size(); ++i) {

            #ifdef WEIGHTED
            val = adj_edges[i].val;
            #else
            val = 1;
            #endif

            // adj_edges[i].id is physical id
            vid = adj_edges[i].id;
            if (final[vid] == 0 && dist[next] + val < dist[vid]) {
                dist[vid] = dist[next] + val;
            }
        }
        count += 1;
    }
    
    weight_t total_dist = 0;
    for (vtxid_t i = 0; i < n; ++i) {
        if (dist[i] != INF) {
            total_dist += dist[i];
        }
    }
    return total_dist;

}
#endif