#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <tcpmgr_sock.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

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

int trasvc_str_recv(int sock, char* buf, int bufLen, int timeout)
{
	int bufIndex = 0;
	int ret = TRASVC_NO_ERROR;
	char tmpRead;

	struct timespec timeHold, timeTmp;
	struct timeval timeLeft;
	fd_set fdSet;

	LOG("enter");

	// Initialize buffer
	memset(buf, 0, bufLen);

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

			if(tmpRead == TRASVC_CMD_END_CHAR)
			{
				goto RET;
			}
		}
	}

RET:
	LOG("exit");
	return ret;
}

