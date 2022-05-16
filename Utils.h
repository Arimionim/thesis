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
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

namespace logger {

    std::mutex mutex;

    std::vector<double> all_w;
    std::vector<double> all_r;

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        all_w.clear();
        all_r.clear();
    }

    void add_w(double s) {
        return;
        std::lock_guard<std::mutex> lock(mutex);
        all_w.push_back(s);
    //    std::cout << s << std::endl;
    }

    void add_r(double s) {
        std::lock_guard<std::mutex> lock(mutex);
        all_r.push_back(s);
        //    std::cout << s << std::endl;
    }

    double avg_w() {
        std::lock_guard<std::mutex> lock(mutex);

        if (all_w.empty()) {
            return 0;
        }

        double avg = 0;
        for (auto v: all_w) {
            avg += v;
        }

        return avg / all_w.size();
    }

    double avg_r() {
        std::lock_guard<std::mutex> lock(mutex);

        if (all_r.empty()) {
            return 0;
        }

    }

    struct res {
        double avg;
        double m50, m90, m95;
    };

    res read_res() {
        std::lock_guard<std::mutex> lock(mutex);

        if (all_r.empty()) {
            return {0, 0, 0};
        }

        double avg = 0;
        for (auto v: all_r) {
            avg += v;
        }

         ;

        std::sort(all_r.begin(), all_r.end());
        return {avg / all_r.size(), all_r[all_r.size() * 0.5], all_r[all_r.size() * 0.90], all_r[all_r.size() * 0.95]};
    }


    std::vector<double> timing_c;
    std::vector<double> timing_s;
}


#endif //THESIS_UTILS_H
