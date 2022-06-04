//
// Created by Holden Nicoletti on 6/3/22.
//

#include "../include/Logger.h"

//#include <syncstream>

using namespace std;

Logger::Logger(const std::string) {
    logFile.open("/tmp/yashd.logger", ofstream::trunc);
    if(!logFile.is_open()){
        perror("Opening Log File");
        exit(EXIT_FAILURE);
    }
}

void Logger::log(const string msg){
    logFile << msg << endl;
    logFile.flush();
}

void Logger::lerror(const string msg) {
    logFile << msg << strerror(errno) << endl;
    logFile.flush();
}