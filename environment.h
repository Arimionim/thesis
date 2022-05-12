#ifndef THESIS_ENVIRONMENT_H
#define THESIS_ENVIRONMENT_H

#include <cstddef>

namespace config {
    size_t data_size = 10000; // per server
    size_t transaction_size = 100;
    size_t servers_number = 10;
    size_t coordinator_worker_number = 1;
    size_t server_worker_number = 1;
    size_t server_read_worker_number = 1;
    size_t clients_number = 5;
    const bool log = false;
}

#endif //THESIS_ENVIRONMENT_H
