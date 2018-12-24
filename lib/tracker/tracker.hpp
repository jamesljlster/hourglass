#ifndef __TRACKER_HPP__
#define __TRACKER_HPP__

#include <fstream>
#include <string>

#include <SPID.h>
#include <args.h>
#include <lstm.h>
#include <modcfg.h>
#include <trasvc.h>
#include <wclt.h>
#include <ftsvc.hpp>

namespace hourglass
{
class Tracker
{
   protected:
    // Member enumeration
    enum TKR_CTRL_METHOD
    {
        TKR_CTRL_METHOD_PID,
        TKR_CTRL_METHOD_LSTM,
        TKR_CTRL_METHOD_LEARN
    };

    // Member class
    class TkrNet
    {
       public:
        std::string ip;
        int port;
    };

    class PidArg
    {
       public:
        double kp;
        double ki;
        double kd;
    };

    class LstmArg
    {
       public:
        std::string modelPath;
    };

    class LearnArg
    {
       public:
        std::string modelBase;
        std::string modelPrefix;
        std::string modelExt;

        double targetMse;

        double speedUpWithDeltaDown;
        double speedUp;
        double speedDeltaUp;
        double speedPreserve;
        double speedBaseDown;
        double speedDown;

        double speedLRate;

        int sendLimit;
    };

   public:
    // Member variables
    int stop;

    // Member functions
    bool init(int argc, char* argv[]);
    void cleanup();

    bool svc_connect();
    bool svc_disconnect();

   protected:
    // Member variables
    enum TKR_CTRL_METHOD ctrlMethod;
    PidArg pidArg;
    LstmArg lstmArg;
    LearnArg learnArg;

    TkrNet wsvr;
    TkrNet trasvr;

    int speedMin;
    int speedMax;
    int speedBase;

    int camIndex;
    int camWidth;
    int camHeight;

    laneft::LANE_TYPE ftType;
    int ftLineHeightFilter;

    std::string logBase;
    std::string logExt;

    lstm_t model;
    SPID sPid;

    ftsvc ft;
    std::fstream fLog;

    int wcltStatus;
    wclt_t wclt;

    int tsStatus;
    trasvc_client_t ts;

    // Member functions
    bool arg_parse(int argc, char* argv[]);

    int arg_parse_ctrl(MODCFG cfg, args_t args[]);
    int arg_parse_ctrl_pid(MODCFG cfg);
    int arg_parse_ctrl_lstm(MODCFG cfg, args_t args[]);
    int arg_parse_ctrl_learn(MODCFG cfg, args_t args[]);
    int arg_parse_wheel_server(MODCFG cfg);
    int arg_parse_training_server(MODCFG cfg);
    int arg_parse_speed(MODCFG cfg);
    int arg_parse_cam(MODCFG cfg);
    int arg_parse_ft(MODCFG cfg);
    int arg_parse_log(MODCFG cfg);
};

}  // namespace hourglass

#endif
