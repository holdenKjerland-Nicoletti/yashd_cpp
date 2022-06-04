//
// Created by Holden Nicoletti on 5/24/22.
//

#include "../include/Job.h"

#include <sstream>

using namespace std;

string statusmsg(Status status){
    switch(status){
        case RUNNING:
            return "Running";
        case STOPPED:
            return "Stopped";
        case DONE:
            return "Done";
    }
}

Job::Job(std::string cmd) : cmd(cmd)
{
}

/*
 * Create new process
 * Split string by spaces:
 *  If string = |
 *      Create new process
 *  If string = <
 *      New infile
*   If string = >
 *      New outfile
 *
 *
 * TODO: Handling multiple input or output files
 */
void Job::createProcs() {

    if(cmd[cmd.size()-1] == '&'){
        fg = false;
        cmd.erase(cmd.size()-1);
    }

    //https://stackoverflow.com/a/5607650
    istringstream ss(cmd);
    string arg;
    unique_ptr<Process> proc = make_unique<Process>();

    while(ss >> arg){
        if(arg=="|"){
            procs.push_back(move(proc));
            proc = make_unique<Process>();
        }else if(arg=="<") {
            ss >> arg;
            proc->addInFile(arg);
        }else if(arg==">"){
            ss >> arg;
            proc->addOutFile(arg);
        }else{
            proc->addArg(arg);
        }
    }
    procs.push_back(move(proc));
//    printProcs();
}

void Job::printProcs() {
    if(!fg)
        cout<<"Background:"<<endl;
    for(int i = 0; i < procs.size(); i++){
        cout<<"Process "<<i<<":"<<endl;
        cout<<*procs[i]<<endl;
    }
}

std::ostream &operator<<(ostream & out, const Job & j) {
    out<<" "<< statusmsg(j.status)<<'\t'; // print status
    out<<j.cmd;
//    out<<((!j.fg) ? " &\n" : "\n");
    out<<endl;
    return out;
}

bool Job::isfg() {
    return fg;
}

int Job::getpgid() {
    return pgid;
}

void Job::setpg(int p, int pg) {
    setpgid(p, pg);
    pgid = pg;
}

void Job::setStatus(Status s) {
    status = s;
}

void Job::setfg(bool b) {
    fg = b;
}

Status Job::getStatus() {
    return status;
}

std::string Job::getcmd() {
    return cmd;
}

void Job::printJob(bool plus, int jobNo){
    cout<<"["<<jobNo<<"] ";
    cout<<(plus ? '+' : '-')<<"  ";
    cout<<*this;
}
