#ifndef THESIS_CLIENT_H
#define THESIS_CLIENT_H

#include "RequestGenerator.h"
#include "../messages/Transaction.h"
#include "../network/NetworkInteractor.h"
#include <vector>
#include <cstddef>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <thread>

class Client : public Receiver {
public:
    explicit Client(NetworkInteractor *coordinator, size_t id) : id(id), coordinator(coordinator), interactor(this) {}

    // wait *delay*, then sends all transactions from load to coordinator with interval *interval*
    void startLoad(size_t interval, size_t delay = 0) { // ms
        if (delay > 0) {
            sleep(delay);
        }

        for (const auto &tr: load) {
            if (config::log) std::cout << "client sent " << tr.id << std::endl;
            std::unique_lock<std::mutex> lock(sent_mutex);
            times[tr.id] = timeSinceEpochMs();
            sent.insert(tr.id);
            lock.unlock();
            interactor.send(coordinator, tr);
            if (interval > 0)
                sleep(interval);
        }

        wait_finish();
        if (config::log) std::cout << "finished! " << std::endl;

        double avg = 0;
        for (auto v: delays_w) {
            avg += v;
        }
        logger::add_w((avg / delays_w.size()));

        avg = 0;
        for (auto v: delays_r) {
            avg += v;
        }
        logger::add_r((avg / delays_r.size()));


        // std::cout << id << ' ' << (avg / delays.size()) << std::endl;
    }

    void addLoad(Transaction tr) {
        load.push_back(tr);
    }

    /*
     * generate *number* requests, where write_fraction * 100 percents are write transactions
     * write_fraction = [0,1]
     * adds generated transactions to load
     */
    void addLoad(size_t number, double write_fraction) {
        auto newLoad = RequestGenerator::generate(number, write_fraction);

        load.reserve(load.size() + newLoad.size());
        for (const auto &tr: newLoad) { // TODO: check if it can be optimized
            load.push_back(tr);
        }
    }

    void clearLoad() {
        load.clear();
    }

    void receive(NetworkInteractor *sender, Transaction transaction) override {
        if (config::log) std::cout << "client receive " << transaction.id << std::endl;
        if (transaction.type == TransactionType::READ_RESPONSE ||
            transaction.type == TransactionType::WRITE_RESPONSE) {
            std::unique_lock<std::mutex> lock(sent_mutex);
            sent.erase(sent.find(transaction.id));
            lock.unlock();
            sent_cv.notify_all();
            (transaction.type == TransactionType::READ_RESPONSE ? delays_r : delays_w)
                    .push_back(timeSinceEpochMs() - times[transaction.id]);
            if (config::log) std::cout << transaction.id << ' ' << (transaction.type == TransactionType::READ_RESPONSE ? delays_r : delays_w).back() << std::endl;
        }
    }

    NetworkInteractor interactor;
private:
    void wait_finish() {
        std::unique_lock<std::mutex> lock(sent_mutex);
        sent_cv.wait(lock, [this] {
            return sent.empty();
        });
    }

    mutable std::mutex sent_mutex;
    std::condition_variable sent_cv;
    size_t id;
    std::unordered_set<size_t> sent;
    std::vector<Transaction> load;
    std::unordered_map<size_t, uint64_t> times;
    std::vector<uint64_t> delays_w;
    std::vector<uint64_t> delays_r;
    NetworkInteractor *coordinator;
};

#endif //THESIS_CLIENT_H
