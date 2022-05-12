#include <iostream>
#include <vector>
#include "Utils.h"

#include "server/Server.h"
#include "coordinator/Coordinator.h"
#include "client/Client.h"


void test1(NetworkInteractor *coordinator) {
    std::vector<Client*> clients;

    for (int i = 0; i < config::clients_number; i++) {
        clients.push_back(new Client(coordinator));
    }


    std::vector<std::thread> ts;
    for (auto& c : clients) {
        c->addLoad(100, 0);
        ts.emplace_back(&Client::startLoad, c, 500, 0);
    }

    for (auto& t : ts) {
        t.join();
    }
}

int main() {
    Coordinator coordinator;

    std::vector<Server*> servers;

    for (int i = 0; i < config::servers_number; i++) {
        servers.push_back(new Server(&coordinator.interactor, config::data_size));
    }

    std::vector<NetworkInteractor*> servers_ints;
    for (auto s : servers) {
        servers_ints.push_back(&s->interactor);
    }

    std::cout << "servers created" << std::endl;

    coordinator.setServers(servers_ints);

    test1(&coordinator.interactor);
    return 0;
}