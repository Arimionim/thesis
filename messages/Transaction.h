#ifndef THESIS_TRANSACTION_H
#define THESIS_TRANSACTION_H

#include <cstddef>
#include <vector>
#include "TransactionType.h"

class Transaction {
public:

    explicit Transaction(TransactionType type) : type(type) {}

    std::vector<size_t> indexes; // if transaction is write we will write random values
    const TransactionType type;
};

#endif //THESIS_TRANSACTION_H
