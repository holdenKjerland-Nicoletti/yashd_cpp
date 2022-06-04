//
// Created by Holden Nicoletti on 5/24/22.
//

#ifndef YASH_JOB_H
#define YASH_JOB_H

#include <string>
#include <vector>
#include "Process.h"

enum Status {
    RUNNING,
    STOPPED,
    DONE
};

std::string statusmsg(Status status);

class Job {
private:
    std::string cmd;
    int pgid {0};
    bool fg {true};
    Status status {RUNNING};

public:
    std::vector<std::unique_ptr<Process>> procs;

    Job(std::string);
    void createProcs();
    [[maybe_unused]]void printProcs();
    bool isfg();
    int getpgid();
    Status getStatus();
    void setpg(int, int);
    void setStatus(Status);
    void setfg(bool);
    std::string getcmd();
    void printJob(bool, int);

    friend std::ostream& operator << (std::ostream&, const Job&);
};


#endif //YASH_JOB_H
