#ifndef THESIS_ENVIRONMENT_H
#define THESIS_ENVIRONMENT_H

#include <cstddef>

namespace config {
    size_t servers_number = 5;
    size_t data_size = 1000; // per server
    size_t server_update_limit = data_size * 2;
    size_t desire_update_limit = servers_number;
    size_t transaction_size = data_size / 10;
    size_t coordinator_worker_number = 1;
    size_t server_worker_number = 1;
    size_t server_read_worker_number = 1;
    size_t clients_number = 50;
    const bool log = false;
} // 20 * 300 = 6000 / 5 = 1200

#endif //THESIS_ENVIRONMENT_H
