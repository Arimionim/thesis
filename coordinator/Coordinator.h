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
     //   std::cout << "coor received " << transaction.id << std::endl;
        std::lock_guard<std::mutex> lock(ts_mutex);
        if (transaction.type == TransactionType::READ_ONLY ||
            transaction.type == TransactionType::WRITE_ONLY) {
            sent_transaction[transaction.id] = {sender, 0};
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
                coordinator->sent_transaction[tx.id].pieces = splitted.size();
//                std::cout << tx.id << ' ' << "set " << coordinator->sent_transaction[tx.id].pieces << std::endl;
                for (auto &a: splitted) {
                    if (a.second.type == TransactionType::READ_ONLY) {
                        a.second.data.push_back(coordinator->version);
                    }

                    coordinator->interactor.send(coordinator->find_server(a.first), a.second);
                }
            } else if (tx.type == TransactionType::READ_RESPONSE) {
                sent_record &history = coordinator->sent_transaction[tx.id];
           //     std::cout << "history: " << history.sender << ' ' << history.pieces << ' ' << history.response.size() << std::endl;

                history.pieces--;
                for (auto p: tx.data) {
                    history.response.push_back(p);
                }


                if (coordinator->sent_transaction[tx.id].pieces == 0) {
        //            std::cout << "coor sends " << tx.id << std::endl;
                    coordinator->interactor.send(coordinator->sent_transaction[tx.id].sender,
                                                 Transaction(tx.id, tx.type, history.response));
                }
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
            Transaction tmp(tx.id, tx.type, std::move(a.second));
            res.emplace_back(a.first, tmp);
        }

        return res;
    }

    bool stop = false;

    class sent_record {
    public:
        NetworkInteractor *sender;
        size_t pieces = 3000;
        std::vector<size_t> response;
    };

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
