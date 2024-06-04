cmake_minimum_required(VERSION 3.4)
project(NTGraph CXX)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

link_directories(/usr/local/lib)
link_directories(/usr/local/pcm-202311/build/lib)
#link_directories(/opt/intel/oneapi/tbb/latest/lib/intel64/gcc4.8)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mavx512bw -msse2 -mavx2 -fopenmp")
# -mavx -mavx2


set(test_print_neighbours_sources
    src/edge.cc
    test/test_print_neighbours.cc
)

set(test_edge_iterator_sources
    src/edge.cc
    test/test_edge_iterator.cc
)

set(graph_benchmark_sources
    src/edge.cc
    apps/graph_benchmark.cc
)

set(header_source
    /usr/local/include
    /usr/include
    /usr/local/pcm-202311/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    #/opt/intel/oneapi/tbb/latest/include
)



# add_executable(print_neighbours ${test_print_neighbours_sources})
# target_include_directories(print_neighbours PUBLIC ${header_source})
# target_link_libraries(print_neighbours -pthread -ltbb)

# add_executable(edge_iterator ${test_edge_iterator_sources})
# target_include_directories(edge_iterator PUBLIC ${header_source})
# target_link_libraries(edge_iterator -pthread -ltbb)

add_executable(graph_benchmark ${graph_benchmark_sources})
target_include_directories(graph_benchmark PUBLIC ${header_source})
target_link_libraries(graph_benchmark -pthread -ltbb -lpcm)