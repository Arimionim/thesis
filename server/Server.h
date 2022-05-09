#ifndef THESIS_SERVER_H
#define THESIS_SERVER_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include "../messages/Transaction.h"
#include "../environment.h"
#include "../network/NetworkInteractor.h"

class Server : public Receiver {
public:
    explicit Server(NetworkInteractor *coordinator, size_t data_size) : data_size(data_size),
                                                                        data(1, std::unordered_map<size_t, int>()),
                                                                        interactor(this),
                                                                        coordinator(coordinator) {

        for (size_t i = 0; i < config::data_size; i++) {
            data[0][i] = 0;
        }

        read_workers.resize(config::server_read_worker_number);
        for (size_t i = 0; i < config::server_read_worker_number; i++) {
            read_workers[i] = std::thread(proceed_read, this, i);
        }

        write_workers.resize(config::server_worker_number);
        for (size_t i = 0; i < config::server_worker_number; i++) {
            write_workers[i] = std::thread(proceed_write, this, i);
        }
    }

    void receive(Transaction transaction) {
        if (transaction.type == TransactionType::READ_ONLY) {
            std::lock_guard<std::mutex> lock(read_ts_mutex);
            read_ts.push(std::move(transaction));
            read_q_cv.notify_one();
        } else {
            std::lock_guard<std::mutex> lock(write_ts_mutex);
            write_ts.push(std::move(transaction));
            write_q_cv.notify_one();
        }
    }

    Transaction getReadTransaction() {
        std::unique_lock<std::mutex> lock(read_ts_mutex);
        read_q_cv.wait(lock);
        Transaction res = std::move(read_ts.front());
        read_ts.pop();
        lock.unlock();
        return res;
    }

    void send(const Transaction &transaction) {
        interactor.send(coordinator, std::move(transaction));
    }

    static void proceed_read(Server *server, int i) {
        while (1) {
            Transaction tx = server->getReadTransaction();

            std::vector<size_t> res(tx.data.size() * 2); // results are stored as {idx0, val0, idx1, val1, idx2 ...
            for (size_t j = 0; j < tx.data.size() - 1; j++) {
                res[j * 2] = tx.data[j];
                res[j * 2 + 1] = server->get(tx.data.back(), tx.data[j]);
            }

            Transaction a = Transaction(TransactionType::READ_RESPONSE, res);

            server->send(Transaction(TransactionType::READ_RESPONSE, res));
        }
    }

    Transaction getWriteTransaction() {
        std::unique_lock<std::mutex> lock(write_ts_mutex);
        write_q_cv.wait(lock);
        Transaction res = std::move(write_ts.front());
        write_ts.pop();
        lock.unlock();
        return res;
    }

    static void proceed_write(Server *server, int i) {
        while (1) {
            Transaction tx = server->getWriteTransaction();
        }
    }

    int get(size_t version, size_t idx) {
        for (size_t v = version; v >= 0; v--) {
            if (exists(v, idx)) {
                return data[v][idx];
            }
        }
    }

    bool exists(size_t v, size_t idx) {
        return data[v].find(idx) != data[v].end();
    }

    void receive(NetworkInteractor *sender, Transaction transaction) override {

    }

    NetworkInteractor interactor;
private:
    NetworkInteractor *coordinator;

    size_t data_size;
    std::vector<std::thread> read_workers;
    std::vector<std::thread> write_workers;

    mutable std::mutex read_ts_mutex;
    std::queue<Transaction> read_ts;
    std::condition_variable read_q_cv;

    mutable std::mutex write_ts_mutex;
    std::queue<Transaction> write_ts;
    std::condition_variable write_q_cv;

    std::vector<std::unordered_map<size_t, int>> data;
    std::unordered_map<size_t, int> draft;
};

#endif //THESIS_SERVER_H
