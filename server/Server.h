#ifndef THESIS_SERVER_H
#define THESIS_SERVER_H

#include <mutex>
#include <shared_mutex>
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
                                                                        coordinator(coordinator),
                                                                        version_processor(this) {
        read_worker = std::thread(&Server::proceed_read, this);
        write_worker = std::thread(&Server::proceed_write, this);
        update_worker = std::thread(&Server::proceed_update, this);

        for (size_t i = 0; i < config::data_size; i++) {
            data[0][i] = 0;
        }
    }

    ~Server() override {
        stop = true;
        if (config::log) std::cout << "destroying server" << std::endl;
        read_q_cv.notify_all();
        write_q_cv.notify_all();
        update_cv.notify_all();
        read_worker.join();
        write_worker.join();
        update_worker.join();
        if (config::log) std::cout << "server destroyed" << std::endl;
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

    void proceed_read() {
        while (!stop) {
            Transaction tx = getReadTransaction();

            if (stop) {
                break;
            }

            std::shared_lock<std::shared_mutex> shared_lock(data_mutex);
            std::vector<uint32_t> res(tx.data.size() * 2); // results are stored as {idx0, val0, idx1, val1, idx2 ...
            for (size_t j = 0; j < tx.data.size() - 1; j++) {
                res[j * 2] = tx.data[j];
                res[j * 2 + 1] = get(tx.data.back(), tx.data[j]);
            }

            Transaction a = Transaction(tx.id, TransactionType::READ_RESPONSE, res);

            send(a);
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

    void proceed_write() {
        while (!stop) {
            Transaction tx = getWriteTransaction();
            if (stop) {
                break;
            }

            version_processor.register_read(tx.data.size());

            std::unique_lock<std::mutex> lock(draft_mutex);
            for (uint32_t & i : tx.data) {
                draft[i] = random::xorshf96();
            }
            lock.unlock();

            send(Transaction(tx.id, TransactionType::WRITE_RESPONSE));
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
        } else if (transaction.type == TransactionType::WRITE_ONLY) {
            std::lock_guard<std::mutex> lock(write_ts_mutex);
            write_ts.push(transaction);
            write_q_cv.notify_one();
        } else if (transaction.type == TransactionType::UPDATE) {
            std::lock_guard<std::mutex> lock(update_mutex);
            updating = true;
            update_cv.notify_one();
        }
    }

    void proceed_update() {
        while (!stop) {
            std::unique_lock<std::mutex> update_lock(update_mutex);
            if (!updating) {
                update_cv.wait(update_lock);
            }
            update_lock.unlock();

            if (stop) return;

            std::unique_lock<std::shared_mutex> data_lock(data_mutex);
            std::unique_lock<std::mutex> draft_lock(draft_mutex);

            if (!draft.empty()) {
                data.emplace_back(std::move(draft));
                draft = std::unordered_map<size_t, uint32_t>();
            }

            draft_lock.unlock();
            data_lock.unlock();

            version_processor.updated();
        }
    }


    NetworkInteractor interactor;
private:
    std::thread update_worker;

    mutable std::mutex update_mutex;
    bool updating = false;
    std::condition_variable update_cv;

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

    std::shared_mutex data_mutex; // does not lock parallel read transactions. Does not allow to write while reading
    std::vector<std::unordered_map<size_t, uint32_t>> data;

    std::mutex draft_mutex; // TODO: make it more concurrent
    std::unordered_map<size_t, uint32_t> draft;


    class VersionProcessor {
    public:
        explicit VersionProcessor(Server *server) : server(server), id(getUid()) {}

        void register_read(uint32_t val = 1) {
            total += val;

            if (should_desire()) {
                sent = true;
                server->send(Transaction(id, TransactionType::UPDATE_DESIRE));
            }
        }

        bool should_desire() {
            return (!sent && total >= config::server_update_limit);
        }

        void updated() {
            total = 0;
            sent = false;
            server->updating = false;
            server->send(Transaction(id, TransactionType::UPDATED));
        }

    private:
        std::atomic<uint32_t> total = 0;
        std::atomic<bool> sent = false;
        Server *server;
        uint32_t id;
    };

    VersionProcessor version_processor;
};

#endif //THESIS_SERVER_H
