#ifndef __TKR_ARGDEF_HPP__
#define __TKR_ARGDEF_HPP__

#define TKRARG_ROOT "tracker"

#define TKRARG_CTRL_METHOD "ctrl_method"
#define TKRARG_CTRL_METHOD_PID "pid"
#define TKRARG_CTRL_METHOD_LSTM "lstm"
#define TKRARG_CTRL_PID_ARG "ctrl_pid_arg"
#define TKRARG_CTRL_PID_ARG_KP "kp"
#define TKRARG_CTRL_PID_ARG_KD "kd"
#define TKRARG_CTRL_PID_ARG_KI "ki"
#define TKRARG_CTRL_LSTM_ARG "ctrl_lstm_arg"
#define TKRARG_CTRL_LSTM_ARG_MODEL_PATH "model_path"

#define TKRARG_WHEEL_SERVER_IP "wheel_server_ip"
#define TKRARG_WHEEL_SERVER_PORT "wheel_server_port"

#define TKRARG_TRAINING_SERVER_IP "training_server_ip"
#define TKRARG_TRAINING_SERVER_PORT "training_server_port"

#define TKRARG_SPEED_MIN "speed_min"
#define TKRARG_SPEED_MAX "speed_max"
#define TKRARG_SPEED_BASE "speed_base"

#define TKRARG_CAM_INDEX "cam_index"
#define TKRARG_CAM_WIDTH "cam_width"
#define TKRARG_CAM_HEIGHT "cam_height"

#define TKRARG_FT_TYPE "ft_type"
#define TKRARG_FT_LINE_HEIGHT_FILTER "ft_line_height_filter"

#define TKRARG_LOG_BASE "log_base"
#define TKRARG_LOG_EXT "log_ext"

#define TKRARG_PID_PK "kp"
#define TKRARG_PID_KD "kd"
#define TKRARG_PID_KI "ki"

#define TKRARG_LSTM_MODEL_PATH "model_path"

namespace hourglass
{
enum TKRARG_LIST
{
    TKRARG_CFG_PATH,
    TKRARG_LSTM_MODEL_OVERRIDE,
    TKRARG_HELP
};
}

#endif
