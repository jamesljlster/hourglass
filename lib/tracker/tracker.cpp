#include <exception>
#include <stdexcept>

#include "tkr_private.hpp"
#include "tracker.hpp"

#define BUF_SIZE 128

using namespace std;

namespace hourglass
{
Tracker::Tracker(int argc, char* argv[])
{
    // Parse argument
    if (!this->arg_parse(argc, argv))
    {
        throw runtime_error("Argument parsing failed!");
    }

    // Connect to service
    if (!this->svc_connect())
    {
        throw runtime_error("Service initialization failed!");
    }

    // Open camera
    if (!this->ft.open_cam(this->camIndex, this->camWidth, this->camHeight))
    {
        throw runtime_error("Failed to open camera!");
    }

    // Setup feature
    this->ft.set_line_height_filter(this->ftLineHeightFilter);
    this->ft.set_image_show(1);
}

Tracker::~Tracker() { this->svc_disconnect(); }

bool Tracker::svc_connect()
{
    int ret;

    // Connect wheel service
    __run_chk(wclt_connect(&this->wclt, this->wsvr.ip.c_str(), this->wsvr.port),
              ret, RET);
    this->wcltStatus = 1;

    if (this->ctrlMethod == Tracker::TKR_CTRL_METHOD::TKR_CTRL_METHOD_LEARN)
    {
        // Connect training service
        __ts_run_chk(trasvc_client_connect(&this->ts, this->trasvr.ip.c_str(),
                                           this->trasvr.port),
                     ret, ERR);
        this->tsStatus = 1;

        // Clear training data
        __ts_run_chk(trasvc_client_clear(this->ts), ret, ERR);

        // Upload lstm base model
        __ts_run_chk(trasvc_client_model_upload(
                         this->ts, this->learnArg.modelBase.c_str()),
                     ret, ERR);

        // Start training service
        __ts_run_chk(trasvc_client_start(this->ts), ret, ERR);
    }

ERR:
    this->svc_disconnect();

RET:
    return (ret >= 0);
}

void Tracker::svc_disconnect()
{
    if (this->wcltStatus)
    {
        wclt_disconnect(this->wclt);
        this->wcltStatus = 0;
    }

    if (this->tsStatus)
    {
        trasvc_client_disconnect(this->ts);
        this->tsStatus = 0;
    }
}

void Tracker::reinf_speed(float err, int sal, int sar, int* reSalPtr,
                          int* reSarPtr)
{
    int tmpSal, tmpSar;

    // Reset offset
    sal = sal - 255;
    sar = sar - 255;

    // Set default
    tmpSal = sal;
    tmpSar = sar;

    int baseSpeed = (sal + sar) / 2.0;
    int speedDelta = sar - baseSpeed;
    float absErr = fabs(err);

    if (absErr <= this->learnArg.speedUpWithDeltaDown)
    {
        baseSpeed = baseSpeed * (1.0 + this->learnArg.speedLRate);
        speedDelta = speedDelta * (1.0 - this->learnArg.speedLRate);

        tmpSal = baseSpeed - speedDelta;
        tmpSar = baseSpeed + speedDelta;
    }
    else if (absErr <= this->learnArg.speedUp)
    {
        tmpSal = sal * (1.0 + this->learnArg.speedLRate);
        tmpSar = sar * (1.0 + this->learnArg.speedLRate);
    }
    else if (absErr <= this->learnArg.speedDeltaUp)
    {
        speedDelta = speedDelta * (1.0 + this->learnArg.speedLRate);
        tmpSal = baseSpeed - speedDelta;
        tmpSar = baseSpeed + speedDelta;
    }
    else if (absErr <= this->learnArg.speedPreserve)
    {
        // No change
        tmpSal = sal;
        tmpSar = sar;
    }
    else if (absErr <= this->learnArg.speedBaseDown)
    {
        speedDelta = speedDelta * (1.0 - this->learnArg.speedLRate);
        tmpSal = baseSpeed - speedDelta;
        tmpSar = baseSpeed + speedDelta;
    }
    else if (absErr <= this->learnArg.speedDown)
    {
        tmpSal = sal * (1.0 - this->learnArg.speedLRate);
        tmpSar = sar * (1.0 - this->learnArg.speedLRate);
    }

    // Restore offset
    tmpSal = tmpSal + 255;
    tmpSar = tmpSar + 255;

    // Check if speed out of range
    if (tmpSal > this->speedMax)
    {
        tmpSal = this->speedMax;
    }
    else if (tmpSal < this->speedMin)
    {
        tmpSal = this->speedMin;
    }

    if (tmpSar > this->speedMax)
    {
        tmpSar = this->speedMax;
    }
    else if (tmpSar < this->speedMin)
    {
        tmpSar = this->speedMin;
    }

    // Assign value
    *reSalPtr = tmpSal;
    *reSarPtr = tmpSar;
}

string Tracker::make_log_fname(string suffix)
{
    // Make filename
    string ret = "";

    ret += this->logBase;
    ret += this->make_time_str();
    if (suffix.size())
    {
        ret += suffix;
    }
    ret += this->logExt;

    return ret;
}

string Tracker::make_model_fname(string suffix)
{
    // Check control mode
    if (this->ctrlMethod != Tracker::TKR_CTRL_METHOD::TKR_CTRL_METHOD_LEARN)
    {
        throw runtime_error("Running in imcompatible control method!");
    }

    // Make filename
    string ret = "";
    ret += this->learnArg.modelPrefix;
    ret += this->make_time_str();
    if (suffix.size())
    {
        ret += suffix;
    }
    ret += this->learnArg.modelExt;

    return ret;
}

struct tm Tracker::get_current_date()
{
    time_t t;
    struct tm tm;

    t = time(NULL);
    tm = *localtime(&t);

    tm.tm_year += 1990;
    tm.tm_mon += 1;

    return tm;
}

string Tracker::make_time_str()
{
    char strBuf[BUF_SIZE] = {0};
    struct tm date = get_current_date();

    snprintf(strBuf, BUF_SIZE, "%d%02d%02d_%02d%02d", date.tm_year, date.tm_mon,
             date.tm_mday, date.tm_hour, date.tm_min);

    string ret = strBuf;
    return ret;
}

void Tracker::start_new_log(string suffix)
{
    // Close file stream
    if (this->fLog.is_open())
    {
        this->fLog.close();
    }

    // Open file stream
    string logPath = this->make_log_fname(suffix);
    this->fLog.open(logPath, ios_base::out);
    if (!fLog.is_open())
    {
        throw runtime_error(string("Failed to open log file with path: ") +
                            logPath);
    }

    // Dump csv header
    fLog << "offset,sal,sar" << endl;
}

void Tracker::dump_info(float offset, float sal, float sar)
{
    // Check file stream
    if (!this->fLog.is_open())
    {
        throw runtime_error("Log file is not opened");
    }

    // Dump log
    this->fLog << offset << "," << sal << "," << sar << endl;
}

}  // namespace hourglass
