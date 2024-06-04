#ifndef _NTGRAPH_APPS_DELETION_H_
#define _NTGRAPH_APPS_DELETION_H_

#include <string>
#include <iostream>
#include <vector>
#include <numeric>
#include <thread>
#include <fstream>
#include <cstring>

#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#include "inc/ntgraph.h"


inline void check_one_deletion(ntgraph &g, vtxid_t src, vtxid_t dst) {
    int i = g.find_edge_g(src, dst);
    assert(i == 0);
}

void check_batch_deletion(ntgraph &g, std::vector<vtxid_t> &src_vertices, std::vector<vtxid_t> &dst_vertices) {

    #ifndef NO_CONCURRENT
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(THREAD_NUM));
    unsigned int TASK_NUM = src_vertices.size();
    unsigned int AVG_NUM = TASK_NUM / THREAD_NUM;

    for (int i = 0; i < THREAD_NUM - 1; i++) {
        threads.emplace_back([i, AVG_NUM, &src_vertices, &dst_vertices, &g](){
            for (unsigned int j = i * AVG_NUM; j < (i + 1) * AVG_NUM; j++) {
                check_one_deletion(g, src_vertices[j], dst_vertices[j]);
            }
        });
    }
    threads.emplace_back([TASK_NUM, AVG_NUM, &src_vertices, &dst_vertices, &g](){
        for (unsigned int j = (THREAD_NUM - 1) * AVG_NUM; j < TASK_NUM; j++) {
            check_one_deletion(g, src_vertices[j], dst_vertices[j]);
        }
    });

    for (auto &t : threads) {
        t.join();
    }
    #else
    for (unsigned int i = 0; i < src_vertices.size(); i++) {
        check_one_deletion(g, src_vertices[i], dst_vertices[i]);
    }
    #endif


}


void batch_deletion(ntgraph& g, edgeid_t& num_lines, edgeid_t& total_deleted, long long& total_time, std::vector<vtxid_t>& src_vertices, std::vector<vtxid_t>& dst_vertices, std::vector<long long>& batch_times, batchid_t& batch_id, batchid_t num_batches) {

    struct timeval _start_tv, _end_tv;
    long long total_us;

    #ifndef NO_CONCURRENT
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(THREAD_NUM));
    edgeid_t TASK_NUM = num_lines;
    edgeid_t AVG_NUM = TASK_NUM / THREAD_NUM;
    gettimeofday(&_start_tv, NULL);
    for (int i = 0; i < THREAD_NUM - 1; i++) {
        threads.emplace_back([i, AVG_NUM, &src_vertices, &dst_vertices, &g](){
            for (edgeid_t j = i * AVG_NUM; j < (i + 1) * AVG_NUM; j++) {
                g.delete_edge(src_vertices[j], dst_vertices[j]);
            }
        });
    }
    threads.emplace_back([TASK_NUM, AVG_NUM, &src_vertices, &dst_vertices, &g](){
        for (edgeid_t j = (THREAD_NUM - 1) * AVG_NUM; j < TASK_NUM; j++) {
            g.delete_edge(src_vertices[j], dst_vertices[j]);
        }
    });

    for (auto &t : threads) {
        t.join();
    }
    #else
    gettimeofday(&_start_tv, NULL);
    for(edgeid_t e = 0; e < num_lines; e++) {
        g.delete_edge(src_vertices[e], dst_vertices[e]);
    }
    #endif

    gettimeofday(&_end_tv, NULL);
    total_us = (_end_tv.tv_sec - _start_tv.tv_sec) * 1000000 + (_end_tv.tv_usec - _start_tv.tv_usec);
    total_time += total_us;
    batch_times.push_back(total_us);

    
    batch_id += 1;

    #ifdef PRINT_DETAILED_INFO
    printf("the batch %d deleted %d edges, elapsed time: %lld ms, running throughput: %f M/s, %d batches left to finish.\n", batch_id, num_lines, total_us / 1000, (double) total_deleted / total_time, num_batches - batch_id);
    #endif

    #ifdef CHECK
    check_batch_deletion(g, src_vertices, dst_vertices);
    printf("batch %d verification passed.\n\n", batch_id);
    #endif

    num_lines = 0;
    src_vertices.clear();
    dst_vertices.clear();
    
}

void test_deletion_performance(ntgraph &g, std::string dataset, batchid_t num_batches, edgeid_t num_edges) {
    std::vector<long long> batch_times;

    std::vector<vtxid_t> src_vertices;
    std::vector<vtxid_t> dst_vertices;

    std::string file_name = edge_file_directory + dataset;
    std::ifstream infile(file_name, std::ifstream::in);

    if (!infile.good()) {
        std::cout << "Error: test_deletion_performance: failed to open dataset file " << file_name << " !" << std::endl;
        exit(1);
    }
    
    
    edgeid_t num_lines = 0;
    edgeid_t total_deleted = 0;
    long long total_time = 0;
    batchid_t batch_id = 0;
    bool is_weighted = g.is_weighted_g();

    vtxid_t src;
    vtxid_t dst;
    weight_t w;

    
    while (!infile.eof()) {
        if (is_weighted) {
            infile >> src >> dst >> w;
        } else {
            infile >> src >> dst;
        }
        if (infile.peek() == EOF)
            break;

        #ifdef start_from_1
        src_vertices.push_back(src - 1);
        dst_vertices.push_back(dst - 1);
        #else
        src_vertices.push_back(src);
        dst_vertices.push_back(dst);
        #endif
        
        num_lines += 1;
        total_deleted += 1;

        if (!g.is_directed_g()) {
            #ifdef start_from_1
            src_vertices.push_back(dst - 1);
            dst_vertices.push_back(src - 1);
            #else
            src_vertices.push_back(dst);
            dst_vertices.push_back(src);
            #endif
            num_lines += 1;
            total_deleted += 1;
        }

        if (num_lines >= BATCH_SIZE && batch_id != num_batches - 1) {
            batch_deletion(g, num_lines, total_deleted, total_time, src_vertices, dst_vertices, batch_times, batch_id, num_batches);
        }
    }

    batch_deletion(g, num_lines, total_deleted, total_time, src_vertices, dst_vertices, batch_times, batch_id, num_batches);
    
    long long sum_us = std::accumulate(batch_times.begin(), batch_times.end(), 0ll);
    printf("total elapsed time: %.4f s, deleted edges num: %d / %d, running throughput: %.4f M/s.\n", (double)sum_us / 1000000, total_deleted, num_edges, (double)total_deleted / sum_us);
}
#endif