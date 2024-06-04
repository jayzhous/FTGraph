#ifndef _NTGRAPH_APPS_BFS_H_
#define _NTGRAPH_APPS_BFS_H_

#include "inc/ntgraph.h"

uint64_t test_bfs(ntgraph& g, vtxid_t source) {

    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t n = vtx_index.count_vertex();

    int32_t* status = (int32_t*) malloc(sizeof(int32_t) * n);
    for (vtxid_t i = 0; i < n; i++) status[i] = -1;

    int level = 0;
    status[vtx_index.find_vertex(source)] = level;


    std::vector<edge> adj_edges;
    vtxid_t vid;
    uint64_t count;
    do {
        count = 0;
        for (vtxid_t v = 0; v < n; v++) {
            if (status[v] != level) continue;
            g.get_adj_edges_l(v, adj_edges);
            for (uint32_t i = 0; i < adj_edges.size(); ++i) {
                vid = adj_edges[i].id;
                if (status[vid] == -1) {
                    status[vid] = level + 1; 
                    ++count;
                }
            }
            adj_edges.clear();
        }
        ++level;
    } while (count);


    uint64_t total_depth = 0;
    for (vtxid_t i = 0; i < n; i++) {
        if (status[i] != -1) {
            total_depth += status[i];
        }
    }

    free(status);
    status = nullptr;

    return total_depth;
}

uint64_t test_bfs2(ntgraph& g, vtxid_t source) {

    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t n = vtx_index.count_vertex();

    int32_t* status = (int32_t*) malloc(sizeof(int32_t) * n);
    for (vtxid_t i = 0; i < n; i++) status[i] = -1;

    int level = 0;
    status[vtx_index.find_vertex(source)] = level;


    edge* adjlist;
    vtxid_t vid;
    degree_t degree;
    degree_t nebr_count;
    uint64_t count;
    do {
        count = 0;
        for (vtxid_t v = 0; v < n; v++) {
            if (status[v] != level) continue;

            degree = vtx_index.get_degree_l(v);
            if (0 == degree) continue;

            adjlist = new edge[degree];

            nebr_count = g.get_adj_edges_l(v, adjlist);

            for (uint32_t i = 0; i < nebr_count; ++i) {
                vid = adjlist[i].id;
                if (status[vid] == -1) {
                    status[vid] = level + 1; 
                    ++count;
                }
            }
            delete [] adjlist;
        }
        ++level;
    } while (count);


    uint64_t total_depth = 0;
    for (vtxid_t i = 0; i < n; i++) {
        if (status[i] != -1) {
            total_depth += status[i];
        }
    }

    free(status);
    status = nullptr;

    return total_depth;
}

uint64_t test_bfs3(ntgraph& g, vtxid_t source) {
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t n = vtx_index.count_vertex();

    int level = 0;
    uint64_t count = 0;
    int32_t* status = (int32_t*) malloc(sizeof(int32_t) * n);
    for (vtxid_t i = 0; i < n; i++) status[i] = -1;

    status[vtx_index.find_vertex(source)] = level;

    do {
        count = 0;
        for (vtxid_t v = 0; v < n; v++) {
            if (status[v] != level) continue;
            ntgraph::sbtree_iterator iter(g, v);
            while (!iter.done()) {
                if (status[(*iter)->id] == -1) {
                    status[(*iter)->id] = level + 1; 
                    ++count;
                }
                ++iter;
            }
        }

        ++level;
    } while (count);

    uint64_t total_depth = 0;
    for (vtxid_t i = 0; i < n; i++) {
        if (status[i] != -1) {
            total_depth += status[i];
        }
    }

    free(status);
    status = nullptr;

    return total_depth;
}


uint64_t test_bfs5(ntgraph& g, vtxid_t source) {
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t n = vtx_index.count_vertex();

    int level = 0;
    uint64_t count = 0;
    int32_t* status = (int32_t*) malloc(sizeof(int32_t) * n);
    for (vtxid_t i = 0; i < n; i++) status[i] = -1;

    status[vtx_index.find_vertex(source)] = level;
    vtxid_t v_id;
    do {
        count = 0;
        for (vtxid_t v = 0; v < n; v++) {
            // v is a logical id
            if (status[v] != level) continue;
            // if (vtx_index.find_vertex(v) == -1) continue;
            ntgraph::iterator2 iter(g, v);
            while (!iter.done()) {
                if (status[(*iter)->id] == -1) {
                    status[(*iter)->id] = level + 1; 
                    ++count;
                }
                ++iter;
            }
        }

        ++level;
    } while (count);

    uint64_t total_depth = 0;
    for (vtxid_t i = 0; i < n; i++) {
        if (status[i] != -1) {
            total_depth += status[i];
        }
    }

    free(status);
    status = nullptr;

    return total_depth;
}

#endif //_NTGRAPH_APPS_BFS_H_