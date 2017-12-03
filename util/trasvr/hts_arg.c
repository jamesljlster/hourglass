#include <stdlib.h>
#include <args.h>

#include "hts.h"

char* ts_ip_def[] = {
	"0.0.0.0"
};

char* ts_port_def[] = {
	"7600"
};

char* ts_max_client_def[] = {
	"1"
};

char* ts_dev_timeout_def[] = {
	"3000"
};

char* ts_lstm_cfg_def[] = {
	"lstm.cfg"
};

char* ts_data_limit_def[] = {
	"5000"
};

args_t ts_arg_list[] = {
	{1, "host-ip", 'I', 1, ts_ip_def, "Server Setting", "Host IP dddress"},
	{1, "host-port", 'P', 1, ts_port_def, NULL, "Host port"},
	{1, "max-client", 'M', 1, ts_max_client_def, NULL, "Maximum client connection"},
	{1, "timeout", 'T', 1, ts_dev_timeout_def, NULL, "Operation timeout"},
	{1, "lstm-cfg", 'F', 1, ts_lstm_cfg_def, "Training Service Setting", "LSTM config file path"},
	{1, "data-limit", 'L', 1, ts_data_limit_def, NULL, "Maximum queue length of training data"},
	{0, "help", 'H', 0, NULL, "User Interface", "Print detail of arguments"},
	ARGS_TERMINATE
};

