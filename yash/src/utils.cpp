//
// Created by Holden Nicoletti on 5/24/22.
//

#include <unistd.h>
#include <iostream>

void getTC(){
    // Ensure shell has control of terminal
    if (tcsetpgrp(STDIN_FILENO, getpgid(0)) != 0){
        perror("tcsetpgrp() error");
    }
}


void trim(std::string& line){
    // https://stackoverflow.com/a/216883

    // Trim left side
    line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));

    // Trim right side
    line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), line.end());
}