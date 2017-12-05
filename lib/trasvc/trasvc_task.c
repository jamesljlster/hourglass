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
	int bufLen, tmpLen;
	int defaultResp;
	int needResp;
	char* buf = NULL;
	char* bufBak = NULL;

	char* tmpPtr;
	char* savePtr;

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

	bufBak = calloc(bufLen, sizeof(char));
	if(bufBak == NULL)
	{
		printf("Memroy allocation failed!\n");
		free(buf);
		buf = NULL;
		return;
	}

	// Loop for communication
	while(1)
	{
		defaultResp = 1;
		needResp = 1;

		// Receive command string
		ret = trasvc_str_recv(sock, buf, bufLen);
		if(ret < 0)
		{
			printf("trasvc_str_recv() failed with error: %s\n", trasvc_get_error_msg(ret));
			printf("Shutdown connection\n");
			break;
		}

		// Parse command
		strcpy(bufBak, buf);
		ret = trasvc_cmd_parse(bufBak);
		if(ret < 0)
		{
			printf("trasvc_cmd_parse() failed with error: %s\n", trasvc_get_error_msg(ret));
			printf("Restart\n");
		}

		// Run command
		if(ret > 0)
		{
			if((ret & TRASVC_CMD_APPEND_FLAG) > 0)
			{
				// Receive external data
				ret = trasvc_data_recv(sock, buf, bufLen, svc->mgrData.dataCols * sizeof(float), DEFAULT_TIMEOUT);
				if(ret < 0)
				{
					printf("trasvc_data_recv() failed with error: %s\n", trasvc_get_error_msg(ret));
					printf("Shutdown connection\n");
					break;
				}

				// Append data
				ret = trasvc_data_append(svc, (float*)buf, DEFAULT_TIMEOUT);
				if(ret < 0)
				{
					printf("trasvc_data_append() failed with error: %s\n", trasvc_get_error_msg(ret));
				}
			}
			else if((ret & TRASVC_CMD_START_FLAG) > 0)
			{
				// Start training task
				ret = trasvc_start(svc);
				if(ret < 0)
				{
					printf("trasvc_start(ts) failed with error: %s\n", trasvc_get_error_msg(ret));
				}
			}
			else if((ret & TRASVC_CMD_STOP_FLAG) > 0)
			{
				// Stop training task
				trasvc_stop(svc);
				ret = TRASVC_NO_ERROR;
			}
			else if((ret & TRASVC_CMD_MSE_FLAG) > 0)
			{
				// Make response string
				snprintf(buf, bufLen, "%s %f\x0A",
						TRASVC_CMD_HEAD_STR,
						svc->mse
						);

				// Using custom response string
				defaultResp = 0;
			}
			else if((ret & TRASVC_CMD_STATUS_FLAG) > 0)
			{
				// Make response string
				snprintf(buf, bufLen, "%s %x\x0A",
						TRASVC_CMD_HEAD_STR,
						svc->status
						);

				// Using custom response string
				defaultResp = 0;
			}
			else if((ret & TRASVC_CMD_UPLOAD_FLAG) > 0)
			{
				// Parse model size
				savePtr = NULL;
				tmpPtr = strtok_r(buf, " ", &savePtr);
				tmpPtr = strtok_r(NULL, " ", &savePtr);
				tmpPtr = strtok_r(NULL, " ", &savePtr);
				if(tmpPtr == NULL)
				{
					LOG("Missing model size!");
					ret = TRASVC_INVALID_CMD;
				}
				else
				{
					tmpLen = strtol(tmpPtr, &savePtr, 10);
					if(tmpPtr == savePtr)
					{
						LOG("Failed to convert %s to model size!", tmpPtr);
						ret = TRASVC_INVALID_CMD;
					}
					else
					{
						// Lock lstm receive buffer
						pthread_mutex_lock(&svc->lstmRecvBuf.mutex);

						// Receive model
						ret = trasvc_model_recv(sock, &svc->lstmRecvBuf.lstm, tmpLen);
						if(ret < 0)
						{
							printf("trasvc_model_recv() failed with error: %s\n", trasvc_get_error_msg(ret));
						}
						else
						{
							svc->lstmRecvBuf.status = 1;
						}

						// Unlock lstm receive buffer
						pthread_mutex_unlock(&svc->lstmRecvBuf.mutex);
					}
				}
			}
			else if((ret & TRASVC_CMD_DOWNLOAD_FLAG) > 0)
			{
				// Lock lstm send buffer
				pthread_mutex_lock(&svc->lstmSendBuf.mutex);

				// Send model
				ret = trasvc_model_send(sock, svc->lstmSendBuf.lstm);
				if(ret < 0)
				{
					printf("trasvc_model_send() failed with error: %s\n", trasvc_get_error_msg(ret));
				}

				// Unlock lstm send buffer
				pthread_mutex_unlock(&svc->lstmSendBuf.mutex);

				if(ret == TRASVC_NO_ERROR)
				{
					// Already send response
					needResp = 0;
				}
			}
		}

		// Make default response string
		if(needResp > 0)
		{
			if(defaultResp > 0)
			{
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
			}

			// Send response
			ret = send(sock, buf, strlen(buf), 0);
			if(ret < 0)
			{
				printf("send() failed with error: %d\n", ret);
				printf("Shutdown connection\n");
				break;
			}
		}
	}

	pthread_cleanup_pop(1);
}

void* trasvc_tra_task(void* arg)
{
	int ret;
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
		// Update flag
		trasvc_flag_update(svc);

		// Update model if available
		if(svc->lstmRecvBuf.status > 0)
		{
			// Lock lstm receive buffer
			pthread_mutex_lock(&svc->lstmRecvBuf.mutex);

			// Replace model
			lstm_delete(svc->lstm);

			ret = lstm_clone(&svc->lstm, svc->lstmRecvBuf.lstm);
			if(ret < 0)
			{
				printf("lstm_clone() failed with error: %d\n", ret);
				break;
			}
			else
			{
				lstm_delete(svc->lstmRecvBuf.lstm);
				svc->lstmRecvBuf.lstm = NULL;
				svc->lstmRecvBuf.status = 0;
			}

			// Unlock lstm receive buffer
			pthread_mutex_unlock(&svc->lstmRecvBuf.mutex);
		}

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

		// Lock lstm send buffer
		pthread_mutex_lock(&svc->lstmSendBuf.mutex);

		// Clean and copy model
		lstm_delete(svc->lstmSendBuf.lstm);
		lstm_clone(&svc->lstmSendBuf.lstm, svc->lstm);

		// Unlock lstm send buffer
		pthread_mutex_unlock(&svc->lstmSendBuf.mutex);
	}

RET:
	svc->traTaskStatus = 0;

	pthread_cleanup_pop(1);	// Reset active flag
	pthread_cleanup_pop(mgrLockStatus);	// Unlock mutex
	pthread_cleanup_pop(1);	// Free errBuf
	pthread_cleanup_pop(1);	// Free outBuf

	pthread_exit(NULL);
	return NULL;
}
