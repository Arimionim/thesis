#ifndef THESIS_NETWORKINTERACTOR_H
#define THESIS_NETWORKINTERACTOR_H

#include "../messages/Transaction.h"
#include "Receiver.h"


class NetworkInteractor {
public:
    explicit NetworkInteractor(Receiver *receiver) : owner(receiver) { }

    ~NetworkInteractor() {
        stop = true;
        q_cv.notify_all();
        worker.join();
    }

    void receive(NetworkInteractor *sender, const Transaction &transaction) {
        owner->receive(sender, transaction);
    }

    void send(NetworkInteractor *receiver, Transaction tx) {
//        std::cout << "sending " << tx.id << ' ' << owner << "->" << receiver << std::endl;
        std::lock_guard<std::mutex> lock(ts_mutex);
        ts.push({receiver, std::move(tx)});
        q_cv.notify_one();
    }

private:
    std::pair<NetworkInteractor *, Transaction> getTransaction() {
        std::unique_lock<std::mutex> lock(ts_mutex);
        if (ts.empty()) {
            q_cv.wait(lock);
        }

        if (stop) {
            return {nullptr, Transaction(-1, TransactionType::READ_ONLY)}; // dummy transaction
        }

        auto res = std::move(ts.front());
        ts.pop();
        return res;
    }

    static void send_helper(NetworkInteractor *sender) {
        // std::cout << "transaction " << sender << "->" << receiver << std::endl;

        while (!sender->stop) {
            auto tx = sender->getTransaction();
            if (sender->stop) {
                break;
            }
            tx.first->receive(sender, tx.second);
        }
    }

    std::thread worker = std::thread(send_helper, this);
    std::unordered_map<uint32_t, uint32_t> timings;
    std::atomic<bool> stop = false;
    std::condition_variable q_cv;
    std::queue<std::pair<NetworkInteractor *, Transaction>> ts;
    mutable std::mutex ts_mutex;
    Receiver *owner;
};

#endif //THESIS_NETWORKINTERACTOR_H
