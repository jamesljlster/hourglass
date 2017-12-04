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
