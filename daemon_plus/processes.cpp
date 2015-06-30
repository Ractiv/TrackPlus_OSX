#include "processes.h"

int process_running(const string name)
{
#ifdef _WIN32
	int process_count = 0;
	HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if(SnapShot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if(!Process32First(SnapShot, &procEntry))
		return false;

	do
	{
		if(strcmp(procEntry.szExeFile, name.c_str()) == 0)
			++process_count;
	}
	while (Process32Next(SnapShot, &procEntry));

	return process_count;
#elif __APPLE__
    
    return 0;
    
#endif
}

int process_running_osx(int pid)
{

    return -1;
}

int create_process_osx(const string path, const string name)
{
    printf("in create process -- path=%s  -- name = %s\n",path.c_str(),name.c_str());
    
   
    while (true) {
        pid_t pID = vfork();
        if (pID == 0)                // child
        {
            // Code only executed by child process
            execl(path.c_str(), path.c_str(),  (char *) 0);
            printf("Child going to exit\n");
            _exit(0);
            
        }
        else if (pID < 0)            // failed to fork
        {
            cerr << "Failed to fork" << endl;
            exit(1);
            // Throw exception
        }
        else                                   // parent
        {
            // Code only executed by parent process
            
            printf("Parent Process started\n");
            printf("parent waiting\n");
            int childExitStatus;
            printf("child running with process ID = %d\n",pID);
            
            wait(&pID);
            usleep(3000000);
            if( WIFEXITED(childExitStatus) )
            {
                // Child process exited thus exec failed.
                // LOG failure of exec in child process.
                cout << "Result of waitpid: Child process exited thus exec failed." << endl;
            }
            printf("done waiting, child exited\n");
        }
    }
    return  -1;
}

void create_process(const string path, const string name)
{
#ifdef _WIN32
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
	STARTUPINFO StartupInfo; //This is an [in] parameter
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo; //Only compulsory field

	CreateProcess(path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);
	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);

	while (process_running(name.c_str()) == 0)
		Sleep(1);
#elif __APPLE__

#endif
}

void kill_process(const string name)
{
#ifdef _WIN32
	CHAR szProcBuff[101];
	DWORD pIDs[300], dwBytesReturned;
	HANDLE hProcess;
	INT i, procCount;

	EnumProcesses(pIDs, sizeof(pIDs), &dwBytesReturned);
	procCount = dwBytesReturned / sizeof(DWORD);

	for (i = 0; i < procCount; i++)
		if (pIDs[i] != 0)
		{
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, 0, pIDs[i]);
			GetModuleBaseName(hProcess, 0, szProcBuff, 100);

			if (strcmp(szProcBuff, name.c_str()) == 0)
				TerminateProcess(hProcess, EXIT_SUCCESS);

			CloseHandle(hProcess);
		}
#elif __APPLE__
    
#endif
}