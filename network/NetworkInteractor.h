#ifndef THESIS_NETWORKINTERACTOR_H
#define THESIS_NETWORKINTERACTOR_H

#include "../messages/Transaction.h"
#include "Receiver.h"
#include <winsock2.h>


class NetworkInteractor {
public:
    explicit NetworkInteractor(Receiver *receiver) : owner(receiver) { }

    void
    receive(NetworkInteractor *sender, const Transaction& transaction) {
        owner->receive(sender, transaction);
    }

    void send(NetworkInteractor *receiver, Transaction tx) {
//        std::cout << "sending " << tx.id << ' ' << owner << "->" << receiver << std::endl;
        std::thread worker = std::thread(send_helper, this, receiver, tx);
        worker.detach();
    }

private:
    static void send_helper(NetworkInteractor *sender, NetworkInteractor *receiver, Transaction tx) {
       // std::cout << "transaction " << sender << "->" << receiver << std::endl;
        receiver->receive(sender, tx);
    }

    Receiver *owner;
};

#endif //THESIS_NETWORKINTERACTOR_H
