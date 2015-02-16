#ifndef ARIFTCONTROL_H
#define ARIFTCONTROL_H
#include <opencv2/core/core.hpp>
#include "CameraInputHandler.h"
#include "IDSuEyeInputHandler.h"

#define CAM1 1
#define CAM2 2
#define RIFT_RESOLUTION_WIDTH 1280
#define RIFT_RESOLUTION_HEIGHT 800

class ARiftControl
{
  public:
    ARiftControl();
    virtual ~ARiftControl();
    void init();
    bool getImages();
    void createDisplay();

    void handleKey(char key);
    void hanldeFlip();
    void handleSave();
    void handleCameraAutoFeatures();

    void start() {running = true;};
    void stop() {running = false;};
    bool keepRunning() {return running;};

    void undistortImages();

    cv::Mat left_pic;
    cv::Mat right_pic;
    cv::Mat left_resized;
    cv::Mat right_resized;
    cv::Mat left_undistorted;
    cv::Mat right_undistorted;
    cv::Mat full_view;
    cv::Mat full_view_undist;
    std::string base_save_name;

    cv::Mat left_camera_mat;
    cv::Mat right_camera_mat;
    cv::Mat left_distortion;
    cv::Mat right_distortion;
    cv::Mat left_map1;
    cv::Mat left_map2;
    cv::Mat right_map1;
    cv::Mat right_map2;
    int wait_time = 30;
  protected:
  private:
    bool running = false;
    int write_counter = 0;
    CameraInputHandler *cam_input = NULL;
};

#endif // ARIFTCONTROL_H