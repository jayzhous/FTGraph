#include <iostream>
#include <cassert>
#include <set>
#include <map>
#include <string>

#include <stdlib.h>

#include "common/btree.h"

using namespace std;

int main() {
    uint64_t size = 1L * 1024 * 1024 * 1024;
    std::string path = "../btree_canbedeleted";
    nvm_alloc = new pmem_pool_allocator((char*) path.data(), size);

    manage_meta_data* meta_data = nvm_alloc->get_meta_data();

    if (meta_data->get_offset() == sizeof(manage_meta_data)) {
        nvm_alloc->set_node_start(nvm_alloc->get_cur());
        node_start = (char*) nvm_alloc->get_node_start();

        char* child_addr = (char*) nvm_alloc->alloc(sizeof(node));
    } else {
        node_start = (char*) (nvm_alloc->get_base() + sizeof(manage_meta_data));
        nvm_alloc->set_node_start(node_start);
        
    }

    cout << "sizeofnode: " << sizeof(node) << endl;
    
    btree tree;

    int nvals = 1000;
    for (int i = nvals - 1; i >= 0; i--) {
        tree.insert(i);
    }

/*
    node* root = offset_to_node(tree.root_offset);
    cout << "root_num_keys " << root->num_keys << endl;
    cout << "nums_node " << tree.get_num_nodes() << endl;

    cout << "root is leaf:" << root->is_leaf << ", offset: " << root->offset << endl;

    node* left_child = offset_to_node(root->children[0]);
    node* right_child = offset_to_node(root->children[1]);

    cout << "left is leaf:" << left_child->is_leaf << ", offset: " << left_child->offset << endl;
    for (int i = 0; i < left_child->num_keys; i++) {
        cout << left_child->keys[i] << " ";
    }
    cout << endl;

    cout << "right is leaf:" << right_child->is_leaf << ", offset: " << right_child->offset << endl;
    for (int i = 0; i < right_child->num_keys; i++) {
        cout << right_child->keys[i] << " ";
    }
    cout << endl;
*/



    for (int i = 0; i < nvals; i++) {
        if (tree.find(i) == nullptr) {
            printf("Error-%d ", i);
        }
    }
    printf("\n");

    /*
    for (auto it = tree.begin(); !it.done(); ++it) {
        printf("%d ", *it);
	}
    printf("\n");

    tree.traverse();
    printf("\n");
    */


    for (int i = 0; i < nvals; i++) {

		if (tree.find(i) != nullptr) {
			if (!tree.remove(i)) {
				printf("remove failed-%d ", i);
			}
			if (tree.find(i) != nullptr) {
				printf("Query failed after remove! ");
			}
		}
	}

    delete nvm_alloc;
    return 0;
}

// g++ test_btree.cc -I/mnt/pmem1/sungan/dynamic_graph_230207/ntgraph/v4.7/ 