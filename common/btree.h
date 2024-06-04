#ifndef _BTREE_H_
#define _BTREE_H_

#include <string.h>
#include <utility>
#include <iostream>
#include <stack>
#include <cassert>
#include <vector>

#include "apps/config.h"
#include "inc/pmem_pool_manage.h"

#define MIN_KEYS 64
#define MAX_KEYS (2*MIN_KEYS-1) //127
#define MAX_CHILDREN (2*MIN_KEYS) //128

static pmem_pool_allocator* nvm_alloc = nullptr;
static char* node_start = nullptr;
class node;


inline static node* offset_to_node(offset_t _offset) {
    if (_offset == 0) return nullptr;
    return (node*) (node_start + _offset);
}

class node {
public:
    node(bool is_leaf): num_keys(0), offset(0), is_leaf(is_leaf) {

    }
    void init(bool _is_leaf) {
        is_leaf = _is_leaf;
        num_keys = 0;
        offset = 0;
        for (uint32_t i = 0; i < MAX_KEYS; i++) {
            keys[i] = 0;
#ifdef WEIGHTED
            weights[i] = 0;
#endif
        }

        for (uint32_t i = 0; i < MAX_CHILDREN; i++) {
            children[i] = 0;
        }


    }

#ifdef WEIGHTED
    weight_t get_val(vtxid_t id) {
        uint32_t i = 0;
        while (i < num_keys && id > keys[i])
            i++;
        
        if (keys[i] == id)
            return weights[i];

        if (is_leaf)
            return 0;

        return offset_to_node(children[i])->get_val(id);
    }
#endif

#ifdef WEIGHTED
    bool insertNonFull(vtxid_t id, weight_t weight) {
#else
    bool insertNonFull(vtxid_t id) {
        // 已经存在，返回 false，
        // 叶子节点，插入成功，返回 true
        // 当前的非叶子节点满了，

        uint32_t i;
        for (i = 0; i < num_keys; i++) {
            if (keys[i] < id)
                continue;
            else if (id == keys[i]) {
#ifdef WEIGHTED
                weights[i] = weight;
#endif
                return false;
            } else {
                break;
            }
        }

        if (is_leaf) {
//             for (uint32_t j = num_keys; j >= i + 1; j--) {
//                 keys[j] = keys[j - 1];
// #ifdef WEIGHTED
//                 weights[j] = weights[j - 1];
// #endif
//             }

            memmove(&keys[i+1], &keys[i], (MAX_KEYS-i-1)*sizeof(keys[0]));
#ifdef WEIGHTED
            memmove(&weights[i+1], &weights[i], (MAX_KEYS-i-1)*sizeof(weights[0]));
#endif

            keys[i] = id;
#ifdef WEIGHTED
            weights[i] = weight;
#endif

            num_keys = num_keys + 1;
            return true;
        } else {
            if (offset_to_node(children[i])->num_keys == MAX_KEYS) {
            
                splitChild(i, offset_to_node(children[i]));
                if (keys[i] == id)
                    return false;
                else if (keys[i] < id)
                    i++;
            }
#ifdef WEIGHTED
            return offset_to_node(children[i])->insertNonFull(id, weight);
#else
            return offset_to_node(children[i])->insertNonFull(id);
#endif
        }


    }
#endif

    bool remove(vtxid_t k) {
        uint32_t idx = findKey(k);

        if (idx < num_keys && keys[idx] == k) {
            if (is_leaf) removeFromLeaf(idx);
            else removeFromNonLeaf(idx);
        } else {
            
            if (is_leaf) {
                return false;
            }
            
            bool flag = ((idx == num_keys) ? true : false);

        
            if (offset_to_node(children[idx])->num_keys < MIN_KEYS) fill(idx);

            
            if (flag && idx > num_keys) offset_to_node(children[idx-1])->remove(k);
            else offset_to_node(children[idx])->remove(k);
        }

        return true;
    }
    uint32_t findKey(vtxid_t k) { // 找到第一个大于等于 k 的位置，如果不存在（k 最大），则返回 num_keys
        int idx = 0;
        while (idx < num_keys && keys[idx] < k)
            ++idx;
        return idx;
    }
    void removeFromLeaf(vtxid_t idx) {
        memcpy(&keys[idx], &keys[idx+1], (MAX_KEYS-idx-1)*sizeof(keys[0]));
#ifdef WEIGHTED
        memcpy(&weights[idx], &weights[idx+1], (MAX_KEYS-idx-1)*sizeof(weights[0]));
#endif
        num_keys--;
    }
    void removeFromNonLeaf(vtxid_t idx) {
        vtxid_t k = keys[idx];
        if (offset_to_node(children[idx])->num_keys >= MIN_KEYS) {
            auto pred = getPred(idx);
#ifdef WEIGHTED
            keys[idx] = pred.first;
            weights[idx] = pred.second;
            children[idx]->remove(pred.first);
#else
            keys[idx] = pred;
            offset_to_node(children[idx])->remove(pred);
#endif
        } else if (offset_to_node(children[idx+1])->num_keys >= MIN_KEYS) {
            auto succ = getSucc(idx);
#ifdef WEIGHTED
            keys[idx] = succ.first;
            weights[idx] = succ.second;
            children[idx+1]->remove(succ.first);
#else
            keys[idx] = succ;
            offset_to_node(children[idx+1])->remove(succ);
#endif
        } else {
            merge(idx);
            offset_to_node(children[idx])->remove(k);
        }
    }


#ifdef WEIGHTED
    std::pair<vtxid_t, weight_t> getPred(vtxid_t idx) {
#else
    vtxid_t getPred(uint32_t idx) {
#endif

        node* cur = offset_to_node(children[idx]);
        while (!cur->is_leaf) 
            cur = offset_to_node(cur->children[cur->num_keys]); 

  // Return the last key of the leaf 
#ifdef WEIGHTED
        return std::make_pair(cur->keys[cur->num_keys-1], cur->weights[cur->num_keys-1]);
#else
        return cur->keys[cur->num_keys-1]; 
#endif

    }


#ifdef WEIGHTED
    std::pair<vtxid_t, weight_t> getSucc(uint32_t idx) {
#else
    vtxid_t getSucc(uint32_t idx) {
#endif

        node* cur = offset_to_node(children[idx+1]);
        while (!cur->is_leaf) 
            cur = offset_to_node(cur->children[0]); 

#ifdef WEIGHTED
        return std::make_pair(cur->keys[0], cur->weights[0]);
#else
        return cur->keys[0]; 
#endif

    }

    void fill(uint32_t idx) {
        if (idx != 0 && offset_to_node(children[idx-1])->num_keys >= MIN_KEYS) 
            borrowFromPrev(idx); 
        else if (idx != num_keys && offset_to_node(children[idx+1])->num_keys >= MIN_KEYS) 
            borrowFromNext(idx);
        else { 
            if (idx != num_keys) 
                merge(idx); 
            else
                merge(idx-1); 
        }
    }

    void borrowFromPrev(uint32_t idx) {
        node* child = offset_to_node(children[idx]);
        node* sibling = offset_to_node(children[idx-1]);

        for (int32_t i = child->num_keys - 1; i >= 0; --i) {
            child->keys[i+1] = child->keys[i];
#ifdef WEIGHTED
            child->weights[i+1] = child->weights[i];
#endif
        }

        
        if (!child->is_leaf) {
            for(int32_t i = child->num_keys; i >= 0; --i)
                child->children[i+1] = child->children[i];
        }

        
        child->keys[0] = keys[idx-1];
#ifdef WEIGHTED
        child->weights[0] = weights[idx-1];
#endif

        
        if(!child->is_leaf)
            child->children[0] = sibling->children[sibling->num_keys];


        keys[idx-1] = sibling->keys[sibling->num_keys-1];

#ifdef WEIGHTED
        weights[idx-1] = sibling->weights[sibling->num_keys-1];
#endif

        child->num_keys += 1;
        sibling->num_keys -= 1;
    }

    void borrowFromNext(uint32_t idx) {
        node* child = offset_to_node(children[idx]);
        node* sibling = offset_to_node(children[idx+1]);

        child->keys[(child->num_keys)] = keys[idx];
#ifdef WEIGHTED
        child->weights[(child->num_keys)] = weights[idx];
#endif


        if (!(child->is_leaf))
            child->children[(child->num_keys)+1] = sibling->children[0];


        keys[idx] = sibling->keys[0];
#ifdef WEIGHTED
        weights[idx] = sibling->weights[0];
#endif

        for (uint32_t i = 1; i < sibling->num_keys; ++i) {
            sibling->keys[i-1] = sibling->keys[i];
#ifdef WEIGHTED
            sibling->weights[i-1] = sibling->weights[i];
#endif
        }

        if (!sibling->is_leaf) {
            for(uint32_t i = 1; i <= sibling->num_keys; ++i)
                sibling->children[i-1] = sibling->children[i];
        }

        child->num_keys += 1;
        sibling->num_keys -= 1;
    }
    void merge(uint32_t idx) {
        node* child = offset_to_node(children[idx]);
        node* sibling = offset_to_node(children[idx+1]);

        child->keys[MIN_KEYS-1] = keys[idx];
#ifdef WEIGHTED
        child->weights[MIN_KEYS-1] = weights[idx];
#endif


        for (uint32_t i = 0; i < sibling->num_keys; ++i) {
            child->keys[i+MIN_KEYS] = sibling->keys[i];
#ifdef WEIGHTED
            child->weights[i+MIN_KEYS] = sibling->weights[i];
#endif
        }

  
        if (!child->is_leaf) {
            for(uint32_t i = 0; i <= sibling->num_keys; ++i)
                child->children[i+MIN_KEYS] = sibling->children[i];
        }


        for (uint32_t i = idx + 1; i < num_keys; ++i) {
            keys[i-1] = keys[i];
#ifdef WEIGHTED
            weights[i-1] = weights[i];
#endif
        }

        for (uint32_t i = idx + 2; i <= num_keys; ++i)
            children[i-1] = children[i];


        child->num_keys += sibling->num_keys+1;
        num_keys--;

    }

    void splitChild(uint32_t i, node* c) {
        char* child_addr = (char*) nvm_alloc->alloc(sizeof(node));
        offset_t _offset = child_addr - node_start;

        node* child = (node*) child_addr;
        child->init(c->is_leaf);
        child->num_keys = MIN_KEYS - 1;
        child->offset = _offset;

//         int k = 0;
//         for (uint32_t j = MIN_KEYS; j < c->num_keys; j++) {
//             child->keys[k] = c->keys[j];
// #ifdef WEIGHTED
//             child->weights[k] = c->weights[j];
// #endif
//             k++;
//         }

        memmove(&child->keys[0], &c->keys[MIN_KEYS], (MIN_KEYS-1)*sizeof(keys[0]));
#ifdef WEIGHTED
        memmove(&child->weights[0], &c->weights[MIN_KEYS], (MIN_KEYS-1)*sizeof(weights[0]));
#endif

        if (!c->is_leaf) {
            memmove(&child->children[0], &c->children[MIN_KEYS], (MAX_KEYS) * sizeof(children[0]));
        }

        c->num_keys = MIN_KEYS - 1;

        memmove(&children[i + 2], &children[i + 1], (MAX_CHILDREN - i - 2) * sizeof(children[0]));
        children[i+1] = _offset;
        
        memmove(&keys[i+1], &keys[i], (MAX_KEYS-i-1)*sizeof(keys[0]));
#ifdef WEIGHTED
        memmove(&weights[i+1], &weights[i], (MAX_KEYS-i-1)*sizeof(weights[0]));
#endif

        keys[i] = c->keys[MIN_KEYS-1];
#ifdef WEIGHTED
        weights[i] = c->weights[MIN_KEYS-1];
#endif
        num_keys = num_keys + 1;
    }

    const node* find(vtxid_t id) {
        uint32_t i = 0;
        while (i < num_keys && id > keys[i])
            i++;
        
        if (i < num_keys && keys[i] == id)
            return offset_to_node(offset);
        
        if (is_leaf)
            return nullptr;
        
        
        return offset_to_node(children[i])->find(id);
    }

    void traverse() {
        uint32_t i;
        for (i = 0; i < num_keys; i++) {
            if (!is_leaf)
                offset_to_node(children[i])->traverse();
            std::cout << keys[i] << " ";
        }

        if (!is_leaf)
            offset_to_node(children[i])->traverse();
    }
    uint64_t sum() {
        uint32_t i;
        uint64_t count = 0;
        for (i = 0; i < num_keys; i++) {
            if (!is_leaf)
                count += offset_to_node(children[i])->sum();
            count += keys[i];  
        }

        if (!is_leaf)
            count += offset_to_node(children[i])->sum();
        return count;
    }

    uint32_t get_num_nodes() {
        uint32_t count = 1;
        uint32_t i;
        for (i = 0; i < num_keys; i++) {
            if (!is_leaf)
                count += offset_to_node(children[i])->get_num_nodes();
        }
        if (!is_leaf)
            count += offset_to_node(children[i])->get_num_nodes();
        return count;
    }

/*
    template <class F> void map_node(F &f) {
        for (uint64_t i = 0; i < num_keys; i++) {

#ifdef WEIGHTED
            f.update(keys[i], weights[i]);
#else
            f.update(keys[i]);
#endif
        }
    }

    template <class F> void map_tree(F &f) {
        map_node(f);
        if (!is_leaf) {
            //parallel_for (uint32_t i = 0; i < num_keys+1; i++) {
            for (uint32_t i = 0; i < num_keys+1; i++) {
            if (children[i] != 0)
                offset_to_node(children[i])->map_tree(f);
            }
        }
    }
*/

    class node_iterator {
    public:
        node_iterator() {};
        node_iterator(node* n) {
            assert(n != nullptr);
            node* cur = n;
            while (cur != nullptr) {
                stack.push(std::make_pair(cur, 0));
                cur = offset_to_node(cur->children[0]);
            }
            cur_node = stack.top().first;
            cur_idx = 0;
        }

#ifdef WEIGHTED
        std::pair<vtxid_t, weight_t> operator*(void) {
            return std::make_pair(cur_node->keys[cur_idx], cur_node->weights[cur_idx]);
        }
#else
        vtxid_t operator*(void) {
            return cur_node->keys[cur_idx];
        }
#endif

        void operator++() {
            if (cur_node->is_leaf) {  // if we are traversing a leaf node
                if (cur_idx < cur_node->num_keys-1)
                cur_idx++;
                else {  // leaf node is done. Go to parent
                stack.pop();
                if (stack.size()) {
                    cur_node = stack.top().first;
                    cur_idx = stack.top().second;
                    stack.top().second++;
                }
                }
            } else { // if we are traversing an internal node
                if (cur_idx == cur_node->num_keys-1) { // internal node is done. Go to parent
                stack.pop();
                }
                node* cur = offset_to_node(cur_node->children[cur_idx + 1]);
                while (cur != nullptr) {
                    stack.push(std::make_pair(cur, 0));
                    cur = offset_to_node(cur->children[0]);
                }
                cur_node = stack.top().first;
                cur_idx = 0;
            }
        }

        bool done() {
            return !stack.size();
        }

    private:
        std::stack<std::pair<node*, uint32_t>> stack;
        node *cur_node;
        uint32_t cur_idx;
    };

    node_iterator begin(void) { return node_iterator(this); }

    friend class btree;

// private:
    vtxid_t keys[MAX_KEYS];   // 127 key

#ifdef WEIGHTED
    weight_t weights[MAX_KEYS];
#endif

    offset_t children[MAX_CHILDREN];  //128 children
    offset_t offset;
    uint32_t num_keys;

    bool is_leaf;
    char padding[48];
};


class btree {
public:
    btree(): root_offset(0) {}
    void init() {
        root_offset = 0;
    }

#ifdef WEIGHTED
    weight_t get_val(vtxid_t id) const {
        return (root_offset == 0) ? 0 : root->get_val(id);
    }
#endif

    const node* find(vtxid_t id) const {
        return (root_offset == 0) ? nullptr : offset_to_node(root_offset)->find(id);
    }

#ifdef WEIGHTED
    bool insert(vtxid_t k, weight_t w) {
#else
    bool insert(vtxid_t k) {
#endif

        if (root_offset == 0)  {

            char* child_addr = (char*) nvm_alloc->alloc(sizeof(node));
            offset_t _offset = child_addr - node_start;

            

            root_offset = _offset;

            node* root = (node*) child_addr;
            root->init(true);

            root->keys[0] = k;
            root->offset = _offset;

#ifdef WEIGHTED
            root->weights[0] = w;
#endif

            root->num_keys = 1;
            return true;
        } else {  
            if (offset_to_node(root_offset)->num_keys == MAX_KEYS) {
            
                char* child_addr = (char*) nvm_alloc->alloc(sizeof(node));
                offset_t _offset = child_addr - node_start;
                
                node* s = (node*) child_addr;
                
                s->init(false);
                s->children[0] = root_offset;
                s->offset = _offset;
                s->splitChild(0, offset_to_node(root_offset));

                
                int i = 0;
                if (s->keys[0] < k)
                    i++;

                root_offset = _offset;
#ifdef WEIGHTED
                return offset_to_node(s->children[i])->insertNonFull(k, w);
#else
                return offset_to_node(s->children[i])->insertNonFull(k);
#endif
            }
            else  
#ifdef WEIGHTED
                return offset_to_node(root_offset)->insertNonFull(k, w);
#else
                return offset_to_node(root_offset)->insertNonFull(k);
#endif
        }
    }

    bool remove(vtxid_t k) {
        if (!root_offset)
            return false;

        bool ret = offset_to_node(root_offset)->remove(k);


        if (offset_to_node(root_offset)->num_keys == 0) {
            
            if (offset_to_node(root_offset)->is_leaf)
                root_offset = 0;
            else
                root_offset = offset_to_node(root_offset)->children[0];
            
            
        }
        return ret;
    }

    void traverse() const {
        if (root_offset != 0) offset_to_node(root_offset)->traverse();
    }

    uint64_t sum() const {
        if (root_offset != 0)
            return offset_to_node(root_offset)->sum();
        return 0;
    }

    uint32_t get_num_nodes() const {
        if (root_offset != 0) return offset_to_node(root_offset)->get_num_nodes();
        return 0;
    }

    uint64_t get_size(void) const {
        return get_num_nodes() * sizeof(node);
    }

    const node* get_root(void) const {
        return offset_to_node(root_offset);
    }

    /*
    template<class F> void map(F &f) {
        root->map_tree(f);
    }
    */

    class btree_iterator {
    public:
        btree_iterator() {};
        btree_iterator(const btree* tree) {
            assert(tree->root_offset != 0);
            it = offset_to_node(tree->root_offset)->begin();
        }

#ifdef WEIGHTED
        std::pair<vtxid_t, weight_t> operator*(void) { return *it; }
#else
        vtxid_t operator*(void) { return *it; }
#endif
        void operator++(void) { ++it; }
        bool done(void) { return it.done(); }

    private:
        node::node_iterator it;
    };

    btree_iterator begin() const { return btree_iterator(this); }

// private:
    offset_t root_offset; 
};

#endif