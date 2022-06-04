//
// Created by Holden Nicoletti on 5/25/22.
//

#include "../include/Process.h"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

Process::Process(){
}

Process::~Process() {
    for(int i = 0; i < argv.size(); i++){
        free((char *) argv[i]);
    }
}

void Process::addArg(string& args) {
    char* arg = (char *)malloc(args.size());
    strcpy(arg, args.c_str());
    argv.push_back(arg);
}

void Process::addInFile(string& fileName) {

    if(fileName.size() == 0 || fileName[0] == '<'){
        cout<<"syntax error near unexpected token `newline'"<<endl;
        return;
    }
    if(fileName[0] == '|' || fileName[0] == '>'){
        cout<<"syntax error near unexpected token `"<<fileName[0]<<"'"<<endl;
        return;
    }

    inFileName = fileName;
    inFile = true;
}

void Process::addOutFile(string& fileName) {

    if(fileName.size() == 0 || fileName[0] == '>'){
        cout<<"syntax error near unexpected token `newline'"<<endl;
        return;
    }
    if(fileName[0] == '|' || fileName[0] == '<'){
        cout<<"syntax error near unexpected token `"<<fileName[0]<<"'"<<endl;
        return;
    }

    outFileName = fileName;
    outFile = true;
}


// --------------------------------------------------------CHILD--------------------------------------------------------
/*Strategy:
    * Reset signals to default
    * Call exec_process
*/
void Process::exec(){
    // https://stackoverflow.com/a/47716623
    argv.push_back(NULL);
    char** argvchar = argv.data();

    execvp(argvchar[0], &argvchar[0]);
    exit(1);
}

std::ostream &operator<<(ostream & out, const Process & p) {
    for(auto arg: p.argv){
        out<<arg<<endl;
    }
    if(p.inFile) out << "Input:" << p.inFileName << endl;
    if(p.outFile) out << "Output:" << p.outFileName << endl;
    return out;
}

void Process::redirectStreams() {

    // If we were given '<' and/or '>', open files
    if(inFile) {
        if ((infd = open(inFileName.c_str(), O_RDONLY)) == -1) {
            perror("Opening Input File");
            exit(1);
        }
    }
    if(outFile){
        if((outfd = open(outFileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG)) == -1) {
            perror("Opening Output File");
            exit(1);
        }
    }

    // Can be a File or Pipe
    if(infd != -1){
        dup2(infd, STDIN_FILENO);
        close(infd);
    }
    if(outfd != -1){
        dup2(outfd, STDOUT_FILENO);
        close(outfd);
    }
}

void Process::setOutfd(int fd) {
    outfd = fd;
}

void Process::setInfd(int fd) {
    infd = fd;
}

Command Process::getExecCmd() {
    if(!strcmp(argv[0], "jobs")){
        return JOBS;
    }else if(!strcmp(argv[0], "fg")){
        return FG;
    }else if(!strcmp(argv[0], "bg")){
        return BG;
    }
    return EXEC;
}
