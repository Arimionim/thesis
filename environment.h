#ifndef THESIS_ENVIRONMENT_H
#define THESIS_ENVIRONMENT_H

#include <cstddef>

namespace config {
    size_t servers_number = 2;
    size_t data_size = 10000; // per server
    size_t server_update_limit = data_size * 10;
    size_t desire_update_limit = servers_number;
    size_t transaction_size = std::max(1ull, data_size / 5);
    size_t server_worker_number = 1; // not used
    size_t server_read_worker_number = 1; // not used
    size_t clients_number = 20;
    size_t coordinator_worker_number = std::max(1ull, (servers_number + clients_number) / 4);
    const bool log = false;


    // test vars
    double write_ratio = 0.5;
} // 20 * 300 = 6000 / 5 = 1200

#endif //THESIS_ENVIRONMENT_H
