#include "inc/ntgraph.h"

int ntgraph::find_edge_g(vtxid_t src_vtx_id, vtxid_t dst_vtx_id) {
    index_t src_index = vtx_index.find_vertex(src_vtx_id);
    index_t dst_index = vtx_index.find_vertex(dst_vtx_id);
    if (src_index == -1 || dst_index == -1) {
        return 0;
    }
    node* root = (node*) ntgraph::root_node_addr(src_index);
    return ntgraph::find_edge_t(root, dst_index);
}

int ntgraph::find_edge_t(node* root, vtxid_t dst_vtx_id) {

    node* n = root;
    while (1) {
        int k = n->find_key(dst_vtx_id);
        if (k >= 0) {
            return 1;
        }
        if (n->is_offset_valid != 0) {
            n = (node*) n->index_branch_addr(node_start, dst_vtx_id);
        } else {
            // the edge doesn't exist
            return 0;
        }
    } // while
}



#ifdef NO_CONCURRENT
int ntgraph::insert_edge(vtxid_t src_vtx_id, vtxid_t dst_vtx_id, weight_t val) {
    vtxid_t dst_index = vtx_index.insert_vertex(dst_vtx_id);
    vtxid_t src_index = vtx_index.insert_vertex(src_vtx_id);

    vertex* src_vertex = vtx_index.indexed_vertex(src_index);

    node* n = (node*) ntgraph::root_node_addr(src_index);

    #ifndef NO_CHECK_BEFORE_INSERT
    if (ntgraph::find_edge_t(n, dst_index) == 1) {
        return 0;
    }
    #endif

    int high = n->first_zero();
    while (1) {

        if (high == -1) { // the current node is full, and the num of valid bits = 64, so high = -1
            if (n->is_offset_valid == 0) {
                n->branch_out(nvm_alloc);
                src_vertex->add_num_nodes(n->get_num_branch());
            }
            assert(n->first_child_offset != 0);
            n = (node*) n->index_branch_addr(node_start, dst_index);
            high = n->first_zero();
        } else if (high == n->level_to_size()) { // high = 16 or 32
            if (n->size_lv == 0) {
                n->scale_up(1);
            } else if (n->size_lv == 1) {
                n->scale_up(2);
            }

        } else {
            // insert here
            unsigned char key_hash = hashcode(dst_index);
            n->edges[high].id = dst_index;
            
            #ifdef WEIGHTED
            n->edges[high].val = val;
            #endif


            #ifdef CLWB_SFENCE
            int num_edges_per_line = CACHE_LINE_SIZE / sizeof(edge);
            clwb((void*) &(n->edges[high / num_edges_per_line * num_edges_per_line]));
            sfence();
            #endif

            n->fgpt[high] = key_hash;

            #ifdef CLWB_SFENCE
            clwb((void*) n->fgpt);
            sfence();
            #endif

            n->bitmap.set(high);

            #ifdef CLWB_SFENCE
            clwb((void*) n);
            sfence();
            #endif

            src_vertex->add_degree();

            #ifdef EDGELIST
            if (n->node_lv >= EL_START_LV) {
                #ifdef WEIGHTED
                src_vertex->insert_el(dst_index, val);
                #else
                src_vertex->insert_el(dst_index);
                #endif
            }
            #endif

            return 1;
        }
    } //while
    
}
#else

int ntgraph::insert_edge_parallel(vtxid_t src_vtx_id, vtxid_t dst_vtx_id, weight_t val) {
    vtxid_t src_index = vtx_index.insert_vertex_parallel(src_vtx_id);
    vtxid_t dst_index = vtx_index.insert_vertex_parallel(dst_vtx_id);

    node* n = (node*) ntgraph::root_node_addr(src_index);
    
    int high;
    uint64_t old_bm;
    version_t old_version;


    while (1) {
        old_bm = n->locate_map.to_uint64();
        old_version = n->get_read_lock(); // if old_version = 0, the node is branching out / scaling up

        // printf("old version = %lu\n", old_version);

        if (old_bm == n->bitmap.to_uint64() && old_version != 0) {
            
            int k = n->find_key(dst_index);
            if (k >= 0) { 
                // the target edge is found, 
                break;
            }
            // the target edge is not found

            if (old_bm == UINT64) {
                
                // go to child node or branch out

                // bm = UINT64, so no more edges can be inserted. needn't consider whether two two bitmap is the same


                // if (n->is_offset_valid != 0) {
                //     // go to its child node
                //     n = (node*) n->index_branch_addr(node_start, dst_vtx_id);
                // } else {
                //     // branch out

                //     if (!n->release_read_lock(old_version)) { // two version is different, the node is branching out
                //         continue;
                //     } else { // has branched out or has not branched out

                //         if (n->branch_out(nvm_alloc)) {
                //             vertices[src_index].add_num_nodes(n->get_num_branch());
                //         }

                //     }
                // }


                if (!n->release_read_lock(old_version)) { // two version is different, the node is branching out
                    continue;
                } else {
                    if (n->branch_out(nvm_alloc)) {
                        vertices[src_index].add_num_nodes(n->get_num_branch());
                    }
                    n = (node*) n->index_branch_addr(node_start, dst_index);
                }

                
            } /*else if (old_bm == UINT16) {
                // scale up to size 32
                
                operation = 1;
                break;
            } else if (old_bm == UINT32) {
                // scale up to size 64
                
                operation = 2;
                break;
            } */else {
                // has empty entry, insert
                bool changed = false;
                high = n->try_locate_entry(old_bm, changed);
                
                if (changed) {
                    break;

                    /*
                    unsigned char key_hash = hashcode(dst_index);
                    n->edges[high].id = dst_index;   
                    #ifdef WEIGHTED
                    rn->edges[high].val = val;
                    #endif

                    #ifdef CLWB_SFENCE
                    int num_edges_per_line = CACHE_LINE_SIZE / sizeof(edge);
                    clwb((void*) &(n->edges[high / num_edges_per_line * num_edges_per_line]));
                    sfence();
                    #endif

                    n->fgpt[high] = key_hash;
                    #ifdef CLWB_SFENCE
                    clwb((void*) n->fgpt);
                    sfence();
                    #endif

                    n->set_entry(high);
                    #ifdef CLWB_SFENCE
                    clwb((void*) n);
                    sfence();
                    #endif

                    vertices[src_index].add_degree();
                    */

                } // restart find 
            }
        } // a thread is inserting, insert or find process need wait!
    }

    
    unsigned char key_hash = hashcode(dst_index);

    n->edges[high].id = dst_index;   
    #ifdef WEIGHTED
    rn->edges[high].val = val;
    #endif

    #ifdef CLWB_SFENCE
    int num_edges_per_line = CACHE_LINE_SIZE / sizeof(edge);
    clwb((void*) &(n->edges[high / num_edges_per_line * num_edges_per_line]));
    sfence();
    #endif

    
    n->fgpt[high] = key_hash;
    #ifdef CLWB_SFENCE
    clwb((void*) n->fgpt);
    sfence();
    #endif

    n->set_entry(high);
    #ifdef CLWB_SFENCE
    clwb((void*) n);
    sfence();
    #endif

    vertices[src_index].add_degree();
    

    
    return 1;
}
#endif

#ifdef NO_CONCURRENT
int ntgraph::delete_edge(vtxid_t src_vtx_id, vtxid_t dst_vtx_id) {

    vtxid_t dst_index = vtx_index.insert_vertex(dst_vtx_id);
    vtxid_t src_index = vtx_index.insert_vertex(src_vtx_id);

    vertex* src_vertex = vtx_index.indexed_vertex(src_index);
    
    if (src_index == -1 || dst_index == -1) { // the edge does not exist
        return 0; 
    }
    node* cur = (node*) ntgraph::root_node_addr(src_index);

    while (1) {
        if (cur->bitmap.to_uint64() != 0) {
            int k = cur->find_key(dst_index);
            if (k >= 0) {

                int last = cur->first_zero();
                if (last == -1) {
                    last = MAX_ENTRY_NUM - 1;
                } else {
                    last -= 1;
                }

                
                cur->bitmap.clear(k);
                #ifdef CLWB_SFENCE
                clwb((void*) cur);
                sfence();
                #endif

                if (k == last) {
                    src_vertex->add_degree(-1);
                    return 1;
                }

                cur->edges[k] = cur->edges[last];

                #ifdef CLWB_SFENCE
                int num_edges_per_line = CACHE_LINE_SIZE / sizeof(edge);
                clwb((void*) &(cur->edges[k / num_edges_per_line * num_edges_per_line]));
                sfence();
                #endif

                // do not need to persist the cache line of edge last

                cur->fgpt[k] = cur->fgpt[last];
                #ifdef CLWB_SFENCE
                clwb((void*) cur->fgpt);
                sfence();
                #endif

                uint64_t new_bm = cur->bitmap.to_uint64();
                new_bm &= ~(1ull << last);  // set the bit of position last to 0
                new_bm |= (1ull << k); // set the bit of position k to 1

                cur->bitmap.update(new_bm);

                #ifdef CLWB_SFENCE
                clwb((void*) cur);
                sfence();
                #endif

                return 1;
            }
        }
        if (cur->is_offset_valid != 0) {
            cur = (node*) cur->index_branch_addr(node_start, dst_index);
        } else {
            // the edge doesn't exist
            return 0;
        }
    } // while
    return 0;
}
#else
int ntgraph::delete_edge_parallel(vtxid_t src_vtx_id, vtxid_t dst_vtx_id) {
    // not implemented

    return 0;
}
#endif

degree_t ntgraph::get_out_degree(vtxid_t src_vtx_id) {
    index_t src_index = vtx_index.find_vertex(src_vtx_id);
    if (src_index == -1) { // the vertex doesn't exist
        return 0;
    }

    return ntgraph::get_out_degree_l(src_index);
}


degree_t ntgraph::get_out_degree_l(vtxid_t src_index) {
    assert(src_index >= 0);
    assert(src_index < vtx_index.count_vertex());

    degree_t out_degree = 0;

    node* n = (node*) ntgraph::root_node_addr(src_index);
    out_degree += __builtin_popcountll(n->bitmap.to_uint64());

    int MAX_NODE_NUM = 200;
    offset_t stack[MAX_NODE_NUM];
    int top = -1;

    if (n->is_offset_valid != 0) {
        int num_branch = n->get_num_branch();
        for (int j = num_branch - 1; j >= 0; j--) {
            stack[++top] = n->first_child_offset + (sizeof(node) * j);
        }
    }

    offset_t offset;
    while (top != -1) {
        offset = stack[top--];
        n = (node*) (node_start + offset);
        out_degree += __builtin_popcountll(n->bitmap.to_uint64());

        if (n->is_offset_valid != 0) {
            int num_branch = n->get_num_branch();
            for (int j = num_branch - 1; j >= 0; j--) {
                stack[++top] = n->first_child_offset + (sizeof(node) * j);
            }
            if (top >= MAX_NODE_NUM) {
                fprintf(stderr, "Error: stack overflows (get_out_degree). \n");
                exit(1);
            }
        }
    }

    return out_degree;
    
}

degree_t ntgraph::get_adj_edges(vtxid_t src_vertex_id, std::vector<edge>& adj_edges) {
    index_t src_index = vtx_index.find_vertex(src_vertex_id);
    if (src_index == -1) {
        // the vertex doesn't exist
        fprintf(stderr, "Error: get_adj_edges: the vertex %d does not exist \n", src_vertex_id);
        exit(1);
    }
    return ntgraph::get_adj_edges_l(src_index, adj_edges);
}

degree_t ntgraph::get_adj_edges(vtxid_t src_vertex_id, edge* adj_edges) {
    index_t src_index = vtx_index.find_vertex(src_vertex_id);
    if (src_index == -1) {
        // the vertex doesn't exist
        fprintf(stderr, "Error: get_adj_edges: the vertex %d does not exist \n", src_vertex_id);
        exit(1);
    }
    return ntgraph::get_adj_edges_l(src_index, adj_edges);
}


#ifdef EDGELIST
degree_t ntgraph::get_adj_edges_l(vtxid_t src_index, std::vector<edge>& adj_edges) {
    assert(src_index >= 0);
    assert(src_index < vtx_index.count_vertex());

    vertex* src_vertex = vtx_index.indexed_vertex(src_index);
    node* cur = (node*) ntgraph::root_node_addr(src_index);
    
    uint32_t num_nodes = src_vertex->get_num_nodes();
    if (num_nodes <= 1) {
        for (int j = 0; j < cur->get_num_edges(); j++) {
            adj_edges.push_back(cur->edges[j]);
        }
        return adj_edges.size();
    }

    if (EL_START_LV > 0) {
        for (int j = 0; j < cur->get_num_edges(); j++) {
            adj_edges.push_back(cur->edges[j]);
        }
        return adj_edges.size();
    }

    int front = -1;
    int rear = -1;

    uint32_t maxSize = num_nodes - 1;
    node** queue = (node**) malloc(sizeof(node*) * maxSize);

    if (EL_START_LV > 1 && cur->is_offset_valid != 0) {
        for (int j = 0; j < cur->get_num_branch(); j++) {
            rear += 1;
            queue[rear] = ntgraph::offset_to_node(cur->first_child_offset + (sizeof(node) * j));
        }
    }

    while (front != rear) {
        front += 1;
        cur = queue[front];
        if (cur->node_lv >= EL_START_LV) {
            break;
        }
        for (int j = 0; j < cur->get_num_edges(); j++) {
            adj_edges.push_back(cur->edges[j]);
        }

        if (cur->is_offset_valid != 0) {
            for (int j = 0; j < cur->get_num_branch(); j++) {
                rear += 1;
                if (rear == maxSize) {
                    fprintf(stderr, "Error: n_ptrs queue overflows (get_adj_edges_l).\n");
                    exit(1);
                }
                queue[rear] = ntgraph::offset_to_node(cur->first_child_offset + (sizeof(node) * j));
            }
        }
    }

    edgelist* local_el = src_vertex->first_el;
    while (local_el != nullptr) {
        for (int j = 0; j < local_el->get_num_edges(); j++) {
            adj_edges.push_back(local_el->edges[j]);
        }
        local_el = local_el->next_el;
    }
    free(queue);
    return adj_edges.size();
}

degree_t ntgraph::get_adj_edges_l(vtxid_t src_index, edge* adj_edges) {
    assert(src_index >= 0);
    assert(src_index < vtx_index.count_vertex());

    vertex* src_vertex = vtx_index.indexed_vertex(src_index);
    node* cur = (node*) ntgraph::root_node_addr(src_index);
    degree_t count = 0;

    uint32_t num_nodes = src_vertex->get_num_nodes();
    if (num_nodes <= 1) {
        memcpy(adj_edges, cur->edges, sizeof(edge) * cur->get_num_edges());
        count += cur->get_num_edges();
        return count;
    }

    if (EL_START_LV > 0) {
        memcpy(adj_edges, cur->edges, sizeof(edge) * cur->get_num_edges());
        count += cur->get_num_edges();
    }

    int front = -1;
    int rear = -1;

    uint32_t maxSize = num_nodes - 1;
    node** queue = (node**) malloc(sizeof(node*) * maxSize);

    if (EL_START_LV > 1 && cur->is_offset_valid != 0) {
        for (int j = 0; j < cur->get_num_branch(); j++) {
            rear += 1;
            queue[rear] = ntgraph::offset_to_node(cur->first_child_offset + (sizeof(node) * j));
        }
    }

    while (front != rear) {
        front += 1;
        cur = queue[front];
        if (cur->node_lv >= EL_START_LV) {
            break;
        }

        memcpy(adj_edges + count, cur->edges, sizeof(edge) * cur->get_num_edges());
        count += cur->get_num_edges();

        if (cur->is_offset_valid != 0) {
             
            for (int j = 0; j < cur->get_num_branch(); j++) {
                rear += 1;
                if (rear == maxSize) {
                    fprintf(stderr, "Error: n_ptrs queue overflows (get_adj_edges_l).\n");
                    exit(1);
                }
                queue[rear] = ntgraph::offset_to_node(cur->first_child_offset + (sizeof(node) * j));
            }
        }
    }

    edgelist* local_el = src_vertex->first_el;
    while (local_el != nullptr) {
        memcpy(adj_edges + count, local_el->edges, sizeof(edge) * local_el->get_num_edges());
        count += local_el->get_num_edges();
        local_el = local_el->next_el;
    }
    free(queue);

    return count;
}

#else
degree_t ntgraph::get_adj_edges_l(vtxid_t src_index, std::vector<edge>& adj_edges) {
    assert(src_index >= 0);
    assert(src_index < vtx_index.count_vertex());

    vertex* src_vertex = vtx_index.indexed_vertex(src_index);

    node* cur = (node*) ntgraph::root_node_addr(src_index);
    for (int j = 0; j < cur->get_num_edges(); j++) {
        adj_edges.push_back(cur->edges[j]);
    }


    uint32_t num_nodes = src_vertex->get_num_nodes();
    if (num_nodes <= 1) {
        return adj_edges.size();
    }

    int front = -1;
    int rear = -1;

    uint32_t maxSize = num_nodes - 1;
    node** queue = (node**) malloc(sizeof(node*) * maxSize);


    if (cur->is_offset_valid != 0) {
        for (int j = 0; j < cur->get_num_branch(); j++) {
            rear += 1;
            queue[rear] = ntgraph::offset_to_node(cur->first_child_offset + (sizeof(node) * j));
        }
    }

    while (front != rear) {
        front += 1;
        cur = queue[front];

        for (int j = 0; j < cur->get_num_edges(); j++) {
            adj_edges.push_back(cur->edges[j]);
        }

        if (cur->is_offset_valid != 0) {
            for (int j = 0; j < cur->get_num_branch(); j++) {
                rear += 1;
                if (rear == maxSize) {
                    fprintf(stderr, "Error: n_ptrs queue overflows (get_adj_edges_l). \n");
                    exit(1);
                }
                queue[rear] = ntgraph::offset_to_node(cur->first_child_offset + (sizeof(node) * j));
            }
        }
    }

    free(queue);
    return adj_edges.size();
}

degree_t ntgraph::get_adj_edges_l(vtxid_t src_index, edge* adj_edges) {
    assert(src_index >= 0);
    assert(src_index < vtx_index.count_vertex());

    vertex* src_vertex = vtx_index.indexed_vertex(src_index);
    degree_t count = 0;

    node* cur = (node*) ntgraph::root_node_addr(src_index);

    memcpy(adj_edges, cur->edges, sizeof(edge) * cur->get_num_edges());
    count += cur->get_num_edges();


    uint32_t num_nodes = src_vertex->get_num_nodes();
    if (num_nodes <= 1) {
        return count;
    }

    int front = -1;
    int rear = -1;

    uint32_t maxSize = num_nodes - 1;
    node** queue = (node**) malloc(sizeof(node*) * maxSize);


    if (cur->is_offset_valid != 0) {
        for (int j = 0; j < cur->get_num_branch(); j++) {
            rear += 1;
            queue[rear] = ntgraph::offset_to_node(cur->first_child_offset + (sizeof(node) * j));
        }
    }

    while (front != rear) {
        front += 1;
        cur = queue[front];


        memcpy(adj_edges + count, cur->edges, sizeof(edge) * cur->get_num_edges());
        count += cur->get_num_edges();

        if (cur->is_offset_valid != 0) {
            for (int j = 0; j < cur->get_num_branch(); j++) {
                rear += 1;
                if (rear == maxSize) {
                    fprintf(stderr, "Error: n_ptrs queue overflows (get_adj_edges_l). \n");
                    exit(1);
                }
                queue[rear] = ntgraph::offset_to_node(cur->first_child_offset + (sizeof(node) * j));
            }
        }
    }

    free(queue);
    return count;
}
#endif

