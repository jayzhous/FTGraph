#ifndef _NTGRAPH_APPS_PAGE_RANK_H_
#define _NTGRAPH_APPS_PAGE_RANK_H_
#include "inc/ntgraph.h"

void page_rank2(ntgraph& g, int iteration_count) {
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t v_count = vtx_index.count_vertex();

    double* rank_array;
	double* prior_rank_array;
    double* dset;

    rank_array = (double*) calloc(v_count, sizeof(double));
    prior_rank_array = (double*) calloc(v_count, sizeof(double));
    dset = (double*) calloc(v_count, sizeof(double));

    vtxid_t num_empty_vertices = 0;
    int64_t degree = 0;
    double	inv_v_count = 0.15;

    for (vtxid_t v = 0; v < v_count; ++v) {
        degree = vtx_index.get_degree_l(v);
        if (degree > 0) {
            dset[v] = 1.0 / degree;
            prior_rank_array[v] = inv_v_count;
        } else {
            dset[v] = 0;
            prior_rank_array[v] = 0;
            num_empty_vertices += 1;
        }
    }

    for (int iter_count = 0; iter_count < iteration_count; ++iter_count) {
        vtxid_t uid;
        int64_t nebr_count, local_degree;
        double rank = 0.0;

        edge* adj_edges;

        for (vtxid_t v = 0; v < v_count; v++) {

            local_degree = vtx_index.get_degree_l(v);
            if (local_degree == 0) continue;

            adj_edges = new edge[local_degree];
            nebr_count = g.get_adj_edges_l(v, adj_edges);
            
            rank = prior_rank_array[v];
            for(vtxid_t j = 0; j < nebr_count; ++j) {
                uid = adj_edges[j].id;
                rank_array[uid] += rank;
                
            }
            delete [] adj_edges;
        }

        if (iter_count != iteration_count - 1) { // last iteration_count
            for (vtxid_t v = 0; v < v_count; v++) {
                rank_array[v] = (0.15 + 0.85 * rank_array[v]) * dset[v];
                prior_rank_array[v] = 0;
            } 
        } else {
            for (vtxid_t v = 0; v < v_count; v++) {
                rank_array[v] = (0.15 + 0.85 * rank_array[v]);
                prior_rank_array[v] = 0;
            }
        }
        std::swap(prior_rank_array, rank_array);
    }

    double sum = 0.0;
    for (uint32_t i = 0; i < v_count; i++) {
        sum += prior_rank_array[i];
    }
    num_empty_vertices += g.get_num_vertices() - v_count;

    printf("page rank result (just for verification): %.4f, ", sum + num_empty_vertices * inv_v_count);

    free(rank_array);
    free(prior_rank_array);
    free(dset);
}

void page_rank3(ntgraph& g, int iteration_count) {
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t v_count = vtx_index.count_vertex();

    double* rank_array;
	double* prior_rank_array;
    double* dset;

    rank_array = (double*) calloc(v_count, sizeof(double));
    prior_rank_array = (double*) calloc(v_count, sizeof(double));
    dset = (double*) calloc(v_count, sizeof(double));

    vtxid_t num_empty_vertices = 0;
    degree_t degree = 0;
    double	inv_v_count = 0.15;

    for (vtxid_t v = 0; v < v_count; ++v) {
        degree = vtx_index.get_degree_l(v);
        if (degree > 0) {
            dset[v] = 1.0 / degree;
            prior_rank_array[v] = inv_v_count;
        } else {
            dset[v] = 0;
            prior_rank_array[v] = 0;
            num_empty_vertices += 1;
        }
    }

    for (int iter_count = 0; iter_count < iteration_count; ++iter_count) {

        degree_t nebr_count;
        double rank = 0.0;

        for (vtxid_t v = 0; v < v_count; v++) {
            nebr_count = vtx_index.get_degree_l(v);
            if (nebr_count <= 0) continue;

            rank = prior_rank_array[v];

            ntgraph::sbtree_iterator iter(g, v);
            while (!iter.done()) {
                rank_array[(*iter)->id] += rank;
                ++iter;
            }
        
        }

        if (iter_count != iteration_count - 1) { // last iteration_count
            for (vtxid_t v = 0; v < v_count; v++) {
                rank_array[v] = (0.15 + 0.85 * rank_array[v]) * dset[v];
                prior_rank_array[v] = 0;
            } 
        } else { 
            for (vtxid_t v = 0; v < v_count; v++) {
                rank_array[v] = (0.15 + 0.85 * rank_array[v]);
                prior_rank_array[v] = 0;
            }
        }
        std::swap(prior_rank_array, rank_array);
    }

    double sum = 0.0;
    for (int i = 0; i < v_count; i++) {
        sum += prior_rank_array[i];
    }
    num_empty_vertices += g.get_num_vertices() - v_count;

    printf("page rank result (just for verification): %.4f, ", sum + num_empty_vertices * inv_v_count);

    free(rank_array);
    free(prior_rank_array);
    free(dset);
}

void page_rank5(ntgraph& g, int iteration_count) {
    vertex_index& vtx_index = g.get_vertex_index();
    vtxid_t v_count = vtx_index.count_vertex();

    double* rank_array;
	double* prior_rank_array;
    double* dset;

    rank_array = (double*) calloc(v_count, sizeof(double));
    prior_rank_array = (double*) calloc(v_count, sizeof(double));
    dset = (double*) calloc(v_count, sizeof(double));


    degree_t degree = 0;
    double inv_v_count = 0.15;
    vtxid_t num_empty_vertices = 0;

    for (vtxid_t v = 0; v < v_count; ++v) {
        degree = vtx_index.get_degree_l(v);
        if (degree > 0) {
            dset[v] = 1.0 / degree;
            prior_rank_array[v] = inv_v_count;
        } else {
            dset[v] = 0;
            prior_rank_array[v] = 0;
            num_empty_vertices += 1;
        }
    }

    for (int iter_count = 0; iter_count < iteration_count; ++iter_count) {
        degree_t nebr_count;
        double rank = 0.0;
        for (vtxid_t v = 0; v < v_count; v++) {
            nebr_count = vtx_index.get_degree_l(v);
            if (nebr_count == 0) continue;
            rank = prior_rank_array[v];
            ntgraph::iterator2 iter(g, v);
            while (!iter.done()) {
                rank_array[(*iter)->id] += rank;
                ++iter;
            }
        }
        if (iter_count != iteration_count - 1) {
            for (vtxid_t v = 0; v < v_count; v++) {
                rank_array[v] = (0.15 + 0.85 * rank_array[v]) * dset[v];
                prior_rank_array[v] = 0;
            } 
        } else { 
            for (vtxid_t v = 0; v < v_count; v++) {
                rank_array[v] = (0.15 + 0.85 * rank_array[v]);
                prior_rank_array[v] = 0;
            }
        }
        std::swap(prior_rank_array, rank_array);
    }

    double sum = 0.0;
    for (int i = 0; i < v_count; i++) {
        sum += prior_rank_array[i];
    }

    num_empty_vertices += g.get_num_vertices() - v_count;

    printf("page rank result (just for verification): %.4f, ", sum + num_empty_vertices * inv_v_count);

    free(rank_array);
    free(prior_rank_array);
    free(dset);
}
#endif