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
    explicit Client(NetworkInteractor *coordinator) : coordinator(coordinator), interactor(this) {}

    // wait *delay*, then sends all transactions from load to coordinator with interval *interval*
    void startLoad(size_t interval, size_t delay = 0) { // ms
        if (delay > 0) {
            sleep(delay);
        }

        for (const auto &tr: load) {
            times[tr.id] = timeSinceEpochMs();
       //     std::cout << "client sent " << tr.id << std::endl;
            sent.insert(tr.id);
            interactor.send(coordinator, tr);
            sleep(interval);
        }

        wait_finish();
        std::cout << "finished! " << std::endl;
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
      //  std::cout << "client receive " << transaction.id << std::endl;
        if (transaction.type == TransactionType::READ_RESPONSE) {
            std::lock_guard<std::mutex> lock(sent_mutex);
            sent.erase(sent.find(transaction.id));
            sent_cv.notify_one();
            delays.push_back(timeSinceEpochMs() - times[transaction.id]);
            std::cout << delays.back() << std::endl;
        }
    }

    NetworkInteractor interactor;
private:
    void wait_finish() {
        std::unique_lock<std::mutex> lock(sent_mutex);
        sent_cv.wait(lock, [this] { return sent.empty(); });
    }

    mutable std::mutex sent_mutex;
    std::condition_variable sent_cv;

    std::unordered_set<size_t> sent;
    std::vector<Transaction> load;
    std::unordered_map<size_t, uint64_t> times;
    std::vector<uint64_t> delays;
    NetworkInteractor *coordinator;
};

#endif //THESIS_CLIENT_H
