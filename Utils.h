#ifndef THESIS_UTILS_H
#define THESIS_UTILS_H

#include <thread>

namespace random {
    static uint32_t x = 123456789, y = 362436069, z = 521288629;

    static uint32_t rand_max = UINT32_MAX;

    uint32_t xorshf96() {          //period 2^96-1. Simple and fast random
        uint32_t t;
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;

        return z;
    }
}

static void sleep(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


#endif //THESIS_UTILS_H
