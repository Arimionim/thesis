#ifndef THESIS_CLIENT_H
#define THESIS_CLIENT_H

#include "../coordinator/CoordinatorNetworkInteractor.h"
#include "ClientNetworkInteractor.h"
#include "RequestGenerator.h"
#include "../messages/Transaction.h"
#include <vector>
#include <cstddef>
#include <chrono>
#include <thread>

class Client {
public:
    explicit Client(CoordinatorNetworkInteractor *coordinator) : interactor(coordinator) {}

    // wait *delay*, then sends all transactions from load to coordinator with interval *interval*
    void startLoad(size_t interval, size_t delay = 0) { // ms
        if (delay > 0) {
            sleep(delay);
        }

        for (const auto &tr: load) {
            interactor.send(tr);
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

private:
    std::vector<Transaction> load;

    ClientNetworkInteractor interactor;
};

#endif //THESIS_CLIENT_H
