#include <exception>

#include "ftsvc.hpp"

#include "debug.h"

#define FT_WNAME "Lane detection"

using namespace std;
using namespace cv;

namespace hourglass
{
ftsvc::ftsvc()
{
    this->thStatus = 0;
    this->showImg = 0;
    this->kbin = 0;
}

void ftsvc::set_image_show(int enable)
{
    if (enable)
    {
        this->showImg = 1;
    }
    else
    {
        this->showImg = 0;
        destroyWindow(FT_WNAME);
    }
}

float ftsvc::get_norm_feature()
{
    // Read frame
    if (!this->cam.read(this->img))
    {
        throw runtime_error("Failed to read frame form VideoCapture!");
    }

    // Get feature
    Mat canny;
    Canny(this->img, canny, 255, 255);
    float ftTmp = this->laneFt.get_feature(canny);

    // Draw image
    if (this->showImg)
    {
        this->laneFt.draw_line_onto(this->img);
        imshow(FT_WNAME, this->img);
        this->kbin = waitKey(1);
    }

    // Normalize
    ftTmp /= (float)(this->img.cols / 2);

    return ftTmp;
}

bool ftsvc::open_cam(int camIndex, int width, int height)
{
    bool ret = this->cam.open(camIndex);
    if (!ret)
    {
        return ret;
    }

    ret = this->cam.set(CV_CAP_PROP_FRAME_WIDTH, width);
    if (!ret)
    {
        this->cam.release();
        return ret;
    }

    ret = this->cam.set(CV_CAP_PROP_FRAME_HEIGHT, height);
    if (!ret)
    {
        this->cam.release();
        return ret;
    }

    return ret;
}

bool ftsvc::open_cam(string& camPath) { return this->cam.open(camPath); }

ftsvc::~ftsvc()
{
    if (thStatus > 0)
    {
        pthread_cancel(this->th);
        pthread_join(this->th, NULL);
    }
}

void ftsvc::set_lane_type(laneft::LANE_TYPE laneType)
{
    this->laneFt.set_lane_type(laneType);
}

void ftsvc::set_find_point_rule(int maskSize, int threshold)
{
    this->laneFt.set_find_point_rule(maskSize, threshold);
}

void ftsvc::set_find_line_rule(int maxDist, int threshold)
{
    this->laneFt.set_find_line_rule(maxDist, threshold);
}

void ftsvc::set_line_height_filter(int threshold)
{
    this->laneFt.set_line_height_filter(threshold);
}
}  // namespace hourglass
