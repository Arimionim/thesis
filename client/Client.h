#ifndef THESIS_CLIENT_H
#define THESIS_CLIENT_H

#include "RequestGenerator.h"
#include "../messages/Transaction.h"
#include "../network/NetworkInteractor.h"
#include <vector>
#include <cstddef>
#include <chrono>
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
            interactor.send(coordinator, tr);
            sleep(interval);
        }
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
    }

    NetworkInteractor interactor;
private:
    std::vector<Transaction> load;

    NetworkInteractor *coordinator;
};

#endif //THESIS_CLIENT_H
