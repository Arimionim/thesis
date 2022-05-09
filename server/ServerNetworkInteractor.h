#ifndef THESIS_SERVERNETWORKINTERACTOR_H
#define THESIS_SERVERNETWORKINTERACTOR_H

#include "../messages/Transaction.h"
#include "Server.h"

class ServerNetworkInteractor {
public:
    ServerNetworkInteractor(Server *server) : server(server) {}

    void receive(Transaction tx) {
        server->receive(std::move(tx));
    }

private:
    Server *server;
};

#endif //THESIS_SERVERNETWORKINTERACTOR_H
