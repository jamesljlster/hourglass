#include <stdlib.h>
#include <string.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

int trasvc_cmd_parse(char* buf)
{
	int ret = TRASVC_NO_ERROR;

	char* tmpPtr = NULL;
	char* savePtr = NULL;

	LOG("enter");

	// Parsing
	tmpPtr = strtok_r(buf, " ", &savePtr);
	LOG("tmpPtr: %s", tmpPtr);
	if(strcmp(tmpPtr, TRASVC_CMD_HEAD_STR) != 0)
	{
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}
	else
	{
		// Set head flag
		ret |= TRASVC_CMD_HEAD_FLAG;
		LOG("ret = %x", ret);
	}

	while(1)
	{
		tmpPtr = strtok_r(NULL, " ", &savePtr);
		if(tmpPtr == NULL)
		{
			break;
		}

		LOG("tmpPtr: %s", tmpPtr);

		if(strcmp(tmpPtr, TRASVC_CMD_APPEND_STR) == 0)
		{
			LOG("Have APPEND");
			ret |= TRASVC_CMD_APPEND_FLAG;
			LOG("ret = %x", ret);
		}
		else if(strcmp(tmpPtr, TRASVC_CMD_OK_STR) == 0)
		{
			LOG("Have OK");
			ret |= TRASVC_CMD_OK_FLAG;
			LOG("ret = %x", ret);
		}
		else
		{
			LOG("%s not a reserved command.", tmpPtr);
			ret = TRASVC_INVALID_CMD;
		}
	}

RET:
	LOG("exit");
	return ret;
}

