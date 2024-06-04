#ifndef _NTGRAPH_TOOLS_EDGE_FILE_COUNTER_H_
#define _NTGRAPH_TOOLS_EDGE_FILE_COUNTER_H_

#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <set>
#include <unordered_set>
#include <vector>
#include <climits>

#include <stdio.h>

uint32_t max(uint32_t a, uint32_t b) {return a > b ? a : b;}
uint32_t min(uint32_t a, uint32_t b) {return a > b ? b : a;}

std::vector<uint32_t> countup_vertices_info(std::string file_name, const char* splitting_char) {
    std::ifstream ifs;
    ifs.open(file_name, std::ios::in);
    if (!ifs.is_open()) {
        std::cout << "Error: failed to open file " << file_name << std::endl;
        exit(1);
    } else {
        std::cout << "Start reading dataset " << file_name << " ..." << std::endl;
    }
    std::vector<uint32_t> vertices_info;
    uint32_t min_vertex_id = INT_MAX;
    uint32_t max_vertex_id = 0;
    uint32_t edge[3] = {0, 0, 0};
    char *ptr;
    std::string line;
    std::unordered_set<uint32_t> vertices;
    std::unordered_set<std::string> unique_edges;
    uint32_t num_lines = 0;
    uint32_t self_to_self_lines = 0;
    while (getline(ifs, line)) {
        if (line == "" || line == "\n") {
            continue;
        }
        int i = 0;
        num_lines += 1;
        ptr = strtok((char*)line.data(), splitting_char);
        while(ptr != NULL){
            edge[i] = std::stoul(ptr);
            ptr = strtok(NULL, splitting_char);
            i++;
        }
        vertices.insert(edge[0]);
        vertices.insert(edge[1]);
        if (edge[0] == edge[1]) {
            self_to_self_lines += 1;
        }
        std::string str_edge = std::to_string(edge[0]) + "-" + std::to_string(edge[1]);
        unique_edges.insert(str_edge);
        if (max(edge[0], edge[1]) > max_vertex_id) max_vertex_id = max(edge[0], edge[1]);
        if (min(edge[0], edge[1]) < min_vertex_id) min_vertex_id = min(edge[0], edge[1]);
    }
    ifs.close();
    vertices_info.push_back(vertices.size());
    vertices_info.push_back(min_vertex_id);
    vertices_info.push_back(max_vertex_id);
    vertices_info.push_back(num_lines);
    vertices_info.push_back(unique_edges.size());
    vertices_info.push_back(self_to_self_lines);

    return vertices_info;
}

std::vector<uint32_t> countup_vertices_info(std::string file_name, bool weighted) {
    std::ifstream infile(file_name, std::ifstream::in);
    if (!infile.good()) {
        std::cout << "Error: failed to open dataset file " << file_name << "! (countup_vertices_info)" << std::endl;
        exit(1);
    }
    std::vector<uint32_t> vertices_info;
    uint32_t min_vertex_id = INT_MAX;
    uint32_t max_vertex_id = 0;
    uint32_t src, dst, weight;

    std::unordered_set<uint32_t> vertices;
    std::unordered_set<std::string> unique_edges;
    uint32_t num_lines = 0;
    uint32_t self_to_self_lines = 0;

    while (!infile.eof()) {
        if (weighted) {
            infile >> src >> dst >> weight;
        } else {
            infile >> src >> dst;
        }
        if (infile.peek() == EOF)
            break;

        num_lines += 1;
        // vertices.insert(src);
        // vertices.insert(dst);
        if (src == dst) {
            self_to_self_lines += 1;
        }
        std::string str_edge = std::to_string(src) + "-" + std::to_string(dst);
        unique_edges.insert(str_edge);

        if (max(src, dst) > max_vertex_id) max_vertex_id = max(src, dst);
        if (min(src, dst) < min_vertex_id) min_vertex_id = min(src, dst);

    }

    vertices_info.push_back(vertices.size());
    vertices_info.push_back(min_vertex_id);
    vertices_info.push_back(max_vertex_id);
    vertices_info.push_back(num_lines);
    vertices_info.push_back(unique_edges.size());
    vertices_info.push_back(self_to_self_lines);

    return vertices_info;
}


#endif //_NTGRAPH_TOOLS_EDGE_FILE_COUNTER_H_