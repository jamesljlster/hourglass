#ifndef __TRACKER_HPP__
#define __TRACKER_HPP__

#include <wclt.h>
#include <args.h>
#include <trasvc.h>

#define TKS_CFG_ROOT "tracker"
#define TKS_CFG_WSVR_IP "wheel_server_ip"
#define TKS_CFG_WSVR_PORT "wheel_server_port"
#define TKS_CFG_TRASVR_IP "training_server_ip"
#define TKS_CFG_TRASVR_PORT "training_server_port"

// Trakcer argument enumeration
enum TKR_ARG
{
	TKR_ARG_CFG_PATH,
	TKR_ARG_HELP
};

struct TKR_NET
{
	char* ip;
	int port;
};

struct TKR
{
	int stop;

	struct TKR_NET wsvr;
	struct TKR_NET trasvr;

	int wcltStatus;
	wclt_t wclt;

	int tsStatus;
	trasvc_client_t ts;
};

extern args_t arg_list[];

bool tkr_init(struct TKR* tkrPtr, int argc, char* argv[]);
void tkr_delete(struct TKR* tkrPtr);

bool tkr_arg_parse(struct TKR* tkrPtr, int argc, char* argv[]); // No self cleanup
bool tkr_svc_connect(struct TKR* tkrPtr);
void tkr_svc_disconnect(struct TKR* tkrPtr);

#endif
