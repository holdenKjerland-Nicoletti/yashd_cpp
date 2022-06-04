//
// Created by Holden Nicoletti on 6/3/22.
//

#include "../include/YashdServer.h"

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>

using namespace std;


YashdServer::YashdServer(): PORT(3826) {
}

YashdServer::YashdServer(int const port): PORT(port) {
}

void YashdServer::run() {

    // Daemonize process
    createDaemon();

    // Create TCP Server Socket
    createTCPServer();

    while(1){

        acceptClient();

    }
}

void YashdServer::acceptClient() {
    sockaddr_in client;
    socklen_t clientlen = sizeof(client);
    int clientSockfd;

    if((clientSockfd = accept(sockfd, (struct sockaddr*)&client, &clientlen)) < 0){
        logger.lerror("Error accept: ");
    }
//    thread t1 {YashdThread{clientSockfd, ref(client), ref(logger)} };
    printClient(client);
}

/* Create YashdServer:
 * http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
 *
 * Steps:
 * 1. Fork off parent process
 * 2. Change file mode mask to umask
 * 3. Open Log File
 * 4. Create unique session id
 * 5. Change current working directory
 * 6. Close std FDs
 * 7. Enter YashdServer Code
 */
void YashdServer::createDaemon() {
    pid_t pid, sid;

    // 1. Fork Parent Process
    if((pid = fork()) < 0){
        perror("Daemon Fork");
        exit(EXIT_FAILURE);
    // Exit for parent process
    }else if(pid > 0){
        exit(EXIT_SUCCESS);
    }

    cout<<"Daemon pid: "<<getpid()<<endl;


    // 2. Change file mode mask: this allows program to write to any files
    umask(0);


    // 3. Open logFile at "/tmp/yashd.logger",  this is necessary since we close stdout
    logger = Logger("/tmp/yashd.logger");
    logger.log("logFile successfully opened");


    // 4. Create a unique session ID
    if((sid = setsid()) < 0){
        logger.lerror("Error Setting Session ID");
        exit(EXIT_FAILURE);
    }
    logger.log("New Session ID: " + to_string(sid));


    // 5. Changing the working directory ("/tmp/yashd_tmp/")
    if(chdir("/tmp/yashd_tmp/") < 0){
        logger.lerror("Error Changing to /tmp/yashd_tmp/ directory");
        exit(EXIT_FAILURE);
    }
    logger.log("Successfully changed to /tmp/yashd_tmp/ directory");


    // 6. Close Standard File Descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

/* Create TCP Server:
 * http://mij.oltrelinux.com/devel/unixprg/#ipc__sockets
 * https://lenngro.github.io/how-to/2021/01/05/Simple-TCPIP-Server-Cpp/
 *
 * Steps:
 * 1. Create socket: socket()
 *      Optional: setsockopt() helps with reuse of address and port
 * 2. Bind attributes: bind()
 * 3. Open into network port: listen()
 *
 * Later:
 * 4. Read/Write: read()/write()
 * 5. Close: close()
 */
void YashdServer::createTCPServer() {

    int opt = 1;
    socklen_t addrlen;

    // 1. Create Socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        logger.lerror("Error Creating Server");
        exit(EXIT_FAILURE);
    }
    logger.log("Socket Created");


    /* Optional: allow local address reuse
     * allow the server to re-start quickly instead of waiting
     * for TIME_WAIT which can be as large as 2 minutes
     * https://www.geeksforgeeks.org/socket-programming-cc/
    */
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        logger.lerror("Error setsockopt:");
        exit(EXIT_FAILURE);
    }
    logger.log("Local address reuse enabled");


    // 2. Bind Attributes: https://lenngro.github.io/how-to/2021/01/05/Simple-TCPIP-Server-Cpp/
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT); // htons translates an unsigned integer into a network byte order
    server.sin_addr.s_addr =htonl(INADDR_ANY); // bound to local address
    if(::bind(sockfd, (struct sockaddr*) &server, sizeof(server)) < 0){
        logger.lerror("Error bind");
        exit(EXIT_FAILURE);
    }
    logger.log("Successfully binded socket");


    // Gets the name of the socket
    addrlen = sizeof(server);
    if(getsockname(sockfd, (struct sockaddr*) &server, &addrlen) < 0){
        logger.lerror("Error getsockname: ");
        exit(EXIT_FAILURE);
    }
    // https://gist.github.com/listnukira/4045436
    inet_ntop(AF_INET, &server.sin_addr, IP, sizeof(IP));
    logger.log("Local IP Address: " + string(IP));
    logger.log("Local Port: " + to_string(PORT));


    // 3. Listen to port
    if(listen(sockfd, MAXCONNS) < 0){
        logger.lerror("Error listen");
        exit(EXIT_FAILURE);
    }
    logger.log("Server Successfully created and ready to listen!");
}

YashdServer::~YashdServer() {
    logger.log("Closing Yashd Server");
    close(sockfd);
}

void YashdServer::printClient(sockaddr_in& client) {
    logger.log("New connection to: " + string(inet_ntoa(client.sin_addr)) + ", " + to_string(ntohs(client.sin_port)));
}
