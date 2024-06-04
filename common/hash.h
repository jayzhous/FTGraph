#ifndef _NTGRAPH_COMMON_HASH_H_
#define _NTGRAPH_COMMON_HASH_H_

// 将一个 8B 的数值哈希成 1B
static inline unsigned char hashfunc(uint64_t x) {
    x ^= x>>32;  // x / 2^32
    x ^= x>>16;
    x ^= x>>8;
    return (unsigned char)(x & 0x0ffULL); //0x0ffULL:16进制，0一个多个或者没有都不影响，ULL = unsigned long long，表示只取最后8bit，就是1个char
}

static inline unsigned char hashcode(unsigned int x) {
    return (unsigned char)(x % 256);
}



#endif //_NTGRAPH_COMMON_HASH_H_