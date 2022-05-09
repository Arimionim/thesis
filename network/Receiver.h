#ifndef THESIS_RECEIVER_H
#define THESIS_RECEIVER_H

#include "../messages/Transaction.h"
#include "NetworkInteractor.h"

class NetworkInteractor;

class Receiver {
public:
    virtual ~Receiver() = default;
    virtual void receive(NetworkInteractor *sender, Transaction transaction) = 0;
};

#endif //THESIS_RECEIVER_H
