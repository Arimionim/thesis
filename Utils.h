#ifndef THESIS_UTILS_H
#define THESIS_UTILS_H

#include <thread>
#include <functional>
#include <atomic>

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
    if (ms > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

static size_t getUid() {
    static std::atomic<std::size_t> uid { 0 };  // <<== initialised
//    uid = 0;    <<== removed
    return ++uid;
}

uint64_t timeSinceEpochMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}


#endif //THESIS_UTILS_H
