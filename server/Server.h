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
    explicit Server(NetworkInteractor *coordinator, size_t data_size) : data(1, std::unordered_map<size_t, uint32_t>()),
                                                                        interactor(this),
                                                                        coordinator(coordinator) {
        read_worker = std::thread(proceed_read, this);
        write_worker = std::thread(proceed_write, this);

        for (size_t i = 0; i < config::data_size; i++) {
            data[0][i] = 0;
        }
    }

    ~Server() override {
        stop = true;
        read_q_cv.notify_all();
        write_q_cv.notify_all();
        read_worker.join();
        write_worker.join();
    }

    void send(const Transaction &transaction) {
        if (config::log) std::cout << "server sent " << transaction.id << std::endl;
        interactor.send(coordinator, transaction);
    }


    Transaction getReadTransaction() {
        std::unique_lock<std::mutex> lock(read_ts_mutex);
        if (read_ts.empty()) {
            read_q_cv.wait(lock);
        }

        if (stop) {
            return Transaction(-1, TransactionType::READ_ONLY);
        }

        Transaction res = std::move(read_ts.front());
        read_ts.pop();
        lock.unlock();
        return res;
    }

    static void proceed_read(Server *server) {
        while (!server->stop) {
            Transaction tx = server->getReadTransaction();

            if (server->stop) {
                break;
            }

            std::vector<uint32_t> res(tx.data.size() * 2); // results are stored as {idx0, val0, idx1, val1, idx2 ...
            for (size_t j = 0; j < tx.data.size() - 1; j++) {
                res[j * 2] = tx.data[j];
                res[j * 2 + 1] = server->get(tx.data.back(), tx.data[j]);
            }

            Transaction a = Transaction(tx.id, TransactionType::READ_RESPONSE, res);

            server->send(a);
        }
    }

    Transaction getWriteTransaction() {
        std::unique_lock<std::mutex> lock(write_ts_mutex);
        if (write_ts.empty()) {
            write_q_cv.wait(lock);
        }

        if (stop) {
            return Transaction(-1, TransactionType::WRITE_ONLY);
        }

        Transaction res = std::move(write_ts.front());
        write_ts.pop();
        lock.unlock();
        return res;
    }

    static void proceed_write(Server *server) {
        while (!server->stop) {
            Transaction tx = server->getWriteTransaction();
            if (server->stop) {
                break;
            }
            for (uint32_t & i : tx.data) {
                server->draft[i] = random::xorshf96();
            }

            server->send(Transaction(tx.id, TransactionType::WRITE_RESPONSE));
        }
    }

    uint32_t get(size_t version, size_t idx) {
        for (int v = version; v >= 0; v--) {
            if (exists(v, idx)) {
                return data[v][idx];
            }
        }

        return 0;
    }

    bool exists(size_t v, size_t idx) {
        return data[v].find(idx) != data[v].end();
    }

    void receive(NetworkInteractor *sender, Transaction transaction)
    override {
        if (config::log) std::cout << "serv received " << transaction.id << std::endl;
        if (transaction.type == TransactionType::READ_ONLY) {
            std::lock_guard<std::mutex> lock(read_ts_mutex);
            read_ts.push(transaction);
            read_q_cv.notify_one();
        } else {
            std::lock_guard<std::mutex> lock(write_ts_mutex);
            write_ts.push(transaction);
            write_q_cv.notify_one();
        }
    }

    NetworkInteractor interactor;
private:
    NetworkInteractor *coordinator;

    std::atomic<bool> stop = false;

    std::thread read_worker;
    std::thread write_worker;

    mutable std::mutex read_ts_mutex;
    std::queue<Transaction> read_ts;
    std::condition_variable read_q_cv;

    mutable std::mutex write_ts_mutex;
    std::queue<Transaction> write_ts;
    std::condition_variable write_q_cv;

    std::vector<std::unordered_map<size_t, uint32_t>> data;
    std::unordered_map<size_t, uint32_t> draft;
};

#endif //THESIS_SERVER_H
