//
// Created by Holden Nicoletti on 5/25/22.
//

#ifndef YASH_PROCESS_H
#define YASH_PROCESS_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>

enum Command {
    EXEC,
    JOBS,
    FG,
    BG
};

class Process {
private:
    std::string inFileName {""};
    bool inFile {false};
    int infd {-1};
    std::string outFileName {""};
    bool outFile {false};
    int outfd {-1};
    std::vector<char*> argv;
    bool fail = false; // only set if failure setting up arguments and file redirection
    int pid;
    int pgid;
public:
    Process();
    void addArg(std::string&);
    void addInFile(std::string&);
    void addOutFile(std::string&);
    [[noreturn]] void exec();
    void redirectStreams();
    void setOutfd(int);
    void setInfd(int);
    Command getExecCmd();
    ~Process();

    friend std::ostream& operator << (std::ostream&, const Process&);
};


#endif //YASH_PROCESS_H
