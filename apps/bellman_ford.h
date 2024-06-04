#ifndef _NTGRAPH_APPS_BELLMAN_FORD_H_
#define _NTGRAPH_APPS_BELLMAN_FORD_H_

#include <omp.h>

#include "common/cas_util.h"
#include "inc/ntgraph.h"

#define INF 1000000

int64_t bellman_ford2(ntgraph& g, bool *has_negative_cycle, vtxid_t source) {
    
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t n = vtx_index.count_vertex();

    weight_t* dist = (weight_t*) malloc(sizeof(weight_t) * n);
    *has_negative_cycle = false;
    for (vtxid_t i = 0; i < n; i++) {
        dist[i] = INF;
    }

    dist[vtx_index.find_vertex(source)] = 0;

    bool has_change;

    weight_t weight;
    vtxid_t vid;

    edge* adj_edges;

    degree_t degree;
    degree_t nebr_count;


    for (int i = 0; i < n - 1; i++) {
        has_change = false;
        for (vtxid_t u = 0; u < n; u++) { 
            
            degree = vtx_index.get_degree_l(u);
            if (0 == degree) continue;
            adj_edges = new edge[degree];
            nebr_count = g.get_adj_edges_l(u, adj_edges);

            for (vtxid_t v = 0; v < nebr_count; ++v) {
                #ifdef WEIGHTED
                    weight = adj_edges[v].val;
                #else
                    weight = 1;
                #endif

                vid = adj_edges[v].id;
                if (dist[u] + weight < dist[vid]) {
                    has_change = true;
                    dist[vid] = dist[u] + weight;
                }
            }
            delete [] adj_edges;
        }
        if(!has_change) {
            break;
        }
    }

    if (has_change) {
        for (vtxid_t u = 0; u < n; u++) {
            
            degree = vtx_index.get_degree_l(u);
            if (0 == degree) continue;
            adj_edges = new edge[degree];
            nebr_count = g.get_adj_edges_l(u, adj_edges);

            for (vtxid_t v = 0; v < nebr_count; ++v) {
                #ifdef WEIGHTED
                    weight = adj_edges[v].val;
                #else
                    weight = 1;
                #endif
                vid = adj_edges[v].id; 
                if (dist[u] + weight < dist[vid]) {
                    *has_negative_cycle = true;
                    break;
                }
            }
        }
    }
    int64_t total_dist = 0;
    for (vtxid_t i = 0; i < n; ++i) {
        if (dist[i] != INF) {
            total_dist += dist[i];
        }
    }
    free(dist);
    dist = nullptr;

    // just for verification
    return *has_negative_cycle ? -1 : total_dist;
}

int64_t bellman_ford3(ntgraph& g, bool *has_negative_cycle, vtxid_t source) {
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t n = vtx_index.count_vertex();

    weight_t* dist = (weight_t*) malloc(sizeof(weight_t) * n);
    *has_negative_cycle = false;
    for (vtxid_t i = 0; i < n; i++) {
        dist[i] = INF;
    }
    //root vertex always has distance 0
    dist[vtx_index.find_vertex(source)] = 0;
    //a flag to record if there is any distance change in this iteration
    bool has_change;
    weight_t weight;
    //bellman-ford edge relaxation
    for (int i = 0; i < n - 1; i++) {// n - 1 iteration
        has_change = false;
        for (vtxid_t u = 0; u < n; u++) {  // u is external vertex id

            ntgraph::sbtree_iterator iter(g, u);
            while (!iter.done()) {
                #ifdef WEIGHTED
                    weight = e->val;
                #else
                    weight = 1;
                #endif
                
                if (dist[u] + weight < dist[(*iter)->id]) {
                    has_change = true;
                    dist[(*iter)->id] = dist[u] + weight;
                }
                ++iter;
            }
        }
        //if there is no change in this iteration, then we have finished
        if(!has_change) {
            break;
        }
    }
    //do one more iteration to check negative cycles
    if (has_change) {
        for (vtxid_t u = 0; u < n; u++) {

            ntgraph::sbtree_iterator iter(g, u);
            while (!iter.done()) {
                #ifdef WEIGHTED
                    weight = e->val;
                #else
                    weight = 1;
                #endif
                
                if (dist[u] + weight < dist[(*iter)->id]) {
                    has_change = true;
                    dist[(*iter)->id] = dist[u] + weight;
                }
                ++iter;
            }
        }
    }
    
    int64_t total_dist = 0;
    for (vtxid_t i = 0; i < n; ++i) {
        if (dist[i] != INF) {
            total_dist += dist[i];
        }
    }

    free(dist);
    dist = nullptr;

    // just for verification
    return *has_negative_cycle ? -1 : total_dist;
}

int64_t bellman_ford5(ntgraph& g, bool *has_negative_cycle, vtxid_t source) {
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t n = vtx_index.count_vertex();

    weight_t* dist = (weight_t*) malloc(sizeof(weight_t) * n);
    *has_negative_cycle = false;
    for (vtxid_t i = 0; i < n; i++) {
        dist[i] = INF;
    }
    //root vertex always has distance 0
    dist[vtx_index.find_vertex(source)] = 0;
    //a flag to record if there is any distance change in this iteration
    bool has_change;
    weight_t weight;
    //bellman-ford edge relaxation
    for (uint32_t i = 0; i < n - 1; i++) {// n - 1 iteration
        has_change = false;
        for (vtxid_t u = 0; u < n; u++) {  // u is logical vertex id
            ntgraph::iterator2 iter(g, u);
            while (!iter.done()) {
                #ifdef WEIGHTED
                    weight = e->val;
                #else
                    weight = 1;
                #endif
                if (dist[u] + weight < dist[(*iter)->id]) {
                    has_change = true;
                    dist[(*iter)->id] = dist[u] + weight;
                }
                ++iter;
            }
        }
        //if there is no change in this iteration, then we have finished
        if(!has_change) {
            break;
        }
    }
    //do one more iteration to check negative cycles
    if (has_change) {
        for (vtxid_t u = 0; u < n; u++) {
            ntgraph::iterator2 iter(g, u);
            while (!iter.done()) {
                #ifdef WEIGHTED
                    weight = e->val;
                #else
                    weight = 1;
                #endif
                if (dist[u] + weight < dist[(*iter)->id]) {
                    has_change = true;
                    dist[(*iter)->id] = dist[u] + weight;
                }
                ++iter;
            }
        }
    }
    int64_t total_dist = 0;
    for (vtxid_t i = 0; i < n; ++i) {
        if (dist[i] != INF) {
            total_dist += dist[i];
        }
    }

    free(dist);
    dist = nullptr;

    // just for verification
    return *has_negative_cycle ? -1 : total_dist;
}

#endif