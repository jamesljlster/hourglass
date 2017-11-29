#include <stdio.h>
#include <stdlib.h>

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

args_t ts_arg_list[] = {
	{1, "host-ip", 'I', 1, ts_ip_def, "Server Setting", "Host IP dddress"},
	{1, "host-port", 'P', 1, ts_port_def, NULL, "Host port"},
	{1, "max-client", 'M', 1, ts_max_client_def, NULL, "Maximum client connection"},
	{1, "timeout", 'T', 1, ts_dev_timeout_def, NULL, "Operation timeout"},
	{0, "help", 'H', 0, NULL, "User Interface", "Print detail of arguments"},
	ARGS_TERMINATE
};

int main(int argc, char* argv[])
{
	int ret;

	char* hostIP;
	int hostPort, maxClient;
	int timeout;

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
	__strtol(hostPort, ts_arg_list[TS_ARG_HOST_PORT].leading[0],
			"Failed to convert '%s' to host port!\n",
			ts_arg_list[TS_ARG_HOST_PORT].leading[0]
			);

	__strtol(maxClient, ts_arg_list[TS_ARG_MAX_CLIENT].leading[0],
			"Failed to convert '%s' to maximum client limitation!\n",
			ts_arg_list[TS_ARG_MAX_CLIENT].leading[0]
			);

	__strtol(timeout, ts_arg_list[TS_ARG_TIMEOUT].leading[0],
			"Failed to convert '%s' to timeout!\n",
			ts_arg_list[TS_ARG_TIMEOUT].leading[0]
			);

	// Print summary
	printf("Summary:\n");
	args_print_summary(ts_arg_list);

RET:
	return 0;
}
