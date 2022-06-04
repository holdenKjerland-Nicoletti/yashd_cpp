//
// Created by Holden Nicoletti on 6/3/22.
//

#ifndef YASHD_CPP_YASHDSERVER_H
#define YASHD_CPP_YASHDSERVER_H

#include "Logger.h"
#include "YashdThread.h"

#include <fstream>
#include <string>
#include <netinet/in.h>
#include <memory>

#define MAXCONNS 20

class YashdServer {
private:
    Logger logger;
    int const PORT;
    char IP[INET_ADDRSTRLEN];
    int sockfd;
    struct sockaddr_in server; // used for AF_INET domain

    // initialization
    void createDaemon();
    void createTCPServer();
    void acceptClient();

    void printClient(sockaddr_in&);
public:
    YashdServer();
    YashdServer(int const);
    ~YashdServer();
    void run();
};


#endif //YASHD_CPP_YASHDSERVER_H
