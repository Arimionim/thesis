#ifndef THESIS_TRANSACTION_H
#define THESIS_TRANSACTION_H

#include <cstddef>
#include <utility>
#include <vector>
#include "TransactionType.h"

class Transaction {
public:

    explicit Transaction(size_t id, TransactionType type, std::vector<size_t> data = std::vector<size_t>())
            : id(id), type(type), data(std::move(data)) { }

    /*
     * READ_ONLY:     data = {read_idx_0, read_idx_1, ..., version}
     * WRITE_ONLY:    data = {write_idx_0, write_idx_1, ...} // write transaction writes random values
     * READ_RESPONSE: data = {read_idx_0, value_0, read_idx_1, value_1, read_idx_2, ...}
     */

    size_t id = -1;
    std::vector<size_t> data; // if transaction is write we will write random values
    const TransactionType type;
};

#endif //THESIS_TRANSACTION_H
