#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "Utils.h"

#include "server/Server.h"
#include "coordinator/Coordinator.h"
#include "client/Client.h"


void test1(size_t servers_number = config::servers_number, size_t clients_number = config::clients_number,
           double write_ratio = config::write_ratio) {
    std::cout << "starting test " << servers_number << " " << clients_number << ' ' << write_ratio << std::endl << std::endl;

    config::servers_number = servers_number;
    config::clients_number = clients_number;
    config::write_ratio = write_ratio;

    Coordinator coordinator;

    std::vector<Server *> servers;

    for (int i = 0; i < config::servers_number; i++) {
        servers.push_back(new Server(&coordinator.interactor, config::data_size));
    }

    std::vector<NetworkInteractor *> servers_ints;
    for (auto s: servers) {
        servers_ints.push_back(&s->interactor);
    }

    std::cout << "servers created" << std::endl;

    coordinator.setServers(servers_ints);

    auto c_interactor = &coordinator.interactor;

    std::vector<Client *> clients;

    for (int i = 0; i < config::clients_number; i++) {
        clients.push_back(new Client(c_interactor, i));
    }

    std::cout << "clients created\nstarting load.." << std::endl;

    std::vector<std::thread> ts;
    for (auto &c: clients) {
        c->addLoad(100, config::write_ratio);
        ts.emplace_back(&Client::startLoad, c, 100, 0);
    }

    std::cout << "load completed\nwaiting for results.." << std::endl;

    for (auto &t: ts) {
        t.join();
    }

    for (auto c: clients) {
        delete c;
    }

    std::stringstream res_file;
    res_file << config::servers_number << '_' << config::clients_number << '_' << config::write_ratio;
    std::ofstream res(res_file.str());
    res << logger::avg_r() << ' ' << logger::avg_w() << std::endl;

    for (auto s: servers) {
        delete s;
    }
    std::cout << "finished\n" << std::endl;
}


int main() {
   // test1(2, 10);
    for (double wr = 0.1; wr <= 10; wr += 0.1) {
        for (int s = 1; s <= 5; s++) {
            for (int c = 1; c <= 20; c++) {
                try {
                    test1(s, c, wr);
                } catch (const std::exception& e) {
                    std::cout << e.what();
                    c--;
                }
            }
        }
    }

    return 0;
}