#include <iostream>
#include <cstring>

#include "tracker.hpp"

using namespace std;

bool tkr_init(struct TKR* tkrPtr, int argc, char* argv[])
{
	int iResult;
	bool ret;
	const char* tmpStr;

	// Zero memory
	memset(tkrPtr, 0, sizeof(struct TKR));

	// Processing arguments
	ret = args_parse(arg_list, argc, argv, NULL);
	if(ret < 0)
	{
		ret = false;
		goto RET;
	}

	// Check argument
	if(arg_list[TKR_ARG_CFG_PATH].enable <= 0 || arg_list[TKR_ARG_HELP].enable > 0)
	{
		cout << endl;
		cout << "Usage:" << endl;
		args_print_help(arg_list);

		ret = false;
		goto RET;
	}

	// Read config file
	MODCFG cfg;
	iResult = modcfg_create(&cfg, arg_list[TKR_ARG_CFG_PATH].leading[0]);
	if(iResult < 0)
	{
		cout << "modcfg_create() failed with error: " << ret << endl;
		ret = false;
		goto RET;
	}

	// Parse wheel server connection setting
	tmpStr = modcfg_get_content(cfg, TKS_CFG_ROOT, TKS_CFG_WSVR_IP);
	if(tmpStr == NULL)
	{
		printf("'%s' not found in '%s' setting!\n", TKS_CFG_WSVR_IP, TKS_CFG_ROOT);
		ret = false;
		goto ERR;
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
		goto ERR;
	}

	// Parse training server connection setting
	tmpStr = modcfg_get_content(cfg, TKS_CFG_ROOT, TKS_CFG_TRASVR_IP);
	if(tmpStr == NULL)
	{
		printf("'%s' not found in '%s' setting!\n", TKS_CFG_TRASVR_IP, TKS_CFG_ROOT);
		ret = false;
		goto ERR;
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
		goto ERR;
	}

	goto RET;

ERR:
	tkr_delete(tkrPtr);

RET:
	modcfg_delete(cfg);
	return true;
}

void tkr_delete(struct TKR* tkrPtr)
{
	// Free memory
	delete [] tkrPtr->wsvr.ip;
	delete [] tkrPtr->trasvr.ip;
}

