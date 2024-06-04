#include "tools/edge_file_counter.h"

int main() {


    //std::string file_name = "/home/sungan/dataset/hollywood-2009.mtx";
    //std::string file_name = "/home/sungan/dataset/twitter_rv.net";
    // std::string file_name = "/home/sungan/dataset/graph500-26/graph500-26.e";
    std::string file_name = "/home/sungan/dataset/com-orkut.ungraph.txt";

    
    std::vector<uint32_t> vertices_info = countup_vertices_info(file_name, false);
    std::cout << "num of real vertices = " << vertices_info[0] << std::endl;
    std::cout << "min vertex id = " << vertices_info[1] << std::endl;
    std::cout << "max vertex id = " << vertices_info[2] << std::endl;
    std::cout << "num_lines = " << vertices_info[3] << std::endl;
    std::cout << "unique_lines = " << vertices_info[4] << std::endl;
    std::cout << "self_to_self lines = " << vertices_info[5] << std::endl;
    return 0;
}
//g++ test_edge_file_counter.cc -I/mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.5/