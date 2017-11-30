#ifndef __HTS_H__
#define __HTS_H__

enum TS_ARG
{
	TS_ARG_HOST_IP,
	TS_ARG_HOST_PORT,
	TS_ARG_MAX_CLIENT,
	TS_ARG_TIMEOUT,
	TS_ARG_LSTM_CFG_PATH,
	TS_ARG_DATA_LIMIT,
	TS_ARG_HELP
};

extern char* ts_ip_def[];
extern char* ts_port_def[];
extern char* ts_max_client_def[];
extern char* ts_dev_timeout_def[];
extern char* ts_lstm_cfg_def[];
extern char* ts_data_limit_def[];
extern args_t ts_arg_list[];

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
