#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tcpmgr.h>
#include <trasvc.h>
#include <args.h>

#define BUF_SIZE	128

#define __strtol(var, str, errMsg, ...) \
{ \
	char* tmpPtr; \
	var = strtol(str, &tmpPtr, 10); \
	if(tmpPtr == str) \
	{ \
		printf(errMsg, ##__VA_ARGS__); \
		ret = -1; \
		goto RET; \
	} \
}

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

char* ts_ip_def[] = {
	"0.0.0.0"
};

char* ts_port_def[] = {
	"7500"
};

char* ts_max_client_def[] = {
	"5"
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

int main(int argc, char* argv[])
{
	int i;
	int ret;
	char tmpRead;
	char buf[BUF_SIZE] = {0};

	const char* hostIP;
	int hostPort, maxClient;
	//int timeout;
	int dataLimit;

	lstm_config_t lstmCfg = NULL;

	tcpmgr_t mgr = NULL;
	trasvc_t ts = NULL;

	// Process arguments
	ret = args_parse(ts_arg_list, argc, argv, NULL);
	if(ret < 0)
	{
		goto RET;
	}

	if(ts_arg_list[TS_ARG_HELP].enable)
	{
		args_print_help(ts_arg_list);
		goto RET;
	}

	// Parsing argument
	hostIP = ts_arg_list[TS_ARG_HOST_IP].leading[0];

	__strtol(hostPort, ts_arg_list[TS_ARG_HOST_PORT].leading[0],
			"Failed to convert '%s' to host port!\n",
			ts_arg_list[TS_ARG_HOST_PORT].leading[0]
			);

	__strtol(maxClient, ts_arg_list[TS_ARG_MAX_CLIENT].leading[0],
			"Failed to convert '%s' to maximum client limitation!\n",
			ts_arg_list[TS_ARG_MAX_CLIENT].leading[0]
			);

//	__strtol(timeout, ts_arg_list[TS_ARG_TIMEOUT].leading[0],
//			"Failed to convert '%s' to timeout!\n",
//			ts_arg_list[TS_ARG_TIMEOUT].leading[0]
//			);

	__strtol(dataLimit, ts_arg_list[TS_ARG_DATA_LIMIT].leading[0],
			"Failed to convert '%s' to training data limit!\n",
			ts_arg_list[TS_ARG_DATA_LIMIT].leading[0]
			);

	// Print summary
	printf("Summary:\n");
	args_print_summary(ts_arg_list);

	// Import lstm config
	ret = lstm_config_import(&lstmCfg, ts_arg_list[TS_ARG_LSTM_CFG_PATH].leading[0]);
	if(ret < 0)
	{
		printf("lstm_config_import() failed with error: %d\n", ret);
		goto RET;
	}

	// Create tcpmgr
	ret = tcpmgr_create(&mgr, hostIP, hostPort, maxClient);
	if(ret < 0)
	{
		printf("Server initialization failed!\n");
		goto RET;
	}

	// Create trasvc
	ret = trasvc_create(&ts, lstmCfg, dataLimit);
	if(ret < 0)
	{
		printf("Training service initialization failed with error: %d\n", ret);
		goto RET;
	}

	// Start trasvc
	ret = trasvc_start(ts);
	if(ret < 0)
	{
		printf("Start training service failed with error: %d\n", ret);
		goto RET;
	}

	// Start tcpmgr
	ret = tcpmgr_start(mgr, trasvc_client_task, ts);
	if(ret < 0)
	{
		printf("tcpmgr_start() failed with error: %d\n", ret);
		goto RET;
	}

	while(1)
	{
		for(i = 0; i < BUF_SIZE; i++)
		{
			ret = scanf("%c", &tmpRead);
			if(ret <= 0)
			{
				continue;
			}

			if(tmpRead == '\n')
			{
				buf[i] = '\0';
				break;
			}
			else
			{
				buf[i] = tmpRead;
			}
		}

		if(strcmp(buf, "stop") == 0)
		{
			break;
		}
		else if(strcmp(buf, "restart") == 0)
		{
			tcpmgr_stop(mgr);

			ret = tcpmgr_start(mgr, trasvc_client_task, ts);
			if(ret < 0)
			{
				printf("tcpmgr_start() failed with error: %d\n", ret);
				goto RET;
			}
		}
	}

	// Stop trasvc
	trasvc_stop(ts);

	// Stop tcpmgr
	tcpmgr_stop(mgr);

RET:
	// Delete trasvc
	trasvc_delete(ts);

	// Delete tcpmgr
	tcpmgr_delete(mgr);

	// Delete lstm config
	lstm_config_delete(lstmCfg);

	return 0;
}
