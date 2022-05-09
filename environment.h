#ifndef THESIS_ENVIRONMENT_H
#define THESIS_ENVIRONMENT_H

#include <cstddef>

namespace config {
    size_t data_size = 10000;
    size_t transaction_size = 100;
    size_t servers_number = 10; // each server will hold data_size / servers_number values. Last server will store remaining
    size_t coordinator_worker_number = 2;
    size_t server_worker_number = 2;
    size_t server_read_worker_number = 2;
}

#endif //THESIS_ENVIRONMENT_H
