//
//  main.cpp
//  daemon_plus
//
//  Created by Corey Manders on 16/12/14.
//  Copyright (c) 2014 Ractiv Pte. Ltd. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <unistd.h>

#include <thread>
#include "filesystem.h"
#include "settings.h"
#include "globals.h"
#include "ipc.h"
#include "processes.h"


#define MAX_PATH 2048

using namespace std;

IPC* ipc_ptr_global = NULL;

void guardian_thread_main()
{
#ifdef _WIN32
    while (true)
    {
       if (process_running("track_plus.exe") == 0)
        create_process(executable_path + "\\track_plus.exe", "track_plus.exe");
        

        Sleep(1000);
    }
#elif __APPLE__
    //int pid = -1;
    string path=string("my_path"),name=string("my_name");
    printf("guardian thread running\n");
    create_process_osx(executable_path + "/track_plus","track_plus");
    
#endif
    
}

void onExit()
{
#ifdef _WIN32
    if (process_running("menu_plus.exe") == 1)
        kill_process("menu_plus.exe");
#elif __APPLE__
    system("killall track_plus");
#endif
}

int main()
{
    atexit(onExit);
    
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    string::size_type pos = string(buffer).find_last_of("\\/");
    executable_path = string(buffer).substr(0, pos);
    data_path = executable_path + "\\userdata";
    // data_path_current_module = data_path + "\\serial_number";
    settings_file_path = data_path + "\\settings.nrocinunerrad";
    menu_file_path = executable_path + "\\menu_plus\\menu_plus.exe";
    ipc_path = executable_path + "\\ipc";
#elif __APPLE__
    executable_path = getCurrentPath();
    settings_file_path = executable_path + "/settings.nrocinunerrad";
    menu_file_path = executable_path + "/menu_plus/menu_plus";
    ipc_path = executable_path + "/ipc";
    data_path = executable_path + "/userdata";
  //  printf("settings_file_path = %s\n",settings_file_path.c_str());
  //  printf("menufile path = %s\n",menu_file_path.c_str());
  //  printf("ipc_path = %s\n",ipc_path.c_str());
  //  printf("data path = %s\n",data_path.c_str());
#endif
    
    Settings settings;
    
    if (!file_exists(settings_file_path))
    {
        settings.launch_on_startup = "1";
        settings.power_saving_mode = "0";
        settings.check_for_updates = "1";
        settings.touch_control = "1";
        settings.table_mode = "0";
        settings.auto_detect_interaction_plane = "0";
        
        create_directory(data_path);
        
        ofstream settings_ofs(settings_file_path, ios::binary);
        settings_ofs.write((char*)&settings, sizeof(settings));
        
        cout << "settings file created" << endl;
    }
    
    else
    {
        ifstream ifs(settings_file_path, ios::binary);
        ifs.read((char*)&settings, sizeof(settings));
        
        cout << "settings file loaded" << endl;
    }
    
    IPC ipc("daemon_plus");
    ipc_ptr_global = &ipc;
    
    IPC* ipc_ptr = &ipc;
    Settings* settings_ptr = &settings;
    ipc.map_function("get toggles", [ipc_ptr, settings_ptr](const string message_body, const string id)
                     {
                         string response = "";
                         response += settings_ptr->launch_on_startup
                         +  settings_ptr->power_saving_mode
                         +  settings_ptr->check_for_updates
                         +  settings_ptr->touch_control
                         +  settings_ptr->table_mode
                         +  settings_ptr->auto_detect_interaction_plane;
                         
                         ipc_ptr->send_message("menu_plus", "get toggles", response + "\n" + id);
                     });
    
    ipc.map_function("set toggle", [ipc_ptr, settings_ptr](const string message_body, const string id)
                     {
                         const string toggle_name = message_body.substr(0, message_body.size() - 1);
                         const string toggle_value = message_body.substr(message_body.size() - 1, message_body.size());
                         
                         if (toggle_name == "toggleLaunchOnStartup")
                             settings_ptr->launch_on_startup = toggle_value;
                         
                         else if (toggle_name == "togglePowerSavingMode")
                             settings_ptr->power_saving_mode = toggle_value;
                         
                         else if (toggle_name == "toggleCheckForUpdates")
                             settings_ptr->check_for_updates = toggle_value;
                         
                         else if (toggle_name == "toggleTouchControl")
                             settings_ptr->touch_control = toggle_value;
                         
                         else if (toggle_name == "toggleTableMode")
                             settings_ptr->table_mode = toggle_value;
                         
                         else if (toggle_name == "toggleAutoDetectInteractionPlane")
                             settings_ptr->auto_detect_interaction_plane = toggle_value;
                         
                         ofstream settings_ofs(settings_file_path, ios::binary);
                         settings_ofs.write((char*)settings_ptr, sizeof(*settings_ptr));
                         
                         ipc_ptr->send_message("menu_plus", "set toggle", "\n" + id);
                     });
    
    thread guardian_thread(guardian_thread_main);
    
    while (true)
    {
        ipc.update();
#ifdef _WIN32
        Sleep(1);
#elif __APPLE__
        usleep(10);
#endif
    }
     
    
    return 0;
}
