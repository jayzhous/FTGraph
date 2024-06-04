#ifndef _NTGRAPH_COMMON_BIT_UTIL_H_
#define _NTGRAPH_COMMON_BIT_UTIL_H_

struct bitset_ui32_t {
    uint32_t b;

    bitset_ui32_t() {
        b &= 0;
    }

    inline void reset() {
        b &= 0;
    }

    inline uint32_t to_uint32() {
        return b;
    }

    inline uint32_t get_num_1bit() {
        return __builtin_popcount(b);
    }

    // scan bits from right to left
    inline int first_zero() {
        return __builtin_ffs(~b) - 1;
    }

    // set the bit of position k to 1
    inline void set(int k) {
        b |= (1 << k);
    }
    // get the bit of position k, return 1 or 0;
    inline int get(int k) {
        return (b & (1 << k)) > 0 ? 1 : 0;
    }

    // set the bit of position k to 0
    inline void clear(int k) {
        b &= ~(1 << k);
    }
};

struct bitset {
    uint64_t b;

    bitset() {
        b &= 0;
    }

    inline void reset() {
        b &= 0;
    }

    inline uint32_t get_num_1bit() {
        return __builtin_popcountll(b);
    }

    inline uint64_t to_uint64() {
        return b;
    }

    inline uint32_t to_uint32() {
        return (uint32_t) (b & UINT32);
    }

    inline uint32_t to_uint16() {
        return (uint32_t) (b & UINT16);
    }

    // scan bits from right to left
    inline int first_zero() {
        return __builtin_ffsll(~b) - 1;
    }

    // set the bit of position k to 1
    inline void set(int k) {
        b |= (1ull << k);
    }
    // get the bit of position k, return 1 or 0;
    inline int get(int k) {
        return (b & (1ull << k)) > 0 ? 1 : 0;
    }

    inline void update(uint64_t new_bm) {
        uint64_t old_bm = b;
        while (!__sync_bool_compare_and_swap(&b, old_bm, new_bm)){
            old_bm = b;
        }
    }

    // set the bit of position k to 0
    inline void clear(int k) {
        b &= ~(1ull << k);
    }
};

struct atomic_bitset {
    std::atomic<uint64_t> b{0};

    void reset() {
        b = 0;
    }

    inline uint64_t to_uint64() {
        return b.load(std::memory_order_acquire);
    }
    inline uint32_t to_uint32() {
        return (uint32_t) (b.load(std::memory_order_acquire) & UINT32);
    }

    inline uint32_t to_uint16() {
        return (uint32_t) (b.load(std::memory_order_acquire) & UINT16);
    }

    inline int get(int k) {
        uint64_t bm = b.load(std::memory_order_acquire);
        return (bm & (1ull << k)) > 0 ? 1 : 0;
    }
    inline int first_zero() {
        uint64_t bm = b.load(std::memory_order_acquire);
        return __builtin_ffsll(~bm) - 1;
    }
    inline uint32_t get_num_1bit() {
        return __builtin_popcountll(b.load(std::memory_order_acquire));
    }
    void clear(int pos) {
        while (1) {
            uint64_t old_bm = b.load(std::memory_order_acquire);
            uint64_t new_bm = old_bm;
            new_bm &= ~(1ull << pos);

            if (!b.compare_exchange_weak(old_bm, new_bm)){
                continue;
            }
            break;
        }
    }

    int set_first_zero() {
        while (1) {
            uint64_t old_bm = b.load(std::memory_order_acquire);
            int pos = __builtin_ffsll(~old_bm) - 1;  // 0 ~ 63/31/15
            if (pos == -1) return -1;
            uint64_t new_bm = old_bm;
            new_bm |= (1ull << pos);

            if (!b.compare_exchange_weak(old_bm, new_bm)){
                continue;
            }
            return pos;
        }
    }

    int test_and_set_first_zero(uint64_t old, bool& changed) {
        
        uint64_t bm = b.load(std::memory_order_acquire);
        int pos = __builtin_ffsll(~bm) - 1;  // 0 ~ 63/31/15
        if (pos == -1) return -1;
        uint64_t new_bm = bm;
        new_bm |= (1ull << pos);

        
        changed = b.compare_exchange_weak(old, new_bm);
        return pos;
        
    }

    void set(int pos) {
        while (1) {
            uint64_t old_bm = b.load(std::memory_order_acquire);
            uint64_t new_bm = old_bm;
            new_bm |= (1ull << pos);

            if (!b.compare_exchange_weak(old_bm, new_bm)){
                continue;
            }
            break;
        }
    }
};

static unsigned int get_bits(unsigned int number, int start, int num_bits) {
    // 0 <= start 
    unsigned int k = 0;
    for (int i = 0; i < num_bits; i++) {
        k |= (1 << (start + i));
    }
    return (number & k) >> start;
}

#endif