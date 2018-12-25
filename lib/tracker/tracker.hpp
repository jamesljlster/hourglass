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

        int baseSpeed;
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
    Tracker(int argc, char* argv[]);
    ~Tracker();

    bool svc_connect();
    void svc_disconnect();

    void arg_print();

    void reinf_speed(float err, int sal, int sar, int* reSalPtr, int* reSarPtr);

    std::string make_log_fname(std::string suffix);
    std::string make_model_fname(std::string suffix);

    void start_new_log(std::string suffix);
    void dump_info(float offset, float sal, float sar);

    float get_norm_feature();
    void get_norm_ctrl_speed(float ft, float* salPtr, float* sarPtr);
    void wheel_ctrl(float normSal, float normSar);
    void wheel_stop();

    void send_data(float* ptr, int len);
    int get_data_limit();
    float get_training_mse();
    void get_model(lstm_t* lstmPtr);
    void replace_model(lstm_t lstm);

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

    int camIndex;
    int camWidth;
    int camHeight;

    laneft::LANE_TYPE ftType;
    int ftLineHeightFilter;

    std::string logBase;
    std::string logExt;

    lstm_t model = NULL;
    SPID sPid;

    ftsvc ft;
    std::fstream fLog;

    int wcltStatus = 0;
    wclt_t wclt;

    int tsStatus = 0;
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

    void arg_print_ctrl();
    void arg_print_ctrl_pid();
    void arg_print_ctrl_lstm();
    void arg_print_ctrl_learn();
    void arg_print_wheel_server();
    void arg_print_training_server();
    void arg_print_speed();
    void arg_print_cam();
    void arg_print_ft();
    void arg_print_log();

    struct tm get_current_date();
    std::string make_time_str();

    void load_lstm_model();
};

}  // namespace hourglass

#endif
