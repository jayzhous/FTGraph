#ifndef _NTGRAPH_APPS_DEGREE_H_
#define _NTGRAPH_APPS_DEGREE_H_

#include "inc/ntgraph.h"

void avg_degree(ntgraph& g) {
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t nv = g.get_num_vertices();

    degree_t total_degree = 0;
    degree_t max_degree = 0;
    vtxid_t num_non_empty = 0;
    vtxid_t num_empty = 0;
    int gap = 16;
    std::map<vtxid_t, vtxid_t> degrees;

    for (vtxid_t u = 0; u < nv; u++) {
        // u is physical id
        
        if (vtx_index.find_vertex(u) == -1) {
            num_empty += 1;
            continue;
        } 
        // the vertex u doesn't exist

        degree_t out_degree = vtx_index.get_degree(u);
        max_degree = max_degree > out_degree ? max_degree : out_degree;
        if (out_degree > 0) {
            num_non_empty += 1;
            total_degree += out_degree;
        } else if (out_degree == 0) {
            num_empty += 1;
            continue;
        }

        if (degrees.count((out_degree - 1) / gap) == 0) {
            degrees.insert(std::pair<vtxid_t, vtxid_t>((out_degree - 1) / gap, 1));
        } else {
            degrees[(out_degree - 1) / gap] += 1;
        }

    }
    printf("max degree = %d \n", max_degree);
    printf("non-empty vertices' average degree = %d \n", total_degree / num_non_empty);
    printf("all vertices' average degree = %d \n", total_degree / nv);
    printf("num_empty = %d, num_empty + num_non_empty = %d, nv = %d \n", num_empty, num_non_empty + num_empty, nv);
    std::map<vtxid_t, vtxid_t>::iterator it;
    vtxid_t count = num_empty;

    int print_count = 0;

    for (it = degrees.begin(); it != degrees.end(); it++) {
        count += it->second;
        printf("partion [%d ~ %d] : %.4f%% --- [%d ~ %d] : %d\n", 1, (it->first + 1) * gap, (double) count / nv * 100, it->first * gap + 1, (it->first + 1) * gap, it->second);
        
        print_count += 1;
        if (print_count == 100) 
            break;
    }
}
#endif