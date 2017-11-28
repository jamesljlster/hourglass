#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <tcpmgr_sock.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

#define TRASVC_CMD_SEP_CHAR ' '
#define TRASVC_CMD_END_CHAR 0x0A

#define TRASVC_CMD_HEAD_STR "TS"
#define TRASVC_CMD_APPEND_STR "APPEND"
#define TRASVC_CMD_OK_STR "OK"

int trasvc_data_recv(int sock, char* buf, int bufLen, int recvLen, int timeout)
{
	int recvCounter;
	int ret = TRASVC_NO_ERROR;

	struct timespec timeHold, timeTmp;
	struct timeval timeLeft;
	fd_set fdSet;

	LOG("enter");

	// Checking
	if(bufLen < recvLen)
	{
		ret = TRASVC_INSUFFICIENT_BUF;
		goto RET;
	}

	// Set timeout
	timeout = timeout * 1000; // msec to usec
	timeLeft.tv_sec = 0;

	// Get current time
	clock_gettime(CLOCK_MONOTONIC, &timeHold);

	for(recvCounter = 0; recvCounter < recvLen; )
	{
		// Set timeout
		clock_gettime(CLOCK_MONOTONIC, &timeTmp);
		timeTmp.tv_sec = timeTmp.tv_sec - timeHold.tv_sec;
		timeTmp.tv_nsec = timeTmp.tv_nsec - timeHold.tv_nsec;
		timeLeft.tv_usec = timeout - (timeTmp.tv_sec * 1000000 + timeTmp.tv_nsec / 1000);

		// Select
		FD_ZERO(&fdSet);
		FD_SET(sock, &fdSet);
		ret = select(sock + 1, &fdSet, NULL, NULL, &timeLeft);
		if(ret == 0)
		{
			ret = TRASVC_TIMEOUT;
			goto RET;
		}

		// Receive
		ret = recv(sock, &buf[recvCounter], recvLen - recvCounter, 0);
		if(ret < 0)
		{
			ret = TRASVC_SYS_FAILED;
			goto RET;
		}
		else if(ret == 0)
		{
			ret = TRASVC_CONNECTION_CLOSED;
			goto RET;
		}
		else
		{
			recvCounter += ret;
		}
	}

RET:
	LOG("exit");
	return ret;
}

int trasvc_str_recv(int sock, int* tsFlag, char* buf, int bufLen, int timeout)
{
	int status, counter;
	int bufIndex;
	int ret = TRASVC_NO_ERROR;
	char tmpRead;

	struct timespec timeHold, timeTmp;
	struct timeval timeLeft;
	fd_set fdSet;

	LOG("enter");

	// Initialize buffer
	memset(buf, 0, bufLen);

	// Set initial status
	status = 0;
	counter = 0;
	bufIndex = 0;

	// Set timeout
	timeout = timeout * 1000; // msec to usec
	timeLeft.tv_sec = 0;

	// Get current time
	clock_gettime(CLOCK_MONOTONIC, &timeHold);

	while(1)
	{
		// Set timeout
		clock_gettime(CLOCK_MONOTONIC, &timeTmp);
		timeTmp.tv_sec = timeTmp.tv_sec - timeHold.tv_sec;
		timeTmp.tv_nsec = timeTmp.tv_nsec - timeHold.tv_nsec;
		timeLeft.tv_usec = timeout - (timeTmp.tv_sec * 1000000 + timeTmp.tv_nsec / 1000);

		// Select
		FD_ZERO(&fdSet);
		FD_SET(sock, &fdSet);
		ret = select(sock + 1, &fdSet, NULL, NULL, &timeLeft);
		if(ret == 0)
		{
			ret = TRASVC_TIMEOUT;
			goto RET;
		}

		// Receive
		ret = recv(sock, &tmpRead, 1, 0);
		if(ret < 0)
		{
			ret = TRASVC_SYS_FAILED;
			goto RET;
		}
		else if(ret == 0)
		{
			ret = TRASVC_CONNECTION_CLOSED;
			goto RET;
		}
		else
		{
			// Insert character to buf
			buf[bufIndex++] = tmpRead;
		}

		// Determine state
		if(counter == 0)
		{
			if(tmpRead == TRASVC_CMD_HEAD_STR[0])
			{
				status = TRASVC_CMD_HEAD_FLAG;
			}
			else if(tmpRead == TRASVC_CMD_APPEND_STR[0])
			{
				status = TRASVC_CMD_APPEND_FLAG;
			}
			else if(tmpRead == TRASVC_CMD_OK_STR[0])
			{
				status = TRASVC_CMD_OK_FLAG;
			}
		}
		else
		{
			switch(status)
			{
				case TRASVC_CMD_HEAD_FLAG:
					if((tmpRead == TRASVC_CMD_SEP_CHAR || tmpRead == TRASVC_CMD_END_CHAR) &&
							TRASVC_CMD_HEAD_STR[counter] == '\0')
					{
						counter = 0;
						ret |= TRASVC_CMD_HEAD_FLAG;
					}
					else if(tmpRead == TRASVC_CMD_HEAD_STR[counter])
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
					if((tmpRead == TRASVC_CMD_SEP_CHAR || tmpRead == TRASVC_CMD_END_CHAR) &&
							TRASVC_CMD_APPEND_STR[counter] == '\0')
					{
						counter = 0;
						ret |= TRASVC_CMD_APPEND_FLAG;
					}
					else if(tmpRead == TRASVC_CMD_APPEND_STR[counter])
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
					if((tmpRead == TRASVC_CMD_SEP_CHAR || tmpRead == TRASVC_CMD_END_CHAR) &&
							TRASVC_CMD_OK_STR[counter] == '\0')
					{
						counter = 0;
						ret |= TRASVC_CMD_OK_FLAG;
					}
					else if(tmpRead == TRASVC_CMD_OK_STR[counter])
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

		// Check if reach end
		if(tmpRead == TRASVC_CMD_END_CHAR)
		{
			break;
		}

		// Increase counter
		counter++;
	}

RET:
	LOG("exit");
	return ret;
}

