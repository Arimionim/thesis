#ifndef THESIS_COORDINATOR_H
#define THESIS_COORDINATOR_H

#include "../messages/Transaction.h"
#include <queue>
#include <mutex>
#include <thread>
#include "../environment.h"
#include "../Utils.h"
#include <condition_variable>
#include <unordered_map>
#include "CoordinatorNetworkInteractor.h"

class Coordinator {

public:
    Coordinator() : interactor(this) {
        workers.resize(config::coordinator_worker_number);
        for (size_t i = 0; i < config::coordinator_worker_number; i++) {
            workers[i] = std::thread(proceed_tx, this, i);
        }
    }

    void receive(Transaction transaction) {
        std::lock_guard<std::mutex> lock(ts_mutex);
        ts.push(std::move(transaction));
        q_cv.notify_one();
    }

private:

    Transaction getTransaction() {
        std::unique_lock<std::mutex> lock(ts_mutex);
        q_cv.wait(lock);
        Transaction res = std::move(ts.front());
        ts.pop();
        lock.unlock();
        return res;
    }

    static void proceed_tx(Coordinator *coordinator, int idx) {
        while (1) {
            Transaction tx = coordinator->getTransaction();
            std::vector<std::pair<size_t, Transaction>> splitted = split_tx(tx);


        }
    }

    // | | | | | | | | | |
    // 0 1 2 3 4 5 6 7 8 9
    // _____ _____ _______
    // 0     1     2

    static std::vector<std::pair<size_t, Transaction>> split_tx(const Transaction &tx) {
        std::unordered_map<size_t, std::vector<size_t>> res_map;
        for (size_t idx: tx.indexes) {
            res_map[std::min(idx / config::servers_number, config::servers_number - 1)].push_back(idx);
        }

        std::vector<std::pair<size_t, Transaction>> res;
        res.reserve(res_map.size());

        for (auto &a: res_map) {
            Transaction tmp(tx.type);
            tmp.indexes = std::move(a.second);
            res.emplace_back(a.first, tmp);
        }

        return res;
    }

    std::vector<std::thread> workers;
    std::condition_variable q_cv;
    std::queue<Transaction> ts;
    mutable std::mutex ts_mutex;
    CoordinatorNetworkInteractor interactor;
    std::mutex m;
};

#endif //THESIS_COORDINATOR_H
