#include <thread>
#include <vector>
#include <iostream>

#include <stdlib.h>

using namespace std;

const int THREAD_NUM = 2;
const int NODE_NUM = 4;
const uint64_t NODE_SIZE = 1024;
//RNTree中每次分配1024个Node

class TLSManager {
public:
    char* tls_start;
    int   node_cursor;
    TLSManager() {
        //tls_start = 0x50000000;
        node_cursor = 0;
    }
    void* alloc_node() {
        if (node_cursor == NODE_NUM) {
            // 需要从pmem_pool_allocator中分配
            cout << "从pmem_pool_allocator中分配" << endl;
            node_cursor = 0;
            tls_start += 4096;
        }
        return (void*)(tls_start + (node_cursor++) * NODE_SIZE);
    }
};

__thread TLSManager* tls = nullptr; //初始化的时候需要指向一个NVM地址 

void create_tlsmanager(char* addr) {
    if (tls == nullptr) {
        tls = (TLSManager*) addr;
    }
}

int main() {

    vector<thread> threads;
    threads.reserve(static_cast<size_t>(THREAD_NUM));

    char* tls_start = (char*)malloc(2 * sizeof(TLSManager));
    char* node_start = (char*)malloc(16 * 1024);

    for (int i = 0; i < THREAD_NUM; i++) {
        threads.emplace_back([i, tls_start, node_start](){
            create_tlsmanager(tls_start + i * sizeof(TLSManager));
            tls->tls_start = node_start + i * NODE_SIZE * 8;
            void* addr = tls->alloc_node();
            cout << "thread" << i + 1 << ": " << addr << endl;
            addr = tls->alloc_node();
            cout << "thread" << i + 1 << ": " << addr << endl;
            addr = tls->alloc_node();
            cout << "thread" << i + 1 << ": " << addr << endl;
            addr = tls->alloc_node();
            cout << "thread" << i + 1 << ": " << addr << endl;
            addr = tls->alloc_node();
            cout << "thread" << i + 1 << ": " << addr << endl;
            addr = tls->alloc_node();
            cout << "thread" << i + 1 << ": " << addr << endl;
            addr = tls->alloc_node();
            cout << "thread" << i + 1 << ": " << addr << endl;
            addr = tls->alloc_node();
            cout << "thread" << i + 1 << ": " << addr << endl;
        });
    }
    for (auto &t : threads) {
        t.join();
    }
    free(tls_start);
    free(node_start);
    return 0;


}