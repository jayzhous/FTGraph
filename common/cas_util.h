#ifndef _NTGRAPH_COMMON_CAS_UTIL_H_
#define _NTGRAPH_COMMON_CAS_UTIL_H_

#include <iostream>

template <class ET>
inline bool CAS(ET *ptr, ET oldv, ET newv) {
    if (sizeof(ET) == 1) {
        return __sync_bool_compare_and_swap((bool*)ptr, *((bool*)&oldv), *((bool*)&newv));
    } else if (sizeof(ET) == 4) {
        return __sync_bool_compare_and_swap((int*)ptr, *((int*)&oldv), *((int*)&newv));
    } else if (sizeof(ET) == 8) {
        return __sync_bool_compare_and_swap((long*)ptr, *((long*)&oldv), *((long*)&newv));
    } 
    else {
        std::cout << "CAS bad length : " << sizeof(ET) << std::endl;
        abort();
    }
}

template <class ET>
inline bool writeMin(ET *a, ET b) {
    ET c; bool r=0;
    do c = *a;
    while (c > b && !(r=CAS(a,c,b)));
    return r;
}

#endif