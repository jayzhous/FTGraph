// This code is part of the project "Ligra: A Lightweight Graph Processing
// Framework for Shared Memory", presented at Principles and Practice of 
// Parallel Programming, 2013.
// Copyright (c) 2013 Julian Shun and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "apps/map.h"
#include "common/cas_util.h"

struct BF_F {
    int32_t* ShortestPathLen;
    int* Visited;
    BF_F(int32_t* _ShortestPathLen, int* _Visited): ShortestPathLen(_ShortestPathLen), Visited(_Visited) {}
    inline bool update (uint32_t s, uint32_t d) { //Update ShortestPathLen if found a shorter path
        int32_t newDist = ShortestPathLen[s] + 1;
        if(ShortestPathLen[d] > newDist) {
            ShortestPathLen[d] = newDist;
            if(Visited[d] == 0) { Visited[d] = 1 ; return 1;}
        }
        return 0;
    }
    inline bool updateAtomic (uint32_t s, uint32_t d){ //atomic Update
        int32_t newDist = ShortestPathLen[s] + 1;
        // printf("s %d, d %d, edgelen %d\n", s, d, edgeLen);
        return (writeMin(&ShortestPathLen[d],newDist) &&
        CAS(&Visited[d],0,1));
    }
    inline bool cond (uint32_t d) { return 1; } 
    // from ligra readme: for ret true, return cond_true
    // return cond_true(d); }
};

//reset visited vertices
struct BF_Vertex_F {
    int* Visited;
    BF_Vertex_F(int* _Visited) : Visited(_Visited) {}
    inline bool operator() (uint32_t i){
        // printf("reset visited %u\n", i);
        Visited[i] = 0;
        return 1;
    }
};

// template <class vertex>
int32_t* SSSP_BF(ntgraph &G, long start) {

    long n = G.get_num_vertices();
    int32_t* ShortestPathLen = newA(int32_t,n);
    parallel_for (long i=0;i<n;i++) {ShortestPathLen[i] = INT_MAX / 2;}
    ShortestPathLen[start] = 0;

    int* Visited = newA(int,n);
    parallel_for (long i=0;i<n;i++) {Visited[i] = 0;}
    VertexSubset frontier = VertexSubset(start, n);
    long round = 0;

    

    while (frontier.not_empty()) { 

        if (round == n) {
            parallel_for (long i = 0; i < n; i++) {ShortestPathLen[i] = -(INT_MAX / 2);}
            break;
        }

        VertexSubset output = edgeMap(G, frontier, BF_F(ShortestPathLen, Visited), true, 1);

        vertexMap(output, BF_Vertex_F(Visited), false);
        frontier.del();
        frontier = output;
        round++;
    }

    frontier.del();
    free(Visited);
    return ShortestPathLen;
}
