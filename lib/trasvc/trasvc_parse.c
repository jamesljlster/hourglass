#include <stdlib.h>
#include <string.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

int trasvc_cmd_parse(char* buf)
{
	int status, counter;
	int bufIndex;
	int ret = TRASVC_NO_ERROR;

	LOG("enter");

	// Set initial status
	status = 0;
	counter = 0;

	// Parsing
	for(bufIndex = 0; buf[bufIndex] != '\0'; bufIndex++)
	{
		// Determine state
		if(counter == 0)
		{
			if(buf[bufIndex] == TRASVC_CMD_HEAD_STR[0])
			{
				status = TRASVC_CMD_HEAD_FLAG;
			}
			else if(buf[bufIndex] == TRASVC_CMD_APPEND_STR[0])
			{
				status = TRASVC_CMD_APPEND_FLAG;
			}
			else if(buf[bufIndex] == TRASVC_CMD_OK_STR[0])
			{
				status = TRASVC_CMD_OK_FLAG;
			}
		}
		else
		{
			switch(status)
			{
				case TRASVC_CMD_HEAD_FLAG:
					if((buf[bufIndex] == TRASVC_CMD_SEP_CHAR || buf[bufIndex] == TRASVC_CMD_END_CHAR) &&
							TRASVC_CMD_HEAD_STR[counter] == '\0')
					{
						counter = 0;
						ret |= TRASVC_CMD_HEAD_FLAG;
					}
					else if(buf[bufIndex] == TRASVC_CMD_HEAD_STR[counter])
					{
						counter++;
					}
					else
					{
						ret = TRASVC_INVALID_CMD;
						goto RET;
					}

					break;

				case TRASVC_CMD_APPEND_FLAG:
					if((buf[bufIndex] == TRASVC_CMD_SEP_CHAR || buf[bufIndex] == TRASVC_CMD_END_CHAR) &&
							TRASVC_CMD_APPEND_STR[counter] == '\0')
					{
						counter = 0;
						ret |= TRASVC_CMD_APPEND_FLAG;
					}
					else if(buf[bufIndex] == TRASVC_CMD_APPEND_STR[counter])
					{
						counter++;
					}
					else
					{
						ret = TRASVC_INVALID_CMD;
						goto RET;
					}

					break;

				case TRASVC_CMD_OK_FLAG:
					if((buf[bufIndex] == TRASVC_CMD_SEP_CHAR || buf[bufIndex] == TRASVC_CMD_END_CHAR) &&
							TRASVC_CMD_OK_STR[counter] == '\0')
					{
						counter = 0;
						ret |= TRASVC_CMD_OK_FLAG;
					}
					else if(buf[bufIndex] == TRASVC_CMD_OK_STR[counter])
					{
						counter++;
					}
					else
					{
						ret = TRASVC_INVALID_CMD;
						goto RET;
					}

					break;
			}
		}
	}

RET:
	LOG("exit");
	return ret;
}

