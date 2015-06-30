#pragma once

#include <string>

using namespace std;

#define WIDTH_LARGE        640
#define HEIGHT_LARGE       480
#define WIDTH_SMALL        160
#define HEIGHT_SMALL       120
#define WIDTH_SMALL_MINUS  159
#define HEIGHT_SMALL_MINUS 119

#define FOV_WIDTH          23.7
#define FOV_DEPTH          17.9

extern const string cmd_quote;

extern string serial_number;
extern string executable_path;
extern string data_path;
extern string data_path_current_module;
extern string settings_file_path;
extern string menu_file_path;
extern string ipc_path;
extern string pose_database_path;

extern bool first_run;