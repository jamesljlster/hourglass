#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <tcpmgr_sock.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

#define MODEL_SEND_TMP	".send.lstm"
#define MODEL_RECV_TMP	".recv.lstm"

#define DEFAULT_BUF_SIZE	100
#define DATA_RECV_TIMEOUT	5000

int trasvc_model_recv(int sock, lstm_t* lstmDstPtr, int fLen)
{
	int ret = TRASVC_NO_ERROR;

	FILE* fWrite = NULL;
	char* fBuf = NULL;

	LOG("enter");

	// Memory allocation
	trasvc_alloc(fBuf, fLen, char, ret, RET);

	// Receive model
	trasvc_run(trasvc_data_recv(sock, fBuf, fLen, fLen, DATA_RECV_TIMEOUT), ret, RET);

	// Write temp file
	fWrite = fopen(MODEL_RECV_TMP, "wb");
	if(fWrite == NULL)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	ret = fwrite(fBuf, sizeof(char), fLen, fWrite);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	fclose(fWrite);

	// Load model
	ret = lstm_import(lstmDstPtr, MODEL_RECV_TMP);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Reset return value
	ret = TRASVC_NO_ERROR;

RET:
	free(fBuf);

	LOG("exit");
	return ret;
}

int trasvc_model_send(int sock, lstm_t lstmSrc)
{
	int ret = TRASVC_NO_ERROR;
	int fLen;
	char* fBuf = NULL;
	char buf[DEFAULT_BUF_SIZE] = {0};

	FILE* fRead = NULL;

	LOG("enter");

	if(lstmSrc == NULL)
	{
		// Make response
		ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %s\x0A",
				TRASVC_CMD_HEAD_STR,
				TRASVC_CMD_UNAVAILABLE_STR
				);
		if(ret < 0)
		{
			ret = TRASVC_INSUFFICIENT_BUF;
			goto RET;
		}

		// Send response
		ret = send(sock, buf, strlen(buf), 0);
		if(ret < 0)
		{
			ret = TRASVC_CONNECT_FAILED;
			goto RET;
		}
	}
	else
	{
		// Export lstm
		ret = lstm_export(lstmSrc, MODEL_SEND_TMP);
		if(ret < 0)
		{
			ret = TRASVC_SYS_FAILED;
			goto RET;
		}

		// Open model
		fRead = fopen(MODEL_SEND_TMP, "rb");
		if(fRead == NULL)
		{
			ret = TRASVC_SYS_FAILED;
			goto RET;
		}

		// Get file size
		fseek(fRead, 0, SEEK_END);
		fLen = ftell(fRead);

		// Allocate file buffer
		trasvc_alloc(fBuf, fLen, char, ret, RET);

		// Read file to end
		ret = fread(fBuf, sizeof(char), fLen, fRead);
		if(ret != fLen)
		{
			ret = TRASVC_SYS_FAILED;
			goto RET;
		}

		// Make response
		ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %d\x0A", TRASVC_CMD_HEAD_STR, fLen);
		if(ret < 0)
		{
			ret = TRASVC_INSUFFICIENT_BUF;
			goto RET;
		}

		// Send response
		ret = send(sock, buf, strlen(buf), 0);
		if(ret < 0)
		{
			ret = TRASVC_CONNECT_FAILED;
			goto RET;
		}

		// Send file buffer
		ret = send(sock, fBuf, fLen, 0);
		if(ret < 0)
		{
			ret = TRASVC_CONNECT_FAILED;
			goto RET;
		}
	}

	// Reset return value
	ret = TRASVC_NO_ERROR;

RET:
	if(fRead != NULL)
	{
		fclose(fRead);
	}

	free(fBuf);

	LOG("exit");
	return ret;
}

int trasvc_data_recv(int sock, char* buf, int bufLen, int recvLen, int timeout)
{
	int recvCounter;
	int ret;

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

	// Reset returun value
	ret = TRASVC_NO_ERROR;

RET:
	LOG("exit");
	return ret;
}

int trasvc_str_recv(int sock, char* buf, int bufLen)
{
	int bufIndex = 0;
	int ret = TRASVC_NO_ERROR;
	char tmpRead;

	LOG("enter");

	// Initialize buffer
	memset(buf, 0, bufLen);

	while(1)
	{
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
			if(bufIndex == 0 && tmpRead != TRASVC_CMD_HEAD_STR[0])
			{
				continue;
			}

			if(tmpRead == TRASVC_CMD_END_CHAR)
			{
				ret = TRASVC_NO_ERROR;
				goto RET;
			}

			// Insert character to buf
			buf[bufIndex++] = tmpRead;
		}
	}

RET:
	LOG("exit");
	return ret;
}

