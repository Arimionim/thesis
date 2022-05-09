#include <iostream>
#include <vector>
#include "Utils.h"

#include "server/Server.h"
#include "coordinator/Coordinator.h"
#include "client/Client.h"


void test1(NetworkInteractor *coordinator) {
    std::vector<Client*> clients;

    for (int i = 0; i < 2; i++) {
        clients.push_back(new Client(coordinator));
    }

    for (auto& c : clients) {
        c->addLoad(100, 0);
        c->startLoad(100, 1000);
    }
}

int main() {
    Coordinator coordinator;

    std::vector<Server*> servers;

    for (int i = 0; i < config::servers_number; i++) {
        servers.push_back(new Server(&coordinator.interactor, config::data_size / config::servers_number +
                (i == config::servers_number - 1) * (config::data_size % config::servers_number)));
    }

    std::vector<NetworkInteractor*> servers_ints;
    for (auto s : servers) {
        servers_ints.push_back(&s->interactor);
    }

    coordinator.setServers(servers_ints);

    std::vector<Client*> clients;

    test1(&coordinator.interactor);

    for (auto s : servers) {
        delete s;
    }
    return 0;
}