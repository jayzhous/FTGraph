#include <iostream>
#include <atomic>
#include <vector>
#include <thread>

using namespace std;

typedef unsigned long version_t;

int counter4 = 0;  //使用版本锁
uint64_t gen_id = 0;

class version_lock {
private:
    std::atomic<version_t> version{0};

public:
	version_t read_lock(uint64_t gen_id) {
	    version_t _version = version.load(std::memory_order_acquire);
        if ((_version & 1) != 0) { // if _version is an odd number
            return 0;
        }
        return _version;
	}

	version_t write_lock(uint64_t gen_id) {
	    version_t _version = version.load(std::memory_order_acquire);
        if ((_version & 1) == 0 && version.compare_exchange_weak(_version, _version + 1)) {
            return _version;
        }
        return 0;
	}

    bool read_unlock(version_t old_version) {
        std::atomic_thread_fence(std::memory_order_acquire);
        version_t new_version = version.load(std::memory_order_acquire);
        return new_version == old_version;
    }

    void write_unlock() {
        version.fetch_add(1, std::memory_order_release);
        return;
    }
};

version_lock lock;

void func4() {
    
    for (int i = 0; i < 100000; i++) {
        while (!lock.write_lock(gen_id)) {}
        int a = i * 277 / 23 / 19 * 17 / 13; 
        counter4++;
        lock.write_unlock();
    }
    
}

int main() {
    
    
    vector<thread> threads4;
    threads4.reserve(static_cast<size_t>(10));
    
    for (int i = 0; i < 10; ++i) {
        threads4.emplace_back(func4);
    }
    for (auto &t : threads4) {
        t.join();
    }

    cout << counter4 << endl;
}
// g++ vertion_lock.cc -std=c++11 -pthread