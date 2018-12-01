#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <trasvc.h>
#include <trasvc_private.h>

#define DATA_LIMIT 1000
#define DATA_AMOUNT	304

#define INPUTS 1
#define OUTPUTS 1

#define TIMEOUT 1000

#define TARGET_MSE 0.001

#define BUF_SIZE	128

#define SEND_LSTM_PATH "send.lstm"

int main(int argc, char* argv[])
{
	int i;
	int ret;
	float mse;
	int status;

	const char* serverIP;
	int serverPort;
	char buf[BUF_SIZE] = {0};
	char tmpRead;

	trasvc_client_t tsClient;

	// Checking
	if(argc < 3)
	{
		printf("test_hts_exchange <server_ip> <server_port>\n");
		return -1;
	}
	else
	{
		serverIP = argv[1];
		serverPort = atoi(argv[2]);

		printf("Using serverIP: %s, serverPort: %d\n", argv[1], serverPort);
	}

	// Connect to trasvc
	ret = trasvc_client_connect(&tsClient, serverIP, serverPort);
	if(ret < 0)
	{
		printf("trasvc_client_connect() failed with error: %s\n", trasvc_get_error_msg(ret));
		return -1;
	}

	// Get user input
	i = 0;
	while(1)
	{
		printf("hts> ");
		for(i = 0; i < BUF_SIZE; i++)
		{
			ret = scanf("%c", &tmpRead);
			if(ret <= 0)
			{
				continue;
			}

			if(tmpRead == '\n')
			{
				buf[i] = '\0';
				break;
			}
			else
			{
				buf[i] = tmpRead;
			}
		}

		// Parse command
		if(strcmp(buf, "exit") == 0)
		{
			break;
		}
		else if(strcmp(buf, "start") == 0)
		{
			ret = trasvc_client_start(tsClient);
			if(ret < 0)
			{
				printf("trasvc_client_start() failed with error: %s\n", trasvc_get_error_msg(ret));
			}
		}
		else if(strcmp(buf, "stop") == 0)
		{
			ret = trasvc_client_stop(tsClient);
			if(ret < 0)
			{
				printf("trasvc_client_stop() failed with error: %s\n", trasvc_get_error_msg(ret));
			}
		}
		else if(strcmp(buf, "status") == 0)
		{
			ret = trasvc_client_get_status(tsClient, &status);
			if(ret < 0)
			{
				printf("trasvc_client_get_status() failed with error: %s\n", trasvc_get_error_msg(ret));
			}
			else
			{
				printf("Status: 0x%x\n", status);
				printf("Active: %d\n", (status & TRASVC_ACTIVE) > 0);
				printf("TraData Empty: %d\n", (status & TRASVC_TRADATA_EMPTY) > 0);
				printf("TraData Full: %d\n", (status & TRASVC_TRADATA_FULL) > 0);
				printf("MgrData Empty: %d\n", (status & TRASVC_MGRDATA_EMPTY) > 0);
				printf("MgrData Full: %d\n", (status & TRASVC_MGRDATA_FULL) > 0);
			}
		}
		else if(strcmp(buf, "mse") == 0)
		{
			ret = trasvc_client_get_mse(tsClient, &mse);
			if(ret < 0)
			{
				printf("trasvc_client_get_mse() failed with error: %s\n", trasvc_get_error_msg(ret));
			}
			else
			{
				printf("MSE: %f\n", mse);
			}
		}
		else if(strcmp(buf, "upload") == 0)
		{
			ret = trasvc_client_model_upload(tsClient, SEND_LSTM_PATH);
			if(ret < 0)
			{
				printf("trasvc_client_model_upload() failed with error: %s\n", trasvc_get_error_msg(ret));
			}
		}
		else if(strcmp(buf, "download") == 0)
		{
			lstm_t lstm = NULL;
			ret = trasvc_client_model_download(tsClient, &lstm);
			if(ret < 0)
			{
				printf("trasvc_client_model_download() failed with error: %s\n", trasvc_get_error_msg(ret));
			}
			else
			{
				ret = lstm_export(lstm, "recv.lstm");
				if(ret < 0)
				{
					printf("lstm_export() failed with error: %d\n", ret);
				}

				lstm_delete(lstm);
			}
		}
		else if(strcmp(buf, "clear") == 0)
		{
			ret = trasvc_client_clear(tsClient);
			if(ret < 0)
			{
				printf("trasvc_client_clear() failed with error: %s\n", trasvc_get_error_msg(ret));
			}
		}

		printf("\n");
	}

	trasvc_client_disconnect(tsClient);

	return 0;
}

