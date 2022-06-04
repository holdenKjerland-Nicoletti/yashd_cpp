//
// Created by Holden Nicoletti on 5/24/22.
//

#include "../include/Yash.h"
#include "../include/utils.h"

#include <csignal>
#include <iostream>
#include <unistd.h>

using namespace std;

Yash::Yash() {
    // We want to ignore all signals within the shell process
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
}

/*
 * Strategy:
    * Ensure shell has TC
    * Get next command
    * Check all children for SIGCHLD (check_jobs)
    * Remove \n and white space at end of command
    * Parse cmd:
        * fg:
            * fg_job
            * Set job to foreground
        *  bg:
            * Just send kill to last stopped job
        *  else:
            * execute command
 */
void Yash::run(){
    while(1){
        // Ensure shell control of the terminal
        getTC();

        if(!getcmd()){
            checkJobs();
            continue;
        }

        checkJobs();
        execJob();

        cout<<flush;
    }
}

bool Yash::getcmd(){
    cout << "# ";
    if (!getline(cin, cmd)) {
        // If CTRL-D is encountered we enter this if statement: https://stackoverflow.com/a/19228847
        printf("CTRL-D received, quitting shell\n");
        exit(0);
    }

    trim(cmd);

    return (cmd.size() > 0);
}


/* Create a new Job by parsing cmd
 *
 * If cmd = fg or bg, execute functions
 *
 * Loop through all processes and fork each child process
 *      Create pipe if needed
 *      Call child() for each process
 * Call parent()
 */
void Yash::execJob() {

    currentJob = unique_ptr<Job> (new Job(cmd));
    currentJob->createProcs();

    switch(currentJob->procs[0]->getExecCmd()){
        case FG:
            if(!fgExec()){ // if no job to fg, return
                return;
            }
            break; // currentJob has been updated
        case BG:
            bgExec();
            return;
        default:
            break;
    }

    int cpid, cpgid; // child pid and process group for all children
    int numProcs = currentJob->procs.size();

    int pipefd[2];

    for(int i = 0; i < numProcs; i++){

        // Create pipe (if not last process)
        if(i != numProcs-1){
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(-1);
            }
            currentJob->procs[i]->setOutfd(pipefd[1]);
            currentJob->procs[i+1]->setInfd(pipefd[0]);
        }

        // --------- Parent -------------
        if((cpid = fork()) > 0){
            // if first child, set child process group id
            if(i == 0) {
                cpgid = cpid;
            }

            // Set pgid of children and TC in both parent and child to avoid race condition
            currentJob->setpg(cpid, cpgid);
            if(currentJob->isfg()  && (tcsetpgrp(STDIN_FILENO, cpgid) != 0)){
                perror("tcsetpgrp() error in parent fg");
            }

            // close input pipe
            if(i != numProcs-1) {
                close(pipefd[1]);
            }
        //  --------- Child -------------
        }else if(cpid == 0) {

            // Set pgid of children and TC in both parent and child to avoid race condition
            child(i, cpgid);

        // Fork Failure
        }else{
            perror("Fork");
            exit(1);
        }
    }

    //close output pipe
    if(numProcs > 1){
        close(pipefd[0]);
    }

    // Once all processes have been forked, call parent which calls waitpid for foreground
    parent();
}

void Yash::child(int procNum, int pgid){

    // Reset signals to the defaults
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    // if first child, set child process group id
    if (procNum == 0) {
        pgid = getpid();
    }

    // Set pgid of children and TC in both parent and child to avoid race condition
    currentJob->setpg(0, pgid);
    if (currentJob->isfg() && tcsetpgrp(STDIN_FILENO, pgid) != 0){
        perror("tcsetpgrp() error in child fg");
    }

    currentJob->procs[procNum]->redirectStreams();

    // FG and BG already handled
    switch(currentJob->procs[procNum]->getExecCmd()){
        case JOBS:
            jobsExec();
        default:
            currentJob->procs[procNum]->exec();
    }
}


void Yash::parent() {

    // if it is a background job, add to the job table
    if(!currentJob->isfg() ){
        jobToBG();
        return;
    }

    int status, pid;
    int cpgid = currentJob->getpgid();

    // Wait for all processes of child process group to finish
    while((pid = waitpid(-cpgid, &status, WUNTRACED)) > 0){

        // Job recieved a signal, all processes must have recieved it
        if(WIFSIGNALED(status) || WIFSTOPPED(status)){
            updateStatus(status, pid);
            return;
        }
    }
}

void Yash::jobsExec(){

    // loop through all background jobs
    for(auto job = bgJobs.begin(); job != bgJobs.end(); job++){
        // For the last element, print +
        if(next(job) == bgJobs.end()){
            job->second->printJob(true, job->first);
        }else{
            job->second->printJob(false, job->first);
        }
    }

    _exit(127);
}

/*
 * Executes fg command by setting the new currentJob as the last jobToBG
 * that is stopped or running, and sending a SIGCONT
 */
bool Yash::fgExec(){

    // get next background job that is running or stopped
    int next = getNextJob(true);

    // if there is a background job, make it new current job
    if(next > 0){
        currentJob = move(bgJobs[next]);
        removeBGJob(next);

        killpg(currentJob->getpgid(), SIGCONT);
        currentJob->setfg(true);

        cout<<currentJob->getcmd()<<endl;
        return true;
    }

    cout<<"-bash: fg: current: no such job\n";
    return false;
}

// Send sigcont to next stopped job in the background
void Yash::bgExec(){

    // get next stopped jobs in the background
    int next = getNextJob(false);

    if(next > 0){
        killpg(bgJobs[next]->getpgid(), SIGCONT);
        bgJobs[next]->setStatus(RUNNING);

        // print job with + or -
        if(next == bgJobs.rbegin()->first){
            bgJobs[next]->printJob(true, next);
        }else{
            bgJobs[next]->printJob(false, next);
        }
        return;
    }

    cout<<"-bash: bg: current: no such job\n";
    return;
}

/*
 * Gets the next bg job for fg and bg
 *
 * bg = false
 * fg = true
 *
 * if fg is true we return next job that is stopped or running
 * if false return only if job is stopped
 */
int Yash::getNextJob(bool fg){
    for(auto job = bgJobs.rbegin(); job != bgJobs.rend(); job++){
        if(job->second->getStatus() == STOPPED || (fg && job->second->getStatus() == RUNNING)){
            return job->first;
        }
    }
    return 0;
}

// add job to background
void Yash::jobToBG() {
    currentJob->setfg(false);

    // if bgJobs isn't empty, jobNo = max + 1
    int jobNo = (bgJobs.empty()) ? 1 : bgJobs.rbegin()->first + 1;

    // update bgJobPidMap and bgJobs
    bgJobPidMap[currentJob->getpgid()] = jobNo;
    bgJobs[jobNo] = move(currentJob);
}

// update status of job
void Yash::updateStatus(int status, int pid) {
    if (WIFEXITED(status)) {
        jobExited(pid);
    }else if(WIFSIGNALED(status)){
        jobTerminated(pid);
    }else if (WIFSTOPPED(status)) {
        jobStopped(pid);
    } else if (WIFCONTINUED(status)) {
        jobContinued(pid);
    }
}

void Yash::jobExited(int pid) {
    if(bgJobPidMap.find(pid) == end(bgJobPidMap)){
        return;
    }

    int jobNo = bgJobPidMap[pid];
    if(bgJobs.find(jobNo) != end(bgJobs)){
        bgJobs[jobNo]->setStatus(DONE);
        bgJobs[jobNo]->printJob(true, jobNo);

        removeBGJob(jobNo);
    }
}

void Yash::jobTerminated(int pid) {
    if(bgJobPidMap.find(pid) == end(bgJobPidMap)){
        return;
    }

    int jobNo = bgJobPidMap[pid];
    if(bgJobs.find(jobNo) != end(bgJobs)){
        bgJobs[jobNo]->setStatus(DONE);
    }
}

void Yash::jobStopped(int pid) {
    // check if job is in the background already
    if(bgJobPidMap.find(pid) == end(bgJobPidMap)){
        currentJob->setStatus(STOPPED);
        jobToBG();
        return;
    }

    // update status of bgJob
    int jobNo = bgJobPidMap[pid];
    if(bgJobs.find(jobNo) != end(bgJobs)){
        bgJobs[jobNo]->setStatus(STOPPED);
    }
}

void Yash::jobContinued(int pid) {
    if(bgJobPidMap.find(pid) == end(bgJobPidMap)){
        return;
    }

    int jobNo = bgJobPidMap[pid];
    if(bgJobs.find(jobNo) != end(bgJobs)){
        bgJobs[jobNo]->setStatus(RUNNING);
    }
}

/* Check for change in status for any job without hanging
 * If change in status, update job and job table
*/
void Yash::checkJobs() {
    int status, pid;

    // check for any updated processes
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){

        // update status in background job table
        updateStatus(status, pid);
    }
}

void Yash::removeBGJob(int jobNo){
    if(bgJobs.find(jobNo) != end(bgJobs)){

        if(bgJobs[jobNo] && bgJobPidMap.find(bgJobs[jobNo]->getpgid()) != end(bgJobPidMap)){
            bgJobPidMap.erase(bgJobs[jobNo]->getpgid());
        }

        bgJobs.erase(jobNo);
    }
}

