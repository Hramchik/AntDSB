//
// Created by sanek on 05/12/2025.
//

#ifndef ANTDSB_THREADUTILS_H
#define ANTDSB_THREADUTILS_H

#include <thread>
#include <algorithm>
#include <iostream>

inline int calc_thread_count(double ratio = 0.75, int min_threads = 2) {
    unsigned int hw = std::thread::hardware_concurrency();
    if (hw == 0) {
        hw = 4;
    }
    int n = static_cast<int>(hw * ratio);
    n = std::max(min_threads, n);

    std::cout << "[ThreadUtils] HW threads: " << hw
              << ", using: " << n << " (" << ratio * 100 << "%)\n";
    return n;
}

#endif //ANTDSB_THREADUTILS_H