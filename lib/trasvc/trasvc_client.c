#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tcpmgr_sock.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

#define DEFAULT_BUF_SIZE	100

void trasvc_client_disconnect(trasvc_client_t client)
{
	sock_close(client);
}

int trasvc_client_datasend(trasvc_client_t client, float* data, int dataLen)
{
	int ret = TRASVC_NO_ERROR;
	char buf[DEFAULT_BUF_SIZE] = {0};

	LOG("enter");

	// Set command
	ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %s\x0A",
			TRASVC_CMD_HEAD_STR,
			TRASVC_CMD_APPEND_STR
			);
	if(ret < 0)
	{
		ret = TRASVC_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send command and data
	ret = send(client, buf, strlen(buf), 0);
	if(ret < 0)
	{
		ret = TRASVC_CONNECT_FAILED;
		goto RET;
	}

	ret = send(client, data, dataLen * sizeof(float), 0);
	if(ret < 0)
	{
		ret = TRASVC_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	trasvc_run(trasvc_str_recv(client, buf, DEFAULT_BUF_SIZE), ret, RET);

	// Check flag
	ret = trasvc_cmd_parse(buf);
	if(ret & TRASVC_CMD_OK_FLAG)
	{
		ret = TRASVC_NO_ERROR;
	}
	else if(ret & TRASVC_CMD_TIMEOUT_FLAG)
	{
		ret = TRASVC_TIMEOUT;
	}
	else
	{
		ret = TRASVC_INVALID_CMD;
	}

RET:
	LOG("exit");
	return ret;
}

int trasvc_client_connect(trasvc_client_t* clientPtr, const char* serverIP, int serverPort)
{
	int ret = TRASVC_NO_ERROR;
	sock_t sock = -1;
	struct sockaddr_in addrInfo;

	LOG("enter");

	// Create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Connect to server
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_addr.s_addr = inet_addr(serverIP);
	addrInfo.sin_port = htons(serverPort);
	ret = connect(sock, (struct sockaddr*)&addrInfo, sizeof(struct sockaddr));
	if(ret < 0)
	{
		ret = TRASVC_CONNECT_FAILED;
		goto ERR;
	}

	// Assign value
	*clientPtr = sock;

	goto RET;

ERR:
	if(sock >= 0)
	{
		sock_close(sock);
	}

RET:
	LOG("exit");
	return ret;
}
