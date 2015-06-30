#include "filesystem.h"
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <libproc.h>

bool directory_exists(const string path)
{
#ifdef _WIN32
    //define something for Windows (32-bit and 64-bit, this part is common)
    DWORD ftyp = GetFileAttributesA(path.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;
    
    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;
    
    return false;
#elif __APPLE__
    int         status;
    struct      stat buffer;
    status = stat(path.c_str(), &buffer);
    if (status < 0) return false;
    return true;
#endif
}

bool file_exists(const string path)
{
    #ifdef _WIN32
    ifstream f(path.c_str());
    if (f.good())
    {
        f.close();
        return true;
    }
    else
    {
        f.close();
        return false;
    }
    #elif __APPLE__
    int         status;
    struct      stat buffer;
    status = stat(path.c_str(), &buffer);
    if (status < 0) return false;
    return true;
    #endif
    
}

vector<string> list_files_in_directory(const string path)
{
    vector<string> file_name_vec;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
            if (ent->d_name[0] != 46){
                file_name_vec.push_back(ent->d_name);
            }

        closedir (dir);
    }
 
    return file_name_vec;
}

void create_directory(const string path)
{
#ifdef _WIN32
    CreateDirectory(path.c_str(), NULL);
#elif __APPLE__

    mkdir(path.c_str(), 0775);
#endif
}

void write_string_to_file(const string path, const string str)
{
    
    ofstream out(path);
    out << str.c_str();
    out.close(); 
    
}

vector<string> read_text_file(const string path)
{
    
    vector<string> lines;
    
    string line;
    ifstream in_file(path);
    if (in_file)
        while (getline(in_file, line))
            lines.push_back(line);

    in_file.close();
    
    return lines;
}

void copy_file(const string src_path, const string dst_path)
{
    
    ifstream src(src_path, ios::binary);
    ofstream dest(dst_path, ios::binary);
    dest << src.rdbuf();
    
}

void delete_file(const string path)
{
#ifdef _WIN32
    while (remove(path.c_str()) != 0)
        Sleep(1);

#elif __APPLE__
    while (remove(path.c_str()) != 0)
        sleep(1);
#endif
}

void delete_all_files(const string path)
{
    vector<string> file_name_vec = list_files_in_directory(path);
    for (string file_name : file_name_vec)
        delete_file(path + "\\" + file_name);
}
string getCurrentPath(){
    int ret;
    pid_t pid;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    
    pid = getpid();
    ret = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
    if ( ret <= 0 ) {
        fprintf(stderr, "PID %d: proc_pidpath ();\n", pid);
        fprintf(stderr, "    %s\n", strerror(errno));
        return string("PATH ERROR:");
    }
    string myFullPath = string(pathbuf);
    unsigned long found =myFullPath.find_last_of("/");
    return myFullPath.substr(0,found);
}

