#include <stdlib.h>
#include <string.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

#define DEFAULT_ITER	1
#define DELTA_LIMIT		30

void* trasvc_tra_task(void* arg)
{
	int i, j, iter;
	int inputs, outputs;
	int tmpLen, tmpIndex, srcIndex;
	struct TRASVC* svc = arg;

	float mse;
	float lRate, mCoef;

	float* outBuf = NULL;
	float* errBuf = NULL;

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

		// Training
		tmpLen = svc->traData.dataHead - svc->traData.dataTail;
		if(tmpLen < 0)
		{
			tmpLen += svc->traData.dataMemLen;
		}

		for(iter = 0; iter < DEFAULT_ITER; i++)
		{
			mse = 0;
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
		}

		// Find mse
		mse /= (float)(DEFAULT_ITER * outputs);
		LOG("mse = %f", mse);
	}

RET:
	free(outBuf);
	free(errBuf);

	pthread_exit(NULL);
	return NULL;
}
