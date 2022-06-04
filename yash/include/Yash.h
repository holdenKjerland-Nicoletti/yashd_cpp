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


    bool getcmd();
    int getNextJob(bool);

    void checkJobs();
    void jobToBG();
    void updateStatus(int, int);
    void removeBGJob(int);

    void parent();
    [[noreturn]]void child(int, int);

    //The most important function which creates a new job from cmd and executes accordingly
    void execJob();

    /*For executing built in commands:
     * jobs
     * fg
     * bg
     */
    [[noreturn]]void jobsExec();
    bool fgExec();
    void bgExec();

    void jobExited(int);
    void jobTerminated(int);
    void jobStopped(int);
    void jobContinued(int);
public:
    Yash();
    void run();
};


#endif //YASH_YASH_H
