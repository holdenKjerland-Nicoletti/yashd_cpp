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

        if(!getcmd())
            continue;

//        cout<<cmd<<endl;

        //----------------------------TODO----------------------------
        // Check for SIGCHLDS
//        check_jobs();

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

void Yash::execJob() {
    currentJob = unique_ptr<Job> (new Job(cmd));
    currentJob->createProcs();

    int cpid, cpgid; // child pid and process group for all children
    int numProcs = currentJob->procs.size();

    int pipefd[2];

    for(int i = 0; i < numProcs; i++){

        // if this is not the last process, we need to create a pipe
        if(i != numProcs-1){
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(-1);
            }
            // TODO: Add stream redirection: https://www.geeksforgeeks.org/io-redirection-c/
            currentJob->procs[i]->setOutfd(pipefd[1]);
            currentJob->procs[i+1]->setInfd(pipefd[0]);
        }

        // Parent
        if((cpid = fork()) > 0){
            close(pipefd[1]);

            if(i == 0) {// if first child, set child process group id
                cpgid = cpid;
            }

            // Set pgid of children and TC in both parent and child to avoid race condition
            currentJob->setpg(cpid, cpgid);
            if(currentJob->isfg()  && (tcsetpgrp(STDIN_FILENO, cpgid) != 0)){
                perror("tcsetpgrp() error in parent fg");
            }

        // Child
        }else if(cpid == 0) {
            if (i == 0) { // if first child, set child process group id
                cpgid = getpid();
            }
            child(cpgid, i);

        // Fork Failure
        }else{
            perror("Fork");
            exit(1);
        }
    }
    close(pipefd[0]);
    parent();
}

void Yash::child(int pgid, int procNum){
    currentJob->setpg(0, pgid);

    if (currentJob->isfg() && tcsetpgrp(STDIN_FILENO, pgid) != 0){
        perror("tcsetpgrp() error in child fg");
    }

    // Reset signals to the defaults
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    cout<<"Child:"<<getpid()<<endl;
    cout<<"PG:"<<getpgrp()<<endl;

    // TODO: handle bgJobs

    cout<< "before exec:"<<endl;
    cout<<*currentJob->procs[procNum]<<endl;
    cout<< "Executing:"<<endl;

    currentJob->procs[procNum]->redirectStreams();

    switch(currentJob->procs[procNum]->getExecCmd()){
        case EXEC:
            currentJob->procs[procNum]->exec();
        case JOBS:
            jobsExec();
        case FG:
            fgExec();
        case BG:
            bgExec();
    }

    currentJob->procs[procNum]->exec();
}


void Yash::parent() {
    int status;
    int pid = getpid();

    cout<<"Parent:"<<pid<<", waiting for:"<<currentJob->getpgid()<<endl;

    // TODO: add job

    // if it is a background job, add to the job table
    if(!currentJob->isfg() ){
        bgJob();
    }else{
        pid = waitpid(-currentJob->getpgid(), &status, WUNTRACED);
//        TODO: update_status(cpgid, status);

        // Need to make sure job wasn't sent to background
        while(currentJob->isfg()  &&(pid = waitpid(-currentJob->getpgid(), &status, WUNTRACED)) > 0){
//            update_status(getpgid(cpgid), status);
        }
    }
}

void Yash::jobsExec(){
    cout<<"jobs exec"<<endl;

    for(auto& job: bgJobs){
        cout<<*job;
    }

    exit(0);
}

void Yash::fgExec(){
    cout<<"fg exec"<<endl;

    exit(0);
}

void Yash::bgExec(){
    cout<<"bg exec"<<endl;

    exit(0);
}

// add job to background
void Yash::bgJob() {
    cout<<"Job to background"<< endl;

    if((bgJobs.size())) {
        bgJobs[bgJobs.size() - 1]->setNext(false);
    }

    currentJob->setJobNo(maxJobNo+1);
    bgJobs.push_back(move(currentJob));
    maxJobNo++;
}