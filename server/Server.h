#ifndef THESIS_SERVER_H
#define THESIS_SERVER_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include "../messages/Transaction.h"
#include "../environment.h"
#include "ServerNetworkInteractor.h"

class Server {
public:
    explicit Server(size_t data_size) : data_size(data_size),
                                        data(1, std::vector<int>(data_size, 0)),
                                        interactor(this) {
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

    static void proceed_read(Server *server, int i) {
        
    }

    static void proceed_write(Server *server, int i) {

    }

private:
    size_t data_size;
    std::vector<std::thread> read_workers;
    std::vector<std::thread> write_workers;

    mutable std::mutex read_ts_mutex;
    std::queue<Transaction> read_ts;
    std::condition_variable read_q_cv;

    mutable std::mutex write_ts_mutex;
    std::queue<Transaction> write_ts;
    std::condition_variable write_q_cv;

    ServerNetworkInteractor interactor;
    std::vector<std::vector<int>> data;
    std::vector<int> draft;
};

#endif //THESIS_SERVER_H
