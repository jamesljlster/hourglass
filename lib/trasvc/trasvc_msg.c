#include <stdlib.h>

#include "trasvc.h"

#include "debug.h"

const char* trasvc_errmsg[] = {
	"No error",
	"Memory allocation failed",
	"System function failed",
	"Timeout",
	"Connection closed",
	"Connect failed",
	"Invalid command",
	"Insufficient buffer"
};

const char* trasvc_get_error_msg(int retValue)
{
	const char* ret = NULL;

	LOG("enter");

	if(retValue >= 0)
	{
		ret = trasvc_errmsg[0];
	}
	else if(retValue < TRASVC_INSUFFICIENT_BUF)
	{
		ret = NULL;
	}
	else
	{
		ret = trasvc_errmsg[-retValue];
	}

	LOG("exit");
	return ret;
}
