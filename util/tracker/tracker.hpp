#ifndef __TRACKER_HPP__
#define __TRACKER_HPP__

#include <ftsvc.hpp>
#include <wclt.h>
#include <modcfg.h>
#include <args.h>
#include <trasvc.h>

// Trakcer argument enumeration
enum TKR_ARG
{
	TKR_ARG_CFG_PATH
};

struct TKR_NET
{
	const char* ip;
	int port;
};

struct TKR
{
	struct TKR_NET wsvr;
	struct TKR_NET trasvr;

	wclt_t wclt;
	trasvc_client_t ts;
};

extern args_t arg_list[];

#endif
