#include <stdlib.h>
#include <string.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

struct TRASVC_FLAG trasvc_flag_list[] = {
	{TRASVC_CMD_HEAD_FLAG, TRASVC_CMD_HEAD_STR},
	{TRASVC_CMD_APPEND_FLAG, TRASVC_CMD_APPEND_STR},
	{TRASVC_CMD_OK_FLAG, TRASVC_CMD_OK_STR},
	{TRASVC_CMD_ERR_FLAG, TRASVC_CMD_ERR_STR},
	{TRASVC_CMD_TIMEOUT_FLAG, TRASVC_CMD_TIMEOUT_STR},
	{TRASVC_CMD_START_FLAG, TRASVC_CMD_START_STR},
	{TRASVC_CMD_STOP_FLAG, TRASVC_CMD_STOP_STR},
	{TRASVC_CMD_MSE_FLAG, TRASVC_CMD_MSE_STR},
	{TRASVC_CMD_STATUS_FLAG, TRASVC_CMD_STATUS_STR},
	{TRASVC_CMD_UPLOAD_FLAG, TRASVC_CMD_UPLOAD_STR},
	{TRASVC_CMD_DOWNLOAD_FLAG, TRASVC_CMD_DOWNLOAD_STR},
	{TRASVC_CMD_UNAVAILABLE_FLAG, TRASVC_CMD_UNAVAILABLE_STR},
	{0, NULL}
};

int trasvc_cmd_parse(char* buf)
{
	int i;
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

		for(i = 0; trasvc_flag_list[i].flag > 0; i++)
		{
			if(strcmp(tmpPtr, trasvc_flag_list[i].str) == 0)
			{
				LOG("Have %s", trasvc_flag_list[i].str);
				ret |= trasvc_flag_list[i].flag;
				LOG("ret = %x", ret);
			}
		}
	}

RET:
	LOG("exit");
	return ret;
}

