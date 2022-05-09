#ifndef THESIS_COORDINATORNETWORKINTERACTOR_H
#define THESIS_COORDINATORNETWORKINTERACTOR_H

#include "Coordinator.h"
#include "../server/ServerNetworkInteractor.h"

class CoordinatorNetworkInteractor {
public:
    explicit CoordinatorNetworkInteractor(Coordinator *coordinator) : coordinator(coordinator) {}

    void receive(Transaction transaction) {
        coordinator->receive(std::move(transaction));
    }

    void send(ServerNetworkInteractor *server, const Transaction &tx) {
        server->receive(tx);
    }

private:
    Coordinator *coordinator;
};

#endif //THESIS_COORDINATORNETWORKINTERACTOR_H
