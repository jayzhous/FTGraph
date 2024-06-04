#ifndef _NTGRAPH_APPS_CC_H_
#define _NTGRAPH_APPS_CC_H_
#include "inc/ntgraph.h"

// used for undirected graph 

bool compare_and_swap(vtxid_t &x, vtxid_t &old_val, vtxid_t &new_val) {
    return __sync_bool_compare_and_swap(&x, old_val, new_val);
}

uint64_t test_cc2(ntgraph& g) {
    
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t nv = vtx_index.count_vertex();

    vtxid_t* components = (vtxid_t*) malloc(sizeof(vtxid_t) * nv);

    for (vtxid_t v = 0; v < nv; v++) {
        components[v] = v;
    }

    edge* adj_edges;

    degree_t degree;
    degree_t nebr_count;

    for (vtxid_t u = 0; u < nv; u++) {
        
        degree = vtx_index.get_degree_l(u);
        if (0 == degree) continue;
        adj_edges = new edge[degree];
        nebr_count = g.get_adj_edges_l(u, adj_edges);

        
        for (vtxid_t i = 0; i < nebr_count; i++) {
            vtxid_t v = adj_edges[i].id;
            // vid is physical id
            
            vtxid_t p1 = components[u];
            vtxid_t p2 = components[v];
            while (p1 != p2) {
                vtxid_t high = p1 > p2 ? p1 : p2;
                vtxid_t low = p1 + (p2 - high);
                vtxid_t p_high = components[high];
                
                if ((p_high == low) || (p_high == high && compare_and_swap(components[high], high, low)))
                    break;
                p1 = components[components[high]];
                p2 = components[low];
            }
        }

        delete [] adj_edges;
    }


    for (vtxid_t j = 0; j < nv; j++) {
        while (components[j] != components[components[j]]) {
            components[j] = components[components[j]];
        }
    }

    std::set<vtxid_t> comp_ids;
    // just for verification
    for (vtxid_t v = 0; v < nv; v++) {
        comp_ids.insert(components[v]);
    }
    uint64_t ncc = comp_ids.size();

    vtxid_t num_empty_vertices = g.get_num_vertices() - nv;

    free(components);
    components = nullptr;

    return ncc + num_empty_vertices;

}


uint64_t test_cc3(ntgraph& g) {
    
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t nv = vtx_index.count_vertex();
    vtxid_t* components = (vtxid_t*) malloc(sizeof(vtxid_t) * nv);

    for (vtxid_t v = 0; v < nv; v++) {
        components[v] = v;
    }
    edge* e;
    for (vtxid_t u = 0; u < nv; u++) {
        // u is an internal logical id
        ntgraph::sbtree_iterator iter(g, u);
        while (!iter.done()) {
            e = *iter;
            
            vtxid_t v = e->id;
            vtxid_t p1 = components[u];
            vtxid_t p2 = components[v];
            while (p1 != p2) {
                vtxid_t high = p1 > p2 ? p1 : p2;
                vtxid_t low = p1 + (p2 - high);
                vtxid_t p_high = components[high];
                if ((p_high == low) || (p_high == high && compare_and_swap(components[high], high, low)))
                    break;
                p1 = components[components[high]];
                p2 = components[low];
            }
            ++iter;
        }
    }
    for (vtxid_t j = 0; j < nv; j++) {
        while (components[j] != components[components[j]]) {
            components[j] = components[components[j]];
        }
    }

    std::set<vtxid_t> comp_ids;
    // just for verification
    for (vtxid_t v = 0; v < nv; v++) {
        comp_ids.insert(components[v]);
    }
    uint64_t ncc = comp_ids.size();

    vtxid_t num_empty_vertices = g.get_num_vertices() - nv;

    free(components);
    components = nullptr;

    return ncc + num_empty_vertices;

}

uint64_t test_cc5(ntgraph& g) {
    
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t nv = vtx_index.count_vertex();

    vtxid_t* components = (vtxid_t*) malloc(sizeof(vtxid_t) * nv);

    
    for (vtxid_t v = 0; v < nv; v++) {
        components[v] = v;
    }
    edge* e;
    for (vtxid_t u = 0; u < nv; u++) {
        // u is a logical id
        ntgraph::iterator2 iter(g, u);
        while (!iter.done()) {
            e = *iter;
            
            vtxid_t v = e->id;
            vtxid_t p1 = components[u];
            vtxid_t p2 = components[v];
            while (p1 != p2) {
                vtxid_t high = p1 > p2 ? p1 : p2;
                vtxid_t low = p1 + (p2 - high);
                vtxid_t p_high = components[high];
                if ((p_high == low) || (p_high == high && compare_and_swap(components[high], high, low)))
                    break;
                p1 = components[components[high]];
                p2 = components[low];
            }

            ++iter;
        }
    }
    for (vtxid_t j = 0; j < nv; j++) {
        while (components[j] != components[components[j]]) {
            components[j] = components[components[j]];
        }
    }

    std::set<vtxid_t> comp_ids;
    // just for verification
    for (vtxid_t v = 0; v < nv; v++) {
        comp_ids.insert(components[v]);
    }
    uint64_t ncc = comp_ids.size();

    // std::set<vtxid_t>::iterator it;
    // for(it = comp_ids.begin(); it != comp_ids.end(); it++){
    //     #ifdef start_from_1
    //     std::cout << vtx_index.indexed_vtxid(*it) + 1 << " ";
    //     #else
    //     std::cout << vtx_index.indexed_vtxid(*it) << " ";
    //     #endif
    // }

    vtxid_t num_empty_vertices = g.get_num_vertices() - nv;

    free(components);
    components = nullptr;

    return ncc + num_empty_vertices;

}


#endif