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

    ~Coordinator() override {
        stop = true;
        q_cv.notify_all();
        for (auto &w: workers) {
            w.join();
        }
    }

    void receive(NetworkInteractor *sender, Transaction transaction) override {
        if (config::log) std::cout << "coor received " << transaction.id << std::endl;
        std::lock_guard<std::mutex> lock(ts_mutex);

        if (transaction.type == TransactionType::READ_ONLY ||
            transaction.type == TransactionType::WRITE_ONLY) {
            std::unique_lock<std::mutex> sent_lock(sent_mutex);
            sent_transaction[transaction.id] = {sender, 0};
            sent_lock.unlock();
        }
        ts.push(std::move(transaction));
        q_cv.notify_one();
    }

    void setServers(std::vector<NetworkInteractor *> servers_) {
        this->servers = std::move(servers_);
    }

    bool isStopped() {
        return stop;
    }

    NetworkInteractor interactor;
    size_t version = 0;
private:

    Transaction getTransaction() {
        std::unique_lock<std::mutex> lock(ts_mutex);
        if (ts.empty()) {
            q_cv.wait(lock);
        }

        if (isStopped()) {
            return Transaction(-1, TransactionType::READ_ONLY); // dummy transaction
        }

        Transaction res = std::move(ts.front());
        ts.pop();
        lock.unlock();
        return res;
    }

    NetworkInteractor *find_server(size_t idx) {
        return servers[idx];
    }

    static void proceed_tx(Coordinator *coordinator, int idx) {
        while (!coordinator->isStopped()) {
            Transaction tx = coordinator->getTransaction();
            if (coordinator->isStopped()) {
                break;
            }
            if (tx.type == TransactionType::READ_ONLY || tx.type == TransactionType::WRITE_ONLY) {
                std::vector<std::pair<size_t, Transaction>> splitted = split_tx(tx);
                std::unique_lock<std::mutex> lock(coordinator->sent_mutex);

                coordinator->sent_transaction[tx.id].pieces = splitted.size();
                lock.unlock();

                for (auto &a: splitted) {
                    if (a.second.type == TransactionType::READ_ONLY) {
                        a.second.data.push_back(coordinator->version);
                    }

                    coordinator->interactor.send(coordinator->find_server(a.first), a.second);
                }
            } else if (tx.type == TransactionType::READ_RESPONSE) {
                std::unique_lock<std::mutex> lock(coordinator->sent_mutex);
                sent_record &history = coordinator->sent_transaction[tx.id];

                history.pieces--;
                for (auto p: tx.data) {
                    history.response.push_back(p);
                }


                if (history.pieces == 0) {
                    auto sender = history.sender;

                    if (config::log) std::cout << "coor sends " << tx.id << std::endl;
                   // std::cout << "piece send " << tx.id << ' ' << tx.type << ' ' << history.response[0] << ' ' << history.response.size() << std::endl;
                    coordinator->interactor.send(sender,
                                                 Transaction(tx.id, tx.type, history.response));

                    coordinator->sent_transaction.erase(coordinator->sent_transaction.find(tx.id));
                }
                lock.unlock();

            }
        }
    }

    // | | | | | | | | | |
    // 0 1 2 3 4 5 6 7 8 9
    // _____ _____ _______
    // 0     1     2

    static std::vector<std::pair<size_t, Transaction>> split_tx(const Transaction &tx) {
        std::unordered_map<size_t, std::vector<uint32_t>> res_map;
        for (uint32_t idx: tx.data) {
            res_map[idx / config::data_size].push_back(idx);
        }

        std::vector<std::pair<size_t, Transaction>> res;
        res.reserve(res_map.size());

        for (auto &a: res_map) {
            Transaction tmp(tx.id, tx.type, std::move(a.second));
            res.emplace_back(a.first, tmp);
        }

        return res;
    }

    std::atomic<bool> stop = false;

    class sent_record {
    public:
        NetworkInteractor *sender = nullptr;
        size_t pieces = 3000;
        std::vector<uint32_t> response = std::vector<uint32_t>();
    };
    mutable std::mutex sent_mutex;
    std::unordered_map<size_t, sent_record> sent_transaction; // TODO: not thread safe

    std::ofstream logger;
    std::vector<NetworkInteractor *> servers;
    std::vector<std::thread> workers;
    std::condition_variable q_cv;
    std::queue<Transaction> ts;
    mutable std::mutex ts_mutex;
    std::mutex m;
};

#endif //THESIS_COORDINATOR_H
