#ifndef THESIS_TRANSACTION_H
#define THESIS_TRANSACTION_H

#include <cstddef>
#include <utility>
#include <vector>
#include "TransactionType.h"

class Transaction {
public:

    explicit Transaction(TransactionType type, std::vector<size_t> data = std::vector<size_t>())
            : type(type), id(id_counter++), data(std::move(data)) {}

    const size_t id; // unique id of transaction;

    /*
     * READ_ONLY:     data = {read_idx_0, read_idx_1, ..., version}
     * WRITE_ONLY:    data = {write_idx_0, write_idx_1, ...} // write transaction writes random values
     * READ_RESPONSE: data = {read_idx_0, value_0, read_idx_1, value_1, read_idx_2, ...}
     */

    std::vector<size_t> data; // if transaction is write we will write random values
    const TransactionType type;

private:
    static size_t id_counter;
};

#endif //THESIS_TRANSACTION_H
