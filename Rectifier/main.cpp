#include <opencv2/opencv.hpp>
#include "globals.h"
#include "Camera.h"
#include "preprocessor.h"
#include "camera_initializer_new.h"
#include "motion_processor_new.h"
#include "foreground_extractor_new.h"
#include "ipc.h"
#include "hand_splitter_new.h"
#include "mono_processor_new.h"
#include "reprojector.h"
#define MAX_PATH 2048

#define VERBOSE 1
#define MJPEG 1
#define UNCOMPRESSED 0

using namespace cv;
using namespace std;

Mat inputImage;
unsigned char *buffer;

IPC* ipc = NULL;
Camera* camera = NULL;

MotionProcessorNew motion_processor0;
MotionProcessorNew motion_processor1;

ForegroundExtractorNew foreground_extractor0;
ForegroundExtractorNew foreground_extractor1;

HandSplitterNew hand_splitter0;
HandSplitterNew hand_splitter1;

MonoProcessorNew mono_processor0;
MonoProcessorNew mono_processor1;

Reprojector reprojector;

bool tracking = false;
bool initializing = false;

void init()
{
    tracking = false;
    CameraInitializerNew::init(camera);
    
    motion_processor0.init();
    motion_processor1.init();
    
    foreground_extractor0.init();
    foreground_extractor1.init();
    
    hand_splitter0.init();
    hand_splitter1.init();
    
    mono_processor0.init();
    mono_processor1.init();
}

void on_first_frame()
{
    serial_number = camera->getSerialNumber();
  
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
    ipc = new IPC("track_plus");
    reprojector.load(*ipc);
    
    init();
    
    if (!first_run)
    {
        if (process_running("menu_plus.exe") == 0)
            create_process(menu_file_path, "menu_plus.exe");
        
        ipc->send_message("menu_plus", "show initialization", "");
        // ipc->open_udp_channel("menu_plus");
        initializing = true;
    }
}
void update(Mat& image_in)
{
    static bool first_frame = true;
    if (first_frame)
    {
        on_first_frame();
        first_frame = false;
    }
    ipc->update();
    
    //----------------------------------------core algorithm----------------------------------------
    Mat image_flipped;
    flip(image_in, image_flipped, 0);
    
    Mat image0 = image_flipped(Rect(0, 0, 640, 480));
    Mat image1 = image_flipped(Rect(640, 0, 640, 480));
    
    Mat image_small0;
    Mat image_small1;
    resize(image0, image_small0, Size(160, 120), 0, 0, INTER_LINEAR);
    resize(image1, image_small1, Size(160, 120), 0, 0, INTER_LINEAR);
    
    Mat image_preprocessed0;
    Mat image_preprocessed1;
    Preprocessor::compute_channel_diff_image(image_small0, image_preprocessed0);
    Preprocessor::compute_channel_diff_image(image_small1, image_preprocessed1);
    
    // imshow("image_preprocessed0", image_preprocessed0);
    // imshow("image_preprocessed1", image_preprocessed1);
    
    motion_processor0.compute(image_small0, image_preprocessed0, "0", true);
    motion_processor1.compute(image_small1, image_preprocessed1, "1", false);
    
    static bool first_tracking_frame = true;
    if (tracking && motion_processor0.foreground_acquired && motion_processor1.foreground_acquired)
    {
        if (first_tracking_frame)
        {
            first_tracking_frame = false;
            
            motion_processor0.set_active_frame(10);
            motion_processor1.set_active_frame(10);
            
            // ipc->send_message("menu_plus", "show next", "");
            kill_process("menu_plus.exe");
        }
        
//        Mat image_foreground0 = foreground_extractor0.compute(image_preprocessed0, motion_processor0, "0");
//        Mat image_foreground1 = foreground_extractor1.compute(image_preprocessed1, motion_processor1, "1");
        
 //       bool success0 = hand_splitter0.compute(image_foreground0, motion_processor0.x_middle);
 //       if (success0)
  //          mono_processor0.compute(hand_splitter0.image_active_hand, hand_splitter0.blob_vec63);
    }
    
    if (!tracking && motion_processor0.started && motion_processor1.started && motion_processor0.gray_threshold > 0)
        if (CameraInitializerNew::adjust_exposure(camera, image_preprocessed0, motion_processor0.gray_threshold))
        {
            motion_processor0.exposure = CameraInitializerNew::exposure_current;
            motion_processor1.exposure = CameraInitializerNew::exposure_current;
            motion_processor0.construct_static_background_image = true;
            motion_processor1.construct_static_background_image = true;
            tracking = true;
        }
    
    waitKey(1);
    //
}

int main(int ac, char** av)
{
    
        camera = new Camera(true, 1280, 480, update);
        
        while (true)
        {
            cout << "running" << endl;
            cin.get();
        }
        
        return 0;

   // stopVideoStream();
    
}
