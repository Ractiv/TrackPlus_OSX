#pragma once

#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")
#elif __APPLE__
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#endif



using namespace std;

int process_running(const string name);
void create_process(const string path, const string name);
void kill_process(const string name);

//// OSX specific functions

int create_process_osx(string path, string name );
int process_running_osx(int pid);
int kill_process_osx(int pid);