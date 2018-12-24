#ifndef __TKR_ARGDEF_HPP__
#define __TKR_ARGDEF_HPP__

#define TKRARG_ROOT "tracker"

#define TKRARG_CTRL_METHOD "ctrl_method"
#define TKRARG_CTRL_METHOD_PID "pid"
#define TKRARG_CTRL_METHOD_LSTM "lstm"
#define TKRARG_CTRL_METHOD_LEARN "learn"

#define TKRARG_CTRL_PID_ARG "ctrl_pid_arg"
#define TKRARG_CTRL_PID_ARG_KP "kp"
#define TKRARG_CTRL_PID_ARG_KD "kd"
#define TKRARG_CTRL_PID_ARG_KI "ki"
#define TKRARG_CTRL_PID_SPEED_BASE "speed_base"

#define TKRARG_CTRL_LSTM_ARG "ctrl_lstm_arg"
#define TKRARG_CTRL_LSTM_ARG_MODEL_PATH "model_path"

#define TKRARG_CTRL_LEARN_ARG "ctrl_learn_arg"
#define TKRARG_CTRL_LEARN_ARG_MODEL_BASE "model_base"
#define TKRARG_CTRL_LEARN_ARG_MODEL_PREFIX "model_prefix"
#define TKRARG_CTRL_LEARN_ARG_MODEL_EXT "model_ext"
#define TKRARG_CTRL_LEARN_ARG_MODEL_TARGET_MSE "model_target_mse"
#define TKRARG_CTRL_LEARN_ARG_SPEED_UP_WITH_DELTA_DOWN \
    "speed_up_with_delta_down"
#define TKRARG_CTRL_LEARN_ARG_SPEED_UP "speed_up"
#define TKRARG_CTRL_LEARN_ARG_SPEED_DELTA_UP "speed_delta_up"
#define TKRARG_CTRL_LEARN_ARG_SPEED_PRESERVE "speed_preserve"
#define TKRARG_CTRL_LEARN_ARG_SPEED_BASE_DOWN "speed_base_down"
#define TKRARG_CTRL_LEARN_ARG_SPEED_DOWN "speed_down"
#define TKRARG_CTRL_LEARN_ARG_SPEED_LEARNING_RATE "speed_learning_rate"
#define TKRARG_CTRL_LEARN_ARG_DATA_SEND_LIMIT "data_send_limit"

#define TKRARG_WHEEL_SERVER_IP "wheel_server_ip"
#define TKRARG_WHEEL_SERVER_PORT "wheel_server_port"

#define TKRARG_TRAINING_SERVER_IP "training_server_ip"
#define TKRARG_TRAINING_SERVER_PORT "training_server_port"

#define TKRARG_SPEED_MIN "speed_min"
#define TKRARG_SPEED_MAX "speed_max"

#define TKRARG_CAM_INDEX "cam_index"
#define TKRARG_CAM_WIDTH "cam_width"
#define TKRARG_CAM_HEIGHT "cam_height"

#define TKRARG_FT_TYPE "ft_type"
#define TKRARG_FT_TYPE_LANE "lane"
#define TKRARG_FT_TYPE_LINE "line"
#define TKRARG_FT_LINE_HEIGHT_FILTER "ft_line_height_filter"

#define TKRARG_LOG_BASE "log_base"
#define TKRARG_LOG_EXT "log_ext"

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
