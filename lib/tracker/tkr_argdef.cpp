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

    // Parse control method
    __run_chk(this->arg_parse_ctrl(cfg, arg_list), ret, RET);

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
    }
    else if (strcmp(tmpStr, TKRARG_CTRL_METHOD_LSTM) == 0)
    {
        this->ctrlMethod = TKR_CTRL_METHOD::TKR_CTRL_METHOD_LSTM;
    }
    else
    {
        printf("Invalid 'ctrl_method' setting: %s\n", tmpStr);
        ret = -1;
        goto RET;
    }

    // Parse control argument
    if (this->ctrlMethod == TKR_CTRL_METHOD::TKR_CTRL_METHOD_PID)
    {
        double kp, ki, kd;

        // Get pid argument node
        __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_CTRL_PID_ARG, ret,
                         RET);

        // Parse pid argument
        __modcfg_parse_double(kp, cfg, tmpStr, TKRARG_CTRL_PID_ARG_KP, ret,
                              RET);
        __modcfg_parse_double(kd, cfg, tmpStr, TKRARG_CTRL_PID_ARG_KD, ret,
                              RET);
        __modcfg_parse_double(ki, cfg, tmpStr, TKRARG_CTRL_PID_ARG_KI, ret,
                              RET);
    }
    else
    {
        if (args[TKRARG_LSTM_MODEL_OVERRIDE].enable > 0)
        {
            this->modelBasePath =
                string(args[TKRARG_LSTM_MODEL_OVERRIDE].leading[0]);
        }
        else
        {
            const char* modelPath = NULL;

            // Get lstm argument node
            __modcfg_get_str(tmpStr, cfg, TKRARG_ROOT, TKRARG_CTRL_LSTM_ARG,
                             ret, RET);

            // Parse lstm argument
            __modcfg_get_str(modelPath, cfg, tmpStr,
                             TKRARG_CTRL_LSTM_ARG_MODEL_PATH, ret, RET);

            this->modelBasePath = string(modelPath);
        }
    }

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

}  // namespace hourglass
