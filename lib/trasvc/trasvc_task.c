#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <tcpmgr_sock.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

#define DEFAULT_ITER	1
#define DEFAULT_TIMEOUT	3000

#define DELTA_LIMIT		30
#define BUF_RESERVE		100

// Reset active flag
void trasvc_active_reset(void* arg)
{
	struct TRASVC* svc = arg;
	svc->status = svc->status & (~TRASVC_ACTIVE);
}

void trasvc_mem_free(void* arg)
{
	LOG("free mem with address: %p", arg);
	free(arg);
}

void trasvc_mutex_unlock(void* arg)
{
	LOG("Unlock mutex: 0x%p", arg);
	pthread_mutex_unlock(arg);
}

void trasvc_client_task(void* arg, int sock)
{
	int ret;
	int bufLen;
	char* buf = NULL;

	struct TRASVC* svc = arg;

	// Setup cleanup handler
	pthread_cleanup_push(trasvc_mem_free, buf);

	// Memory allocation
	bufLen = BUF_RESERVE + svc->mgrData.dataCols * sizeof(float);
	buf = calloc(bufLen, sizeof(char));
	if(buf == NULL)
	{
		printf("Memory allocation failed!\n");
		return;
	}

	// Loop for communication
	while(1)
	{
		// Receive command string
		ret = trasvc_str_recv(sock, buf, bufLen);
		if(ret < 0)
		{
			printf("trasvc_str_recv() failed with error: %s\n", trasvc_get_error_msg(ret));
			printf("Shutdown connection\n");
			break;
		}

		// Parse command
		ret = trasvc_cmd_parse(buf);
		if(ret < 0)
		{
			printf("trasvc_cmd_parse() failed with error: %s\n", trasvc_get_error_msg(ret));
			printf("Restart\n");
			goto RESP;
		}

		// Receive external data
		if(ret & TRASVC_CMD_APPEND_FLAG)
		{
			ret = trasvc_data_recv(sock, buf, bufLen, svc->mgrData.dataCols * sizeof(float), DEFAULT_TIMEOUT);
			if(ret < 0)
			{
				printf("trasvc_data_recv() failed with error: %s\n", trasvc_get_error_msg(ret));
				printf("Shutdown connection\n");
				break;
			}
		}

RESP:
		// Send response
		strncpy(buf, TRASVC_CMD_HEAD_STR, bufLen - 1);
		strncat(buf, " ", bufLen - strlen(buf) - 1);
		switch(ret)
		{
			case TRASVC_NO_ERROR:
				strncat(buf, TRASVC_CMD_OK_STR, bufLen - strlen(buf) - 1);
				break;

			case TRASVC_TIMEOUT:
				strncat(buf, TRASVC_CMD_TIMEOUT_STR, bufLen - strlen(buf) - 1);
				break;

			default:
				strncat(buf, TRASVC_CMD_ERR_STR, bufLen - strlen(buf) - 1);
		}

		strncat(buf, "\x0A", bufLen - strlen(buf) - 1);

		ret = send(sock, buf, strlen(buf), 0);
		if(ret < 0)
		{
			printf("send() failed with error: %d\n", ret);
			printf("Shutdown connection\n");
			break;
		}
	}

	pthread_cleanup_pop(1);
}

void* trasvc_tra_task(void* arg)
{
	int i, j, iter;
	int mgrLockStatus = 0;
	int inputs, outputs;
	int tmpLen, tmpIndex, srcIndex;
	struct TRASVC* svc = arg;

	float mse;
	float lRate, mCoef;

	float* outBuf = NULL;
	float* errBuf = NULL;

	// Thread cleanup task
	pthread_cleanup_push(trasvc_mem_free, outBuf);
	pthread_cleanup_push(trasvc_mem_free, errBuf);
	pthread_cleanup_push(trasvc_mutex_unlock, &svc->mgrData.mutex);
	pthread_cleanup_push(trasvc_active_reset, svc);

	// Set active flag
	svc->status = svc->status | TRASVC_ACTIVE;

	// Find inputs and outputs
	inputs = lstm_config_get_inputs(svc->lstmCfg);
	outputs = lstm_config_get_outputs(svc->lstmCfg);

	// Find learning rate and momentum coefficient
	lRate = lstm_config_get_learning_rate(svc->lstmCfg);
	mCoef = lstm_config_get_momentum_coef(svc->lstmCfg);

	// Memory allocation
	outBuf = calloc(inputs, sizeof(float));
	if(outBuf == NULL)
	{
		LOG("Memory allocation failed!");
		goto RET;
	}

	errBuf = calloc(outputs, sizeof(float));
	if(errBuf == NULL)
	{
		LOG("Memory allocation failed!");
		goto RET;
	}

	// Training task
	while(svc->stop == 0)
	{
		// Lock mgr data
		LOG("Wait for mgrData lock");
		pthread_mutex_lock(&svc->mgrData.mutex);
		mgrLockStatus = 1;
		LOG("mgrData locked");

		// Update training data
		tmpLen = svc->mgrData.dataHead - svc->mgrData.dataTail;
		if(tmpLen < 0)
		{
			tmpLen += svc->mgrData.dataMemLen;
		}

		if(tmpLen > 0)
		{
			for(i = 0; i < tmpLen; i++)
			{
				// Find queue index
				srcIndex = (svc->mgrData.dataTail + 1 + i) % svc->mgrData.dataMemLen;
				tmpIndex = (svc->traData.dataHead + 1) % svc->traData.dataMemLen;
				if(tmpIndex == svc->traData.dataTail)
				{
					// Set lstm state
					lstm_forward_computation(svc->lstm, svc->traData.data[tmpIndex], outBuf);
					lstm_state_save(svc->lstmState, svc->lstm);

					// Overwrite
					LOG("Overwirte training data index: %d", svc->traData.dataTail);
					svc->traData.dataTail = (svc->traData.dataTail + 1) % svc->traData.dataMemLen;
				}

#ifdef DEBUG
				printf("Push data: ");
				for(j = 0; j < svc->mgrData.dataCols; j++)
				{
					printf("%f", svc->mgrData.data[srcIndex][j]);
					if(j == svc->mgrData.dataCols - 1)
					{
						printf("\n");
					}
					else
					{
						printf(", ");
					}
				}
#endif

				// Copy memory
				memcpy(svc->traData.data[tmpIndex], svc->mgrData.data[srcIndex], sizeof(float) * svc->traData.dataCols);

				// Set queue head
				svc->traData.dataHead = tmpIndex;
			}

			// Clear mgr data queue
			svc->mgrData.dataTail = svc->mgrData.dataHead;
		}
#ifdef DEBUG
		else
		{
			LOG("Empty mgrData");
		}
#endif

		// Unlock mgr data
		mgrLockStatus = 0;
		pthread_mutex_unlock(&svc->mgrData.mutex);
		LOG("mgrData unlocked");

		// Restore lstm state
		lstm_state_restore(svc->lstmState, svc->lstm);

		// Training
		tmpLen = svc->traData.dataHead - svc->traData.dataTail;
		if(tmpLen < 0)
		{
			tmpLen += svc->traData.dataMemLen;
		}

		if(tmpLen <= 0)
		{
			LOG("Empty training data");
			continue;
		}

		for(iter = 0; iter < DEFAULT_ITER; iter++)
		{
			mse = 0;
			LOG("Training data amount: %d", tmpLen);
			for(i = 0; i < tmpLen; i++)
			{
				// Find data index
				tmpIndex = (svc->traData.dataTail + 1 + i) % svc->traData.dataMemLen;

				// Forward computation
				lstm_forward_computation(svc->lstm, svc->traData.data[tmpIndex], outBuf);

				// Sum gradient
				for(j = 0; j < outputs; j++)
				{
					errBuf[j] = svc->traData.data[tmpIndex][j + inputs] - outBuf[j];
					mse += errBuf[j] * errBuf[j];
				}

				lstm_bptt_sum_gradient(svc->lstm, errBuf);
			}

			// Update network
			lstm_bptt_adjust_network(svc->lstm, lRate, mCoef, DELTA_LIMIT);
			lstm_bptt_erase(svc->lstm);

			// Find mse
			mse /= (float)(tmpLen * outputs);
			svc->mse = mse;
			LOG("mse = %f", mse);
		}
	}

RET:
	pthread_cleanup_pop(1);	// Reset active flag
	pthread_cleanup_pop(mgrLockStatus);	// Unlock mutex
	pthread_cleanup_pop(1);	// Free errBuf
	pthread_cleanup_pop(1);	// Free outBuf

	pthread_exit(NULL);
	return NULL;
}
