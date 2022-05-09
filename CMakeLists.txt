cmake_minimum_required(VERSION 3.22)
project(thesis)

set(CMAKE_CXX_STANDARD 14)

add_executable(thesis
        main.cpp
        client/Client.h
        coordinator/Coordinator.h
        client/RequestGenerator.h
        client/ClientNetworkInteractor.h
        messages/Transaction.h messages/TransactionType.h network/NetworkEmulator.h messages/MessageType.h environment.h Utils.h coordinator/CoordinatorNetworkInteractor.h server/ServerNetworkInteractor.h server/Server.h)
