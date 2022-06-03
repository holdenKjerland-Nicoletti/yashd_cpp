//
// Created by Holden Nicoletti on 5/24/22.
//

#ifndef YASH_YASH_H
#define YASH_YASH_H

#include "Job.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>


class Yash {
private:
    std::map<int, std::unique_ptr<Job>> bgJobs; // map of Job Number to Job
    std::unique_ptr<Job> currentJob;
    std::unordered_map<int, int> bgJobPidMap; // Map of pid to job number
    std::string cmd;
//    int maxJobNo {0};



    // gets c
    bool getcmd();
    // Parses command to check if equal to fg, bg, or bgJobs
    void execJob();
    void checkJobs();
    void parent();
    Job& getJob(int);
    [[noreturn]]void child(int, int);
    // send current job to background
    void jobToBG();
    int getNextJob(bool);
    void updateStatus(int, int);
    [[noreturn]]void jobsExec();
    bool fgExec();
    void bgExec();
    void removeBGJob(int);

    void jobExited(int);
    void jobTerminated(int);
    void jobStopped(int);
    void jobContinued(int);
public:
    Yash();
    void run();
};


#endif //YASH_YASH_H
