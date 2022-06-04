//
// Created by Holden Nicoletti on 6/3/22.
//

#ifndef YASHD_CPP_LOGGER_H
#define YASHD_CPP_LOGGER_H

#include <fstream>

class Logger {
private:
    std::ofstream logFile;
public:
    Logger() {};
    Logger(std::string const);
    void log(const std::string);
    void lerror(const std::string);
};


#endif //YASHD_CPP_LOGGER_H
