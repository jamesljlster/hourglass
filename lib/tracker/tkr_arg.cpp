#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "tkr_argdef.hpp"
#include "tracker.hpp"

#define __run_chk(func, retVal, errLabel)                    \
    retVal = func;                                           \
    if (retVal < 0)                                          \
    {                                                        \
        printf("%s failed with error: %d\n", #func, retVal); \
        goto errLabel;                                       \
    }

#define __modcfg_get_str(strVal, cfg, nodeName, elemName, retVal, errLabel) \
    strVal = modcfg_get_content(cfg, nodeName, elemName);                   \
    if (strVal == NULL)                                                     \
    {                                                                       \
        printf("'%s' not found in '%s' setting!\n", nodeName, elemName);    \
        retVal = -1;                                                        \
        goto errLabel;                                                      \
    }

#define __modcfg_parse_double(val, mod, nodeName, elemName, retVal, errLabel) \
    {                                                                         \
        int __ret =                                                           \
            modcfg_parse_content_double(&val, mod, nodeName, elemName);       \
        if (__ret < 0)                                                        \
        {                                                                     \
            printf("Failed to parse '%s' setting in '%s' module\n", elemName, \
                   nodeName);                                                 \
            retVal = -1;                                                      \
            goto errLabel;                                                    \
        }                                                                     \
    }

#define __modcfg_parse_int(val, mod, nodeName, elemName, retVal, errLabel)    \
    {                                                                         \
        int __ret =                                                           \
            modcfg_parse_content_int(&val, mod, nodeName, elemName, 10);      \
        if (__ret < 0)                                                        \
        {                                                                     \
            printf("Failed to parse '%s' setting in '%s' module\n", elemName, \
                   nodeName);                                                 \
            retVal = -1;                                                      \
            goto errLabel;                                                    \
        }                                                                     \
    }

args_t arg_list[] = {
    {0, "cfg-path", 'C', 1, NULL, NULL, "Tracker config file path"},
    {0, "lstm-model", 'M', 1, NULL, NULL,
     "LSTM control model base (override config)"},
    {0, "help", 'H', 0, NULL, NULL, "Help"},
    ARGS_TERMINATE};

using namespace std;

namespace hourglass
{
bool Tracker::arg_parse(int argc, char* argv[])
{
    int ret = 0;
    MODCFG cfg = NULL;

    // Processing arguments
    __run_chk(args_parse(arg_list, argc, argv, NULL), ret, RET);

    // Check argument
#define __arg_chk(index)                                          \
    if (arg_list[index].enable <= 0)                              \
    {                                                             \
        printf("'%s' argument not set!\n", arg_list[index].name); \
        ret = -1;                                                 \
    }

    __arg_chk(TKRARG_LIST::TKRARG_CFG_PATH);

    if (ret < 0 || arg_list[TKRARG_LIST::TKRARG_HELP].enable > 0)
    {
        printf("\nUsage:\n");
        args_print_help(arg_list);
        printf("\n");

        goto RET;
    }

    // Read config file
    __run_chk(
        modcfg_create(&cfg, arg_list[TKRARG_LIST::TKRARG_CFG_PATH].leading[0]),
        ret, RET);

    // Parse arguments
    __run_chk(this->arg_parse_ctrl(cfg, arg_list), ret, RET);
    __run_chk(this->arg_parse_wheel_server(cfg), ret, RET);
    __run_chk(this->arg_parse_training_server(cfg), ret, RET);
    __run_chk(this->arg_parse_speed(cfg), ret, RET);
    __run_chk(this->arg_parse_cam(cfg), ret, RET);
    __run_chk(this->arg_parse_ft(cfg), ret, RET);
    __run_chk(this->arg_parse_log(cfg), ret, RET);

RET:
    modcfg_delete(cfg);
    return (ret >= 0);
}

int Tracker::arg_parse_ctrl(MODCFG cfg, args_t args[])
{
    int ret = 0;
    const char* tmpStr;

    // Parse control method
    __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_CTRL_METHOD, ret, RET);
    if (strcmp(tmpStr, TKRARG_CTRL_METHOD_PID) == 0)
    {
        this->ctrlMethod = TKR_CTRL_METHOD::TKR_CTRL_METHOD_PID;

        // Parse PID argument
        __run_chk(this->arg_parse_ctrl_pid(cfg), ret, RET);
    }
    else if (strcmp(tmpStr, TKRARG_CTRL_METHOD_LSTM) == 0)
    {
        this->ctrlMethod = TKR_CTRL_METHOD::TKR_CTRL_METHOD_LSTM;

        // Parse LSTM argument
        __run_chk(this->arg_parse_ctrl_lstm(cfg, args), ret, RET);
    }
    else if (strcmp(tmpStr, TKRARG_CTRL_METHOD_LEARN) == 0)
    {
        this->ctrlMethod = TKR_CTRL_METHOD::TKR_CTRL_METHOD_LEARN;

        // Parse learner argument
        __run_chk(this->arg_parse_ctrl_learn(cfg, args), ret, RET);
    }
    else
    {
        printf("Invalid '%s' setting: %s\n", TKRARG_CTRL_METHOD, tmpStr);
        ret = -1;
        goto RET;
    }

RET:
    return ret;
}

int Tracker::arg_parse_ctrl_pid(MODCFG cfg)
{
    int ret = 0;
    const char* tmpStr = NULL;

    // Get pid argument node
    __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_CTRL_PID_ARG, ret, RET);

    // Parse pid argument
    __modcfg_parse_double(this->pidArg.kp, cfg, tmpStr, TKRARG_CTRL_PID_ARG_KP,
                          ret, RET);
    __modcfg_parse_double(this->pidArg.kd, cfg, tmpStr, TKRARG_CTRL_PID_ARG_KD,
                          ret, RET);
    __modcfg_parse_double(this->pidArg.ki, cfg, tmpStr, TKRARG_CTRL_PID_ARG_KI,
                          ret, RET);

    __modcfg_parse_int(this->pidArg.baseSpeed, cfg, tmpStr,
                       TKRARG_CTRL_PID_SPEED_BASE, ret, RET);

RET:
    return ret;
}

int Tracker::arg_parse_ctrl_lstm(MODCFG cfg, args_t args[])
{
    int ret = 0;
    const char* tmpStr = NULL;
    const char* modelPath = NULL;

    if (args[TKRARG_LSTM_MODEL_OVERRIDE].enable > 0)
    {
        this->lstmArg.modelPath =
            string(args[TKRARG_LSTM_MODEL_OVERRIDE].leading[0]);
    }
    else
    {
        // Get lstm argument node
        __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_CTRL_LSTM_ARG, ret,
                         RET);

        // Parse lstm argument
        __modcfg_get_str(modelPath, cfg, tmpStr,
                         TKRARG_CTRL_LSTM_ARG_MODEL_PATH, ret, RET);

        this->lstmArg.modelPath = string(modelPath);
    }

RET:
    return ret;
}

int Tracker::arg_parse_ctrl_learn(MODCFG cfg, args_t args[])
{
    int ret = 0;
    const char* rootStr = NULL;
    const char* tmpStr = NULL;

    // Get learner argument node
    __modcfg_get_str(rootStr, cfg, TKRARG_ROOT, TKRARG_CTRL_LEARN_ARG, ret,
                     RET);

    // Parse model base path
    if (args[TKRARG_LSTM_MODEL_OVERRIDE].enable > 0)
    {
        this->learnArg.modelBase =
            string(args[TKRARG_LSTM_MODEL_OVERRIDE].leading[0]);
    }
    else
    {
        // Parse model base path
        __modcfg_get_str(tmpStr, cfg, rootStr, TKRARG_CTRL_LEARN_ARG_MODEL_BASE,
                         ret, RET);

        this->learnArg.modelBase = string(tmpStr);
    }

    // Parse model prefix
    __modcfg_get_str(tmpStr, cfg, rootStr, TKRARG_CTRL_LEARN_ARG_MODEL_PREFIX,
                     ret, RET);
    this->learnArg.modelPrefix = string(tmpStr);

    // Parse model extension
    __modcfg_get_str(tmpStr, cfg, rootStr, TKRARG_CTRL_LEARN_ARG_MODEL_EXT, ret,
                     RET);
    this->learnArg.modelExt = string(tmpStr);

    // Parse target mse
    __modcfg_parse_double(this->learnArg.targetMse, cfg, rootStr,
                          TKRARG_CTRL_LEARN_ARG_MODEL_TARGET_MSE, ret, RET);

    // Parse reinforcement learning rule thresholds
    __modcfg_parse_double(this->learnArg.speedUpWithDeltaDown, cfg, rootStr,
                          TKRARG_CTRL_LEARN_ARG_SPEED_UP_WITH_DELTA_DOWN, ret,
                          RET);
    __modcfg_parse_double(this->learnArg.speedUp, cfg, rootStr,
                          TKRARG_CTRL_LEARN_ARG_SPEED_UP, ret, RET);
    __modcfg_parse_double(this->learnArg.speedDeltaUp, cfg, rootStr,
                          TKRARG_CTRL_LEARN_ARG_SPEED_DELTA_UP, ret, RET);
    __modcfg_parse_double(this->learnArg.speedPreserve, cfg, rootStr,
                          TKRARG_CTRL_LEARN_ARG_SPEED_PRESERVE, ret, RET);
    __modcfg_parse_double(this->learnArg.speedBaseDown, cfg, rootStr,
                          TKRARG_CTRL_LEARN_ARG_SPEED_BASE_DOWN, ret, RET);
    __modcfg_parse_double(this->learnArg.speedDown, cfg, rootStr,
                          TKRARG_CTRL_LEARN_ARG_SPEED_DOWN, ret, RET);

    // Parse learning rate for RL
    __modcfg_parse_double(this->learnArg.speedLRate, cfg, rootStr,
                          TKRARG_CTRL_LEARN_ARG_SPEED_LEARNING_RATE, ret, RET);

    // Parse data send limit
    __modcfg_parse_int(this->learnArg.sendLimit, cfg, rootStr,
                       TKRARG_CTRL_LEARN_ARG_DATA_SEND_LIMIT, ret, RET);

RET:
    return ret;
}

int Tracker::arg_parse_wheel_server(MODCFG cfg)
{
    int ret = 0;
    int port;
    const char* tmpStr = NULL;

    // Parse wheel server ip
    __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_WHEEL_SERVER_IP, ret,
                     RET);
    this->wsvr.ip = string(tmpStr);

    // Parse wheel server port
    __modcfg_parse_int(port, cfg, TKRARG_ROOT, TKRARG_WHEEL_SERVER_PORT, ret,
                       RET);
    this->wsvr.port = port;

RET:
    return ret;
}

int Tracker::arg_parse_training_server(MODCFG cfg)
{
    int ret = 0;
    int port;
    const char* tmpStr = NULL;

    // Parse training server ip
    __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_TRAINING_SERVER_IP, ret,
                     RET);
    this->trasvr.ip = string(tmpStr);

    // Parse training server port
    __modcfg_parse_int(port, cfg, TKRARG_ROOT, TKRARG_TRAINING_SERVER_PORT, ret,
                       RET);
    this->trasvr.port = port;

RET:
    return ret;
}

int Tracker::arg_parse_speed(MODCFG cfg)
{
    int ret = 0;

    // Parse speed settings
    __modcfg_parse_int(this->speedMin, cfg, TKRARG_ROOT, TKRARG_SPEED_MIN, ret,
                       RET);
    __modcfg_parse_int(this->speedMax, cfg, TKRARG_ROOT, TKRARG_SPEED_MAX, ret,
                       RET);

RET:
    return ret;
}

int Tracker::arg_parse_cam(MODCFG cfg)
{
    int ret = 0;

    // Parse camera settings
    __modcfg_parse_int(this->camIndex, cfg, TKRARG_ROOT, TKRARG_CAM_INDEX, ret,
                       RET);
    __modcfg_parse_int(this->camWidth, cfg, TKRARG_ROOT, TKRARG_CAM_WIDTH, ret,
                       RET);
    __modcfg_parse_int(this->camHeight, cfg, TKRARG_ROOT, TKRARG_CAM_HEIGHT,
                       ret, RET);

RET:
    return ret;
}

int Tracker::arg_parse_ft(MODCFG cfg)
{
    int ret = 0;
    const char* tmpStr;

    // Parse feature type
    __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_FT_TYPE, ret, RET);
    if (strcmp(tmpStr, TKRARG_FT_TYPE_LANE) == 0)
    {
        this->ftType = laneft::LANE_TYPE::LANE;
    }
    else if (strcmp(tmpStr, TKRARG_FT_TYPE_LINE) == 0)
    {
        this->ftType = laneft::LANE_TYPE::LINE;
    }
    else
    {
        printf("Invalid '%s' setting: %s\n", TKRARG_FT_TYPE, tmpStr);
        ret = -1;
        goto RET;
    }

    // Parse filter
    __modcfg_parse_int(this->ftLineHeightFilter, cfg, TKRARG_ROOT,
                       TKRARG_FT_LINE_HEIGHT_FILTER, ret, RET);

RET:
    return ret;
}

int Tracker::arg_parse_log(MODCFG cfg)
{
    int ret = 0;
    const char* tmpStr;

    // Parse log file setting
    __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_LOG_BASE, ret, RET);
    this->logBase = string(tmpStr);

    __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_LOG_EXT, ret, RET);
    this->logExt = string(tmpStr);

RET:
    return ret;
}

void Tracker::arg_print()
{
    // Print arguments
}

void Tracker::arg_print_ctrl()
{
    printf("%s:\n", TKRARG_CTRL_METHOD);
    switch (this->ctrlMethod)
    {
        case TKR_CTRL_METHOD_PID:
            printf("\t%s = %s\n", TKRARG_CTRL_METHOD, TKRARG_CTRL_METHOD_PID);
            this->arg_print_ctrl_pid();
            break;

        case TKR_CTRL_METHOD_LSTM:
            printf("\t%s = %s\n", TKRARG_CTRL_METHOD, TKRARG_CTRL_METHOD_LSTM);
            this->arg_print_ctrl_lstm();
            break;

        case TKR_CTRL_METHOD_LEARN:
            printf("\t%s = %s\n", TKRARG_CTRL_METHOD, TKRARG_CTRL_METHOD_LEARN);
            this->arg_print_ctrl_learn();
            break;
    }
}

void Tracker::arg_print_ctrl_pid()
{
    printf("%s:\n", TKRARG_CTRL_PID_ARG);

    printf("\t%s = %lf\n", TKRARG_CTRL_PID_ARG_KP, this->pidArg.kp);
    printf("\t%s = %lf\n", TKRARG_CTRL_PID_ARG_KI, this->pidArg.ki);
    printf("\t%s = %lf\n", TKRARG_CTRL_PID_ARG_KD, this->pidArg.kd);

    printf("\t%s = %d\n", TKRARG_CTRL_PID_SPEED_BASE, this->pidArg.baseSpeed);
}

void Tracker::arg_print_ctrl_lstm()
{
    printf("%s:\n", TKRARG_CTRL_LSTM_ARG);

    printf("\t%s = %s\n", TKRARG_CTRL_LSTM_ARG_MODEL_PATH,
           this->lstmArg.modelPath.c_str());
}

void Tracker::arg_print_ctrl_learn()
{
    printf("%s:\n", TKRARG_CTRL_LEARN_ARG);

    printf("\t%s = %s\n", TKRARG_CTRL_LEARN_ARG_MODEL_BASE,
           this->learnArg.modelBase.c_str());

    printf("\t%s = %s\n", TKRARG_CTRL_LEARN_ARG_MODEL_PREFIX,
           this->learnArg.modelPrefix.c_str());
    printf("\t%s = %s\n", TKRARG_CTRL_LEARN_ARG_MODEL_EXT,
           this->learnArg.modelExt.c_str());

    printf("\t%s = %lf\n", TKRARG_CTRL_LEARN_ARG_MODEL_TARGET_MSE,
           this->learnArg.targetMse);

    printf("\t%s = %lf\n", TKRARG_CTRL_LEARN_ARG_SPEED_UP_WITH_DELTA_DOWN,
           this->learnArg.speedUpWithDeltaDown);
    printf("\t%s = %lf\n", TKRARG_CTRL_LEARN_ARG_SPEED_UP,
           this->learnArg.speedUp);
    printf("\t%s = %lf\n", TKRARG_CTRL_LEARN_ARG_SPEED_DELTA_UP,
           this->learnArg.speedDeltaUp);
    printf("\t%s = %lf\n", TKRARG_CTRL_LEARN_ARG_SPEED_PRESERVE,
           this->learnArg.speedPreserve);
    printf("\t%s = %lf\n", TKRARG_CTRL_LEARN_ARG_SPEED_BASE_DOWN,
           this->learnArg.speedBaseDown);
    printf("\t%s = %lf\n", TKRARG_CTRL_LEARN_ARG_SPEED_DOWN,
           this->learnArg.speedDown);

    printf("\t%s = %lf\n", TKRARG_CTRL_LEARN_ARG_SPEED_LEARNING_RATE,
           this->learnArg.speedLRate);

    printf("\t%s = %d\n", TKRARG_CTRL_LEARN_ARG_DATA_SEND_LIMIT,
           this->learnArg.sendLimit);
}

}  // namespace hourglass
