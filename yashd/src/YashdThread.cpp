//
// Created by Holden Nicoletti on 6/3/22.
//

#include <string>
#include <arpa/inet.h>
#include "../include/YashdThread.h"

using namespace std;

void YashdThread::operator()() {
    logger.log("New connection to: " + string(inet_ntoa(client.sin_addr)) + ", " + to_string(ntohs(client.sin_port)));
}

YashdThread::YashdThread(int fd, sockaddr_in &addr, Logger &logger)
    :sockfd(fd), client(addr), logger(logger)
{
}
