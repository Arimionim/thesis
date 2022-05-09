#ifndef THESIS_NETWORKINTERACTOR_H
#define THESIS_NETWORKINTERACTOR_H

#include "../messages/Transaction.h"
#include "Receiver.h"

class NetworkInteractor {
public:
    explicit NetworkInteractor(Receiver *receiver) : receiver(receiver) { }

    void receive(NetworkInteractor *sender, const Transaction& transaction) {
        receiver->receive(sender, transaction);
    }

    void send(NetworkInteractor *interactor, const Transaction &tx) {
        send_helper(this, interactor, tx);
    }
private:
    static void send_helper(NetworkInteractor *sender, NetworkInteractor *receiver, const Transaction& tx) {
        std::thread worker = std::thread(send_helper, sender, receiver, tx);
        worker.detach();
    }

    Receiver *receiver;
};

#endif //THESIS_NETWORKINTERACTOR_H
