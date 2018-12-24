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
   public:
    // Member variables
    int stop;

    // Member functions
    bool init(int argc, char* argv[]);
    void cleanup();

    bool svc_connect();
    bool svc_disconnect();

   protected:
    // Member enumeration
    enum TKR_CTRL_METHOD
    {
        TKR_CTRL_METHOD_PID,
        TKR_CTRL_METHOD_LSTM
    };

    // Member class
    class TkrNet
    {
       public:
        std::string ip;
        int port;
    };

    // Member variables
    enum TKR_CTRL_METHOD ctrlMethod;

    ftsvc ft;
    std::fstream fLog;

    TkrNet wsvr;
    TkrNet trasvr;

    std::string modelBasePath;
    lstm_t model;
    SPID sPid;

    int wcltStatus;
    wclt_t wclt;

    int tsStatus;
    trasvc_client_t ts;

    // Member functions
    bool arg_parse(int argc, char* argv[]);

    int arg_parse_ctrl(MODCFG cfg, args_t args[]);
    int arg_parse_wheel_server(MODCFG cfg);
    int arg_parse_training_server(MODCFG cfg);
    int arg_parse_speed(MODCFG cfg);
    int arg_parse_cam(MODCFG cfg);
    int arg_parse_ft(MODCFG cfg);
    int arg_parse_log(MODCFG cfg);
};

}  // namespace hourglass

#endif
