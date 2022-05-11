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
        c->addLoad(10, 0);
        std::cout << "add" << std::endl;
        std::thread t(&Client::startLoad, c, 1000, 0);
        std::cout << "start" << std::endl;
    }

    for (auto& t : ts) {
        t.join();
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

    std::cout << "servers created" << std::endl;

    coordinator.setServers(servers_ints);

    test1(&coordinator.interactor);
    return 0;
}