#include <stdio.h>
#include <string.h>

// Socket type and close method
#if defined(_WIN32)

#include <winsock2.h>

typedef SOCKET sock_t;
#define sock_close(fd)	closesocket(fd)

#elif defined(__unix__)

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define sock_close(fd)	close(fd)
typedef int sock_t;

#else
#error Unexpected building system
#endif

#include "wclt.h"
#include "wclt_private.h"

#include "debug.h"

#define STR_FLAG_HEAD	0x01
#define STR_FLAG_END	0x02

#define WCLT_BUF_SIZE 100

#define WCLT_MAX_SPEED 510
#define WCLT_MIN_SPEED 0

int wclt_control(wclt_t wclt, int leftSpeed, int rightSpeed)
{
	int ret = WCLT_NO_ERROR;
	char buf[WCLT_BUF_SIZE] = {0};

	// Checking
	if(leftSpeed < WCLT_MIN_SPEED || leftSpeed > WCLT_MAX_SPEED ||
			rightSpeed < WCLT_MIN_SPEED || rightSpeed > WCLT_MAX_SPEED)
	{
		ret = WCLT_INVALID_ARG;
		goto RET;
	}

	// Make control string
	ret = snprintf(buf, WCLT_BUF_SIZE, "%c%03d%03d%c",
			WCLT_HEAD_CHAR,
			leftSpeed, rightSpeed,
			WCLT_END_CHAR
			);
	if(ret < 0)
	{
		ret = WCLT_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send control string
	ret = send(wclt, buf, strlen(buf), 0);
	if(ret <= 0)
	{
		ret = WCLT_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	ret = wclt_str_recv(wclt, buf, WCLT_BUF_SIZE);
	if(ret < 0)
	{
		goto RET;
	}

	// Check response
	if(strcmp(buf, WCLT_OK_STR) != 0)
	{
		ret = WCLT_CONTROL_FAILED;
	}

RET:
	return ret;
}

int wclt_get_speed(wclt_t wclt, int* leftSpeedPtr, int* rightSpeedPtr)
{
	int tmpSal, tmpSar;
	int ret;
	char buf[WCLT_BUF_SIZE] = {0};

	// Make control string
	ret = snprintf(buf, WCLT_BUF_SIZE, "%s%c", WCLT_GET_STR, WCLT_END_CHAR);
	if(ret < 0)
	{
		ret = WCLT_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send control string
	ret = send(wclt, buf, strlen(buf), 0);
	if(ret <= 0)
	{
		ret = WCLT_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	ret = wclt_str_recv(wclt, buf, WCLT_BUF_SIZE);
	if(ret < 0)
	{
		goto RET;
	}

	// Parse response
	tmpSal = (buf[1] - '0') * 100 + (buf[2] - '0') * 10 + (buf[3] - '0');
	tmpSar = (buf[4] - '0') * 100 + (buf[5] - '0') * 10 + (buf[6] - '0');

	// Assign value
	*leftSpeedPtr = tmpSal;
	*rightSpeedPtr = tmpSar;
	ret = WCLT_NO_ERROR;

RET:
	return ret;
}

int wclt_lock(wclt_t wclt)
{
	int ret = WCLT_NO_ERROR;
	char buf[WCLT_BUF_SIZE] = {0};

	// Make control string
	ret = snprintf(buf, WCLT_BUF_SIZE, "%s%c", WCLT_LOCK_STR, WCLT_END_CHAR);
	if(ret < 0)
	{
		ret = WCLT_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send control string
	ret = send(wclt, buf, strlen(buf), 0);
	if(ret <= 0)
	{
		ret = WCLT_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	ret = wclt_str_recv(wclt, buf, WCLT_BUF_SIZE);
	if(ret < 0)
	{
		goto RET;
	}

	// Check response
	if(strcmp(buf, WCLT_OK_STR) != 0)
	{
		ret = WCLT_CONTROL_FAILED;
	}

RET:
	return ret;
}

int wclt_unlock(wclt_t wclt)
{
	int ret = WCLT_NO_ERROR;
	char buf[WCLT_BUF_SIZE] = {0};

	// Make control string
	ret = snprintf(buf, WCLT_BUF_SIZE, "%s%c", WCLT_UNLOCK_STR, WCLT_END_CHAR);
	if(ret < 0)
	{
		ret = WCLT_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send control string
	ret = send(wclt, buf, strlen(buf), 0);
	if(ret <= 0)
	{
		ret = WCLT_CONNECT_FAILED;
		goto RET;
	}

	// Wait response
	ret = wclt_str_recv(wclt, buf, WCLT_BUF_SIZE);
	if(ret < 0)
	{
		goto RET;
	}

	// Check response
	if(strcmp(buf, WCLT_OK_STR) != 0)
	{
		ret = WCLT_CONTROL_FAILED;
	}

RET:
	return ret;
}

int wclt_str_recv(int sock, char* buf, int bufLen)
{
	int ret = 0;
	char tmpRead;
	int strFlag = 0;
	int procIndex = 0;

	LOG("enter");

	// Initialize buffer
	memset(buf, 0, sizeof(char) * bufLen);

	while(1)
	{
		// Receive a single character
		ret = recv(sock, &tmpRead, 1, 0);
		if(ret <= 0)
		{
			ret = WCLT_CONNECT_FAILED;
			break;
		}

		// If reach string head
		if(tmpRead == WCLT_HEAD_CHAR)
		{
			// Reset string pointer
			procIndex = 0;

			// Reset flag
			strFlag = STR_FLAG_HEAD;
		}
		else if(tmpRead == WCLT_END_CHAR)
		{
			strFlag |= STR_FLAG_END;
		}

		// Check flag
		if(strFlag == (STR_FLAG_HEAD | STR_FLAG_END))
		{
			break;
		}

		// Insert character
		if(procIndex < bufLen - 1)
		{
			buf[procIndex++] = tmpRead;
		}
		else
		{
			ret = WCLT_INSUFFICIENT_BUF;
			break;
		}
	}

	LOG("exit");
	return ret;
}

int wclt_connect(wclt_t* wcltPtr, const char* serverIP, int serverPort)
{
	int tmp;
	int ret = WCLT_NO_ERROR;
	sock_t sock = -1;
	struct sockaddr_in addrInfo;

	LOG("enter");

	// Create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0)
	{
		ret = WCLT_SYS_FAILED;
		goto RET;
	}

	// Connect to server
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_addr.s_addr = inet_addr(serverIP);
	addrInfo.sin_port = htons(serverPort);
	ret = connect(sock, (struct sockaddr*)&addrInfo, sizeof(struct sockaddr));
	if(ret < 0)
	{
		ret = WCLT_CONNECT_FAILED;
		goto ERR;
	}

	// Test connection
	ret = wclt_get_speed(sock, &tmp, &tmp);
	if(ret < 0)
	{
		goto ERR;
	}

	// Assign value
	*wcltPtr = sock;

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

void wclt_disconnect(wclt_t wclt)
{
	sock_close(wclt);
}
