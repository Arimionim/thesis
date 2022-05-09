#ifndef THESIS_CLIENTNETWORKINTERACTOR_H
#define THESIS_CLIENTNETWORKINTERACTOR_H

#include "../coordinator/CoordinatorNetworkInteractor.h"
#include "../messages/Transaction.h"
#include "../messages/MessageType.h"

class ClientNetworkInteractor {
public:
    explicit ClientNetworkInteractor(CoordinatorNetworkInteractor *coordinator) : coordinator(coordinator) {}

    void send(const Transaction &transaction) {
        coordinator->receive(transaction); // first approach. May be more abstract with serialization and MessageType
    }

private:
    CoordinatorNetworkInteractor *coordinator;
};

#endif //THESIS_CLIENTNETWORKINTERACTOR_H
