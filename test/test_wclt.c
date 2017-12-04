#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <wclt.h>

int main(int argc, char* argv[])
{
	int ret;
	char* serverIP, *tmpPtr;
	int serverPort;

	wclt_t wclt;

	// Checking
	if(argc < 3)
	{
		printf("test_connection <server_ip> <server_port>\n");
		return -1;
	}
	else
	{
		serverIP = argv[1];
		serverPort = strtol(argv[2], &tmpPtr, 10);
		if(tmpPtr == argv[2])
		{
			printf("Failed to convert %s to serverPort", argv[2]);
			return -1;
		}

		printf("Using serverIP: %s, serverPort: %d\n", argv[1], serverPort);
	}

	// Connect to wheel server
	ret = wclt_connect(&wclt, serverIP, serverPort);
	if(ret < 0)
	{
		printf("wclt_connect() failed with error: %d\n", ret);
		return -1;
	}

	// Lock device
	ret = wclt_lock(wclt);
	if(ret < 0)
	{
		printf("wclt_lock() failed with error: %d\n", ret);
		return -1;
	}

	// Control test
	ret = wclt_control(wclt, 320, 320);
	if(ret < 0)
	{
		printf("wclt_control() failed with error: %d\n", ret);
		return -1;
	}

	usleep(1000 * 1000);

	ret = wclt_control(wclt, 190, 320);
	if(ret < 0)
	{
		printf("wclt_control() failed with error: %d\n", ret);
		return -1;
	}

	usleep(1000 * 1000);

	ret = wclt_control(wclt, 320, 190);
	if(ret < 0)
	{
		printf("wclt_control() failed with error: %d\n",ret);
		return -1;
	}

	usleep(1000 * 1000);

	ret = wclt_control(wclt, 190, 190);
	if(ret < 0)
	{
		printf("wclt_control() failed with error: %d\n", ret);
		return -1;
	}

	usleep(1000 * 1000);

	ret = wclt_control(wclt, 255, 255);
	if(ret < 0)
	{
		printf("wclt_control() failed with error: %d\n", ret);
		return -1;
	}

	// Unlock device
	ret = wclt_unlock(wclt);
	if(ret < 0)
	{
		printf("wclt_unlock() failed with error: %d\n", ret);
		return -1;
	}

	return 0;
}
