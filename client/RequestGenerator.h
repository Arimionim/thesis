#ifndef THESIS_REQUESTGENERATOR_H
#define THESIS_REQUESTGENERATOR_H

#include <vector>
#include <cstddef>
#include "../messages/Transaction.h"
#include "../Utils.h"
#include "../environment.h"

class RequestGenerator {

public:
    static std::vector<Transaction> generate(size_t number, double write_fraction) {
        std::vector<Transaction> res;
        res.reserve(number);

        for (int i = 0; i < number; i++) {
            bool is_write = random::xorshf96() <
                            random::rand_max * write_fraction; // TODO: check if there is a but with range edges
            auto tr = Transaction(is_write ? TransactionType::WRITE_ONLY : TransactionType::READ_ONLY);

            tr.indexes.resize(config::transaction_size);
            for (size_t j = 0; j < config::transaction_size; j++) {
                tr.indexes[j] = random::xorshf96() % config::data_size;
            }

            res.push_back(tr);
        }

        return res;
    }
};

#endif //THESIS_REQUESTGENERATOR_H
