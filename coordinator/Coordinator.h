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
#include <fstream>
#include <utility>
#include "../network/NetworkInteractor.h"
#include "../server/Server.h"

class Coordinator : public Receiver {
public:

    Coordinator() : interactor(this),
                    logger(std::ofstream("coordinator.txt")) {
        workers.resize(config::coordinator_worker_number);
        for (size_t i = 0; i < config::coordinator_worker_number; i++) {
            workers[i] = std::thread(proceed_tx, this, i);
        }
    }

    void receive(NetworkInteractor *sender, Transaction transaction) override {
        if (transaction.type == TransactionType::READ_ONLY ||
            transaction.type == TransactionType::WRITE_ONLY) {
            std::lock_guard<std::mutex> lock(ts_mutex);
            ts.push(std::move(transaction));
            q_cv.notify_one();
        } else if (transaction.type == TransactionType::READ_RESPONSE) {
            logger << transaction.id << "\n";
        }
    }

    void setServers(std::vector<NetworkInteractor*> servers_) {
        this->servers = std::move(servers_);
    }

    NetworkInteractor interactor;
    size_t version = 0;
private:

    Transaction getTransaction() {
        std::unique_lock<std::mutex> lock(ts_mutex);
        q_cv.wait(lock);
        Transaction res = std::move(ts.front());
        ts.pop();
        lock.unlock();
        return res;
    }

    NetworkInteractor *find_server(size_t idx) {
        return servers[idx];
    }

    static void proceed_tx(Coordinator *coordinator, int idx) {
        while (1) {
            Transaction tx = coordinator->getTransaction();
            std::vector<std::pair<size_t, Transaction>> splitted = split_tx(tx);
            for (auto &a: splitted) {
                if (a.second.type == TransactionType::READ_ONLY) {
                    a.second.data.push_back(coordinator->version);
                }
                coordinator->interactor.send(coordinator->find_server(a.first), a.second);
            }
        }
    }

    // | | | | | | | | | |
    // 0 1 2 3 4 5 6 7 8 9
    // _____ _____ _______
    // 0     1     2

    static std::vector<std::pair<size_t, Transaction>> split_tx(const Transaction &tx) {
        std::unordered_map<size_t, std::vector<size_t>> res_map;
        for (size_t idx: tx.data) {
            res_map[std::min(idx / config::servers_number, config::servers_number - 1)].push_back(idx);
        }

        std::vector<std::pair<size_t, Transaction>> res;
        res.reserve(res_map.size());

        for (auto &a: res_map) {
            Transaction tmp(tx.type);
            tmp.data = std::move(a.second);
            res.emplace_back(a.first, tmp);
        }

        return res;
    }


    std::ofstream logger;
    std::vector<NetworkInteractor *> servers;
    std::vector<std::thread> workers;
    std::condition_variable q_cv;
    std::queue<Transaction> ts;
    mutable std::mutex ts_mutex;
    std::mutex m;
};

#endif //THESIS_COORDINATOR_H
