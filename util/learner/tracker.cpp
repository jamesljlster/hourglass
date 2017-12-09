#include <iostream>
#include <cstdio>
#include <cstring>

#include <modcfg.h>

#include "tracker.hpp"

using namespace std;

bool tkr_svc_connect(struct TKR* tkrPtr)
{
	int iResult;
	bool ret = true;

	// Connect wheel service
	iResult = wclt_connect(&tkrPtr->wclt, tkrPtr->wsvr.ip, tkrPtr->wsvr.port);
	if(iResult < 0)
	{
		ret = false;
		printf("wclt_connect() failed with error: %d\n", iResult);
		goto ERR;
	}
	else
	{
		tkrPtr->wcltStatus = 1;
	}

	// Connect training service
	iResult = trasvc_client_connect(&tkrPtr->ts, tkrPtr->trasvr.ip, tkrPtr->trasvr.port);
	if(iResult < 0)
	{
		ret = false;
		printf("trasvc_client_connect() failed with error: %s\n", trasvc_get_error_msg(iResult));
		goto ERR;
	}
	else
	{
		tkrPtr->tsStatus = 1;
	}

	// Clear training data
	iResult = trasvc_client_clear(tkrPtr->ts);
	if(iResult < 0)
	{
		ret = false;
		printf("trasvc_client_clear() failed with error: %s\n", trasvc_get_error_msg(iResult));
		goto ERR;
	}

	goto RET;

ERR:
	tkr_svc_disconnect(tkrPtr);

RET:

	return ret;
}

void tkr_svc_disconnect(struct TKR* tkrPtr)
{
	if(tkrPtr->wcltStatus)
	{
		wclt_disconnect(tkrPtr->wclt);
		tkrPtr->wcltStatus = 0;
	}

	if(tkrPtr->tsStatus)
	{
		trasvc_client_disconnect(tkrPtr->ts);
		tkrPtr->tsStatus = 0;
	}
}

bool tkr_init(struct TKR* tkrPtr, int argc, char* argv[])
{
	int iResult;
	bool ret = true;

	// Zero memory
	memset(tkrPtr, 0, sizeof(struct TKR));

	// Parse argument
	if(!tkr_arg_parse(tkrPtr, argc, argv))
	{
		ret = false;
		goto ERR;
	}

	// Connect service
	if(!tkr_svc_connect(tkrPtr))
	{
		ret = false;
		goto ERR;
	}

	// Load model
	iResult = lstm_import(&tkrPtr->model, arg_list[TKR_ARG_MODEL_PATH].leading[0]);
	if(iResult < 0)
	{
		printf("lstm_import() failed with error: %d\n", iResult);
		ret = false;
		goto ERR;
	}

	goto RET;

ERR:
	tkr_delete(tkrPtr);

RET:
	return ret;
}

bool tkr_arg_parse(struct TKR* tkrPtr, int argc, char* argv[])
{
	int iResult;
	bool ret = true;
	const char* tmpStr;
	MODCFG cfg = NULL;

	// Processing arguments
	iResult = args_parse(arg_list, argc, argv, NULL);
	if(iResult < 0)
	{
		ret = false;
		goto RET;
	}

	// Check argument
	if(arg_list[TKR_ARG_CFG_PATH].enable <= 0)
	{
		printf("'%s' argument not set!\n", arg_list[TKR_ARG_CFG_PATH].name);
		ret = false;
	}

	if(arg_list[TKR_ARG_MODEL_PATH].enable <= 0)
	{
		printf("'%s' argument not set!\n", arg_list[TKR_ARG_MODEL_PATH].name);
		ret = false;
	}

	if(!ret || arg_list[TKR_ARG_HELP].enable > 0)
	{
		cout << endl;
		cout << "Usage:" << endl;
		args_print_help(arg_list);
		cout << endl;

		ret = false;
		goto RET;
	}

	// Read config file
	iResult = modcfg_create(&cfg, arg_list[TKR_ARG_CFG_PATH].leading[0]);
	if(iResult < 0)
	{
		cout << "modcfg_create() failed with error: " << iResult << endl;
		ret = false;
		goto RET;
	}

	// Parse wheel server connection setting
	tmpStr = modcfg_get_content(cfg, TKS_CFG_ROOT, TKS_CFG_WSVR_IP);
	if(tmpStr == NULL)
	{
		printf("'%s' not found in '%s' setting!\n", TKS_CFG_WSVR_IP, TKS_CFG_ROOT);
		ret = false;
		goto RET;
	}
	else
	{
		tkrPtr->wsvr.ip = new char[strlen(tmpStr) + 1]();
		strcpy(tkrPtr->wsvr.ip, tmpStr);
	}

	iResult = modcfg_parse_content_int(&tkrPtr->wsvr.port, cfg, TKS_CFG_ROOT, TKS_CFG_WSVR_PORT, 10);
	if(iResult < 0)
	{
		printf("Failed to parse wheel server port!\n");
		ret = false;
		goto RET;
	}

	// Parse training server connection setting
	tmpStr = modcfg_get_content(cfg, TKS_CFG_ROOT, TKS_CFG_TRASVR_IP);
	if(tmpStr == NULL)
	{
		printf("'%s' not found in '%s' setting!\n", TKS_CFG_TRASVR_IP, TKS_CFG_ROOT);
		ret = false;
		goto RET;
	}
	else
	{
		tkrPtr->trasvr.ip = new char[strlen(tmpStr) + 1]();
		strcpy(tkrPtr->trasvr.ip, tmpStr);
	}

	iResult = modcfg_parse_content_int(&tkrPtr->trasvr.port, cfg, TKS_CFG_ROOT, TKS_CFG_TRASVR_PORT, 10);
	if(iResult < 0)
	{
		printf("Failed to parse training server port!\n");
		ret = false;
		goto RET;
	}

RET:
	modcfg_delete(cfg);
	return ret;
}

void tkr_delete(struct TKR* tkrPtr)
{
	// Disconnect
	tkr_svc_disconnect(tkrPtr);

	// Delete model
	lstm_delete(tkrPtr->model);

	// Free memory
	delete [] tkrPtr->wsvr.ip;
	delete [] tkrPtr->trasvr.ip;
}

