#include <stdio.h>
#include <stdlib.h>

#include <tcpmgr.h>
#include <trasvc.h>
#include <args.h>

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
	{1, "host-port", 'P', 1, ts_port_def, "Host port"},
	{1, "max-client", 'M', 1, ts_max_client_def, "Maximum client connection"},
	{1, "timeout", 'T', 1, ts_dev_timeout_def, "Operation timeout"},
	{0, "help", 'H', 0, NULL, "User Interface", "Print detail of arguments"},
	ARGS_TERMINATE
};

int main(int argc, char* argv[])
{
	int ret;

	char* hostIP;
	int hostPort, maxClient;

	tcpmgr_t mgr = NULL;
	trasvc_t ts = NULL;

	return 0;
}
