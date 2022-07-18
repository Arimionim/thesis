cmake_minimum_required(VERSION 3.5)
project(thesis)

set(CMAKE_CXX_STANDARD 20)

add_executable(thesis
        main.cpp
        client/Client.h
        coordinator/Coordinator.h
        client/RequestGenerator.h
        messages/Transaction.h messages/TransactionType.h environment.h Utils.h server/Server.h network/NetworkInteractor.h network/Receiver.h)
