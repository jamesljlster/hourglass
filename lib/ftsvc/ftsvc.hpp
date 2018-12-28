#ifndef __FTSVC_HPP__
#define __FTSVC_HPP__

#include <pthread.h>
#include <string>

#include <opencv2/opencv.hpp>

#include <laneft_ocv.hpp>

namespace hourglass
{
class ftsvc
{
   public:
    // Constructor
    ftsvc();

    // Destructor
    ~ftsvc();

    // Open cam function
    bool open_cam(int camIndex, int width, int height);
    bool open_cam(std::string& camPath);

    // Feature setting
    void set_lane_type(laneft::LANE_TYPE laneType);

    void set_find_point_rule(int maskSize, int threshold);
    void set_find_line_rule(int maxDist, int threshold);
    void set_line_height_filter(int threshold);

    // Show setting
    void set_image_show(int enable);

    // Task start and stop

    // Get normalized feature
    float get_norm_feature();

    // User input
    char kbin;

   protected:
    int showImg;

    int thStatus;
    pthread_t th;

    cv::VideoCapture cam;
    cv::Mat img;
    laneft_ocv laneFt = laneft_ocv(laneft::LANE_TYPE::LINE);
};

}  // namespace hourglass

#endif
