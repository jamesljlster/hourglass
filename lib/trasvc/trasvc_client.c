#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tcpmgr_sock.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

#define DEFAULT_BUF_SIZE	100
#define DEFAULT_TIMEOUT		5000

#define MODEL_RECV_TMP	".recv.lstm"

int trasvc_client_model_download(trasvc_client_t client, lstm_t* lstmDstPtr)
{
	int ret = TRASVC_NO_ERROR;
	char buf[DEFAULT_BUF_SIZE] = {0};

	int fLen;
	char* fBuf = NULL;

	char* tmpPtr = NULL;
	char* savePtr = NULL;

	FILE* fWrite = NULL;

	LOG("enter");

	// Make command
	ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %s\x0A",
			TRASVC_CMD_HEAD_STR,
			TRASVC_CMD_DOWNLOAD_STR
			);
	if(ret < 0)
	{
		ret = TRASVC_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send command
	ret = send(client, buf, strlen(buf), 0);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Wait response
	trasvc_run(trasvc_str_recv(client, buf, DEFAULT_BUF_SIZE), ret, RET);

	// Get status
	ret = trasvc_cmd_parse(buf);
	if((ret & TRASVC_CMD_HEAD_FLAG) == 0)
	{
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	if((ret & TRASVC_CMD_OK_FLAG) == 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Parse model size
	tmpPtr = strtok_r(buf, " ", &savePtr);
	tmpPtr = strtok_r(NULL, " ", &savePtr);
	if(tmpPtr == NULL)
	{
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	fLen = strtol(tmpPtr, &savePtr, 10);
	if(tmpPtr == savePtr)
	{
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	// Memory allocation
	trasvc_alloc(fBuf, fLen, char, ret, RET);

	// Get model
	trasvc_run(trasvc_data_recv(client, fBuf, fLen, fLen, DEFAULT_BUF_SIZE), ret, RET);

	// Dump model
	fWrite = fopen(MODEL_RECV_TMP, "rb");
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
	LOG("exit");
	return ret;
}

int trasvc_client_model_upload(trasvc_client_t client, const char* modelPath)
{
	int ret = TRASVC_NO_ERROR;
	char buf[DEFAULT_BUF_SIZE] = {0};

	int fLen;
	char* fBuf = NULL;

	FILE* fRead = NULL;

	LOG("enter");

	// Open model
	fRead = fopen(modelPath, "rb");
	if(fRead == NULL)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Get file size
	fseek(fRead, 0, SEEK_END);
	fLen = ftell(fRead);
	fseek(fRead, 0, SEEK_SET);

	// Memory allocation
	trasvc_alloc(fBuf, fLen, char, ret, RET);

	// Read file
	ret = fread(fBuf, sizeof(char), fLen, fRead);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Make command
	ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %s %d\x0A",
			TRASVC_CMD_HEAD_STR,
			TRASVC_CMD_UPLOAD_STR,
			fLen
			);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Send command and model
	ret = send(client, buf, strlen(buf), 0);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	ret = send(client, fBuf, fLen, 0);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Wait response
	trasvc_run(trasvc_str_recv(client, buf, DEFAULT_BUF_SIZE), ret, RET);

	// Get status
	ret = trasvc_cmd_parse(buf);
	if((ret & TRASVC_CMD_HEAD_FLAG) == 0)
	{
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	if((ret & TRASVC_CMD_OK_FLAG) == 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	// Reset return value
	ret = TRASVC_NO_ERROR;

RET:
	free(fBuf);

	if(fRead != NULL)
	{
		fclose(fRead);
	}

	LOG("exit");
	return ret;
}

int trasvc_client_start(trasvc_client_t client)
{
	int ret = TRASVC_NO_ERROR;
	char buf[DEFAULT_BUF_SIZE] = {0};

	LOG("enter");

	// Set command
	ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %s\x0A",
			TRASVC_CMD_HEAD_STR,
			TRASVC_CMD_START_STR
			);
	if(ret < 0)
	{
		ret = TRASVC_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send command
	ret = send(client, buf, strlen(buf), 0);
	if(ret < 0)
	{
		ret = TRASVC_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	trasvc_run(trasvc_str_recv(client, buf, DEFAULT_BUF_SIZE), ret, RET);

	// Get status
	ret = trasvc_cmd_parse(buf);
	if((ret & TRASVC_CMD_HEAD_FLAG) == 0)
	{
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	if((ret & TRASVC_CMD_OK_FLAG) == 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	ret = TRASVC_NO_ERROR;

RET:
	LOG("exit");
	return ret;
}

int trasvc_client_stop(trasvc_client_t client)
{
	int ret = TRASVC_NO_ERROR;
	char buf[DEFAULT_BUF_SIZE] = {0};

	LOG("enter");

	// Set command
	ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %s\x0A",
			TRASVC_CMD_HEAD_STR,
			TRASVC_CMD_STOP_STR
			);
	if(ret < 0)
	{
		ret = TRASVC_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send command
	ret = send(client, buf, strlen(buf), 0);
	if(ret < 0)
	{
		ret = TRASVC_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	trasvc_run(trasvc_str_recv(client, buf, DEFAULT_BUF_SIZE), ret, RET);

	// Get status
	ret = trasvc_cmd_parse(buf);
	if((ret & TRASVC_CMD_HEAD_FLAG) == 0)
	{
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	if((ret & TRASVC_CMD_OK_FLAG) == 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}

	ret = TRASVC_NO_ERROR;

RET:
	LOG("exit");
	return ret;
}

int trasvc_client_get_status(trasvc_client_t client, int* flagPtr)
{
	int ret = TRASVC_NO_ERROR;
	int tmpStatus;
	char buf[DEFAULT_BUF_SIZE] = {0};
	char bufBak[DEFAULT_BUF_SIZE] = {0};

	char* tmpPtr = NULL;
	char* savePtr = NULL;

	LOG("enter");

	// Set command
	ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %s\x0A",
			TRASVC_CMD_HEAD_STR,
			TRASVC_CMD_STATUS_STR
			);
	if(ret < 0)
	{
		ret = TRASVC_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send command
	ret = send(client, buf, strlen(buf), 0);
	if(ret < 0)
	{
		ret = TRASVC_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	trasvc_run(trasvc_str_recv(client, buf, DEFAULT_BUF_SIZE), ret, RET);
	strcpy(bufBak, buf);
	LOG("Receive: %s", buf);

	// Get status
	ret = trasvc_cmd_parse(buf);
	if((ret & TRASVC_CMD_HEAD_FLAG) == 0)
	{
		LOG("%s doesn't have head!", buf);
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	tmpPtr = strtok_r(bufBak, " ", &savePtr);
	tmpPtr = strtok_r(NULL, " ", &savePtr);
	if(tmpPtr == NULL)
	{
		LOG("too few strings!");
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	// Convert status
	tmpStatus = strtol(tmpPtr, &savePtr, 16);
	if(tmpPtr == savePtr)
	{
		LOG("Failed to convert %s to status", tmpPtr);
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	// Assign value
	*flagPtr = tmpStatus;
	ret = TRASVC_NO_ERROR;

RET:
	LOG("exit");
	return ret;
}

int trasvc_client_get_mse(trasvc_client_t client, float* msePtr)
{
	int ret = TRASVC_NO_ERROR;
	float tmpMse;
	char buf[DEFAULT_BUF_SIZE] = {0};
	char bufBak[DEFAULT_BUF_SIZE] = {0};

	char* tmpPtr = NULL;
	char* savePtr = NULL;

	LOG("enter");

	// Set command
	ret = snprintf(buf, DEFAULT_BUF_SIZE, "%s %s\x0A",
			TRASVC_CMD_HEAD_STR,
			TRASVC_CMD_MSE_STR
			);
	if(ret < 0)
	{
		ret = TRASVC_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send command
	ret = send(client, buf, strlen(buf), 0);
	if(ret < 0)
	{
		ret = TRASVC_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	trasvc_run(trasvc_str_recv(client, buf, DEFAULT_BUF_SIZE), ret, RET);
	strcpy(bufBak, buf);
	LOG("Receive: %s", buf);

	// Get status
	ret = trasvc_cmd_parse(buf);
	if((ret & TRASVC_CMD_HEAD_FLAG) == 0)
	{
		LOG("%s doesn't have head", buf);
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	tmpPtr = strtok_r(bufBak, " ", &savePtr);
	tmpPtr = strtok_r(NULL, " ", &savePtr);
	if(tmpPtr == NULL)
	{
		LOG("too few strings!");
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	// Convert mse
	tmpMse = strtod(tmpPtr, &savePtr);
	if(tmpPtr == savePtr)
	{
		LOG("Failed to convert %s to mse!", tmpPtr);
		ret = TRASVC_INVALID_CMD;
		goto RET;
	}

	// Assign value
	*msePtr = tmpMse;
	ret = TRASVC_NO_ERROR;

RET:
	LOG("exit");
	return ret;
}

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
