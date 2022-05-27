//
// Created by Holden Nicoletti on 5/24/22.
//

#ifndef YASH_YASH_H
#define YASH_YASH_H

#include "Job.h"

#include <string>
#include <memory>
#include <vector>


class Yash {
private:
    std::vector<std::unique_ptr<Job>> bgJobs;
    std::unique_ptr<Job> currentJob;
    std::string cmd;
    int maxJobNo {0};



    // gets c
    bool getcmd();
    // Parses command to check if equal to fg, bg, or bgJobs
    void execJob();
    void parent();
    [[noreturn]]void child(int, int);
    // send current job to background
    void bgJob();
    [[noreturn]]void jobsExec();
    [[noreturn]]void fgExec();
    [[noreturn]]void bgExec();
public:
    Yash();
    void run();
};


#endif //YASH_YASH_H
