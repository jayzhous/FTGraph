#ifndef _NTGRAPH_COMMON_VERSION_LOCK_H_
#define _NTGRAPH_COMMON_VERSION_LOCK_H_

#include <atomic>

typedef unsigned long version_t;

class version_lock {
private:
    std::atomic<version_t> version{2};
public:
    // if the system restarts, we can reset the version to 0; 

	version_t read_lock() {
	    version_t _version = version.load(std::memory_order_acquire);
        if ((_version & 1) != 0) { // if _version is an odd number
            return 0;
        }
        return _version;
	}

	version_t write_lock() {
	    version_t _version = version.load(std::memory_order_acquire);
        
        if ((_version & 1) == 0 && version.compare_exchange_weak(_version, _version + 1)) { //_version is an even number
            return _version + 1;
        }
        return 0;
        
	}

    bool read_unlock(version_t old_version) {
        std::atomic_thread_fence(std::memory_order_acquire);
        version_t new_version = version.load(std::memory_order_acquire);
        return new_version == old_version;
    }
    // bool read_unlock_and_write_lock(version_t old_version, bool& changed) {
    //     // if read_unlock succeeds, and write_lock using a cas
    //     // if old_version == new_version, update the version, changed = true
    //     version_t _version = version.load(std::memory_order_acquire);
    //     version.compare_exchange_weak(old_version, _version + 1);

    // }
    void write_unlock() {
        version.fetch_add(1, std::memory_order_release);
        return;
    }
    version_t get_version() {
        return version.load(std::memory_order_acquire);
    }
    void reset_version() {
        version = 2;
    }
};



#endif
