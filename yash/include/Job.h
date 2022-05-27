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
    int jobNo {0};
    bool next {true}; // If current job or next job for fg

public:
    std::vector<std::unique_ptr<Process>> procs;

    Job(std::string);
    void createProcs();
    void printProcs();
    bool isfg();
    int getpgid();
    void setpg(int, int);
    void setJobNo(int);
    void setNext(bool);

    friend std::ostream& operator << (std::ostream&, const Job&);
};


#endif //YASH_JOB_H
