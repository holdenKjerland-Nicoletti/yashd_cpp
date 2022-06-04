//
// Created by Holden Nicoletti on 6/3/22.
//

#ifndef YASHD_CPP_YASHDTHREAD_H
#define YASHD_CPP_YASHDTHREAD_H

#include "Logger.h"

#include <netinet/in.h>


class YashdThread {
private:
    Logger& logger;
    sockaddr_in client;
    int sockfd;

public:
    YashdThread(int, sockaddr_in&, Logger&);
    void operator() ();
};


#endif //YASHD_CPP_YASHDTHREAD_H
