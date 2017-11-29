#include <stdlib.h>
#include <stdio.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

int trasvc_data_struct_init(struct TRASVC_DATA* dataPtr, int rowLimit, int cols)
{
	int i;
	int rows;
	int ret = TRASVC_NO_ERROR;

	LOG("enter");

	// Set rows
	rows = rowLimit + 1;

	// Memory allocation
	trasvc_alloc(dataPtr->data, rows, float*, ret, ERR);
	for(i = 0; i < rows; i++)
	{
		trasvc_alloc(dataPtr->data[i], cols, float, ret, ERR);
	}

	// Create mutex and condition variable
	ret = pthread_mutex_init(&dataPtr->mutex, NULL);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto ERR;
	}
	else
	{
		dataPtr->mutexStatus = 1;
	}

	ret = pthread_cond_init(&dataPtr->cond, NULL);
	if(ret < 0)
	{
		ret = TRASVC_SYS_FAILED;
		goto ERR;
	}
	else
	{
		dataPtr->condStatus = 1;
	}

	// Assign values
	dataPtr->dataMemLen = rows;
	dataPtr->dataCols = cols;

	goto RET;

ERR:
	trasvc_data_struct_cleanup(dataPtr);

RET:
	LOG("exit");
	return ret;
}

void trasvc_data_struct_cleanup(struct TRASVC_DATA* dataPtr)
{
	int i;

	LOG("enter");

	if(dataPtr->data != NULL)
	{
		for(i = 0; i < dataPtr->dataMemLen; i++)
		{
			trasvc_free(dataPtr->data[i]);
		}
		trasvc_free(dataPtr->data);
	}

	if(dataPtr->mutexStatus > 0)
	{
		pthread_mutex_destroy(&dataPtr->mutex);
	}

	if(dataPtr->condStatus > 0)
	{
		pthread_cond_destroy(&dataPtr->cond);
	}

	LOG("exit");
}

int trasvc_create(trasvc_t* svcPtr, lstm_config_t lstmCfg, int dataLimit)
{
	int cols;
	int ret = TRASVC_NO_ERROR;
	struct TRASVC* tmpSvcPtr = NULL;

	LOG("enter");

	// Memory allocation
	trasvc_alloc(tmpSvcPtr, 1, struct TRASVC, ret, RET);

	// Clone lstm config
	trasvc_run(lstm_config_clone(&tmpSvcPtr->lstmCfg, lstmCfg), ret, ERR);

	// Create lstm state
	trasvc_run(lstm_state_create(&tmpSvcPtr->lstmState, lstmCfg), ret, ERR);

	// Create lstm
	trasvc_run(lstm_create(&tmpSvcPtr->lstm, lstmCfg), ret, ERR);
	lstm_rand_network(tmpSvcPtr->lstm);

	// Create data pool
	cols = lstm_config_get_inputs(lstmCfg) + lstm_config_get_outputs(lstmCfg);
	trasvc_run(trasvc_data_struct_init(&tmpSvcPtr->traData, dataLimit, cols), ret, ERR);
	trasvc_run(trasvc_data_struct_init(&tmpSvcPtr->mgrData, dataLimit, cols), ret, ERR);

	// Assign value
	*svcPtr = tmpSvcPtr;

	goto RET;

ERR:
	trasvc_delete(tmpSvcPtr);

RET:
	LOG("exit");
	return ret;
}

void trasvc_delete(trasvc_t svc)
{
	LOG("enter");

	if(svc != NULL)
	{
		// Stop training task
		trasvc_stop(svc);

		// Cleanup
		if(svc->traTaskStatus > 0)
		{
			pthread_cancel(svc->traTask);
			pthread_join(svc->traTask, NULL);
		}

		trasvc_data_struct_cleanup(&svc->traData);
		trasvc_data_struct_cleanup(&svc->mgrData);

		lstm_delete(svc->lstm);
		lstm_state_delete(svc->lstmState);
		lstm_config_delete(svc->lstmCfg);

		trasvc_free(svc);
	}

	LOG("exit");
}

int trasvc_start(trasvc_t svc)
{
	int ret = TRASVC_NO_ERROR;

	LOG("enter");

	// Set stop state
	svc->stop = 0;

	ret = pthread_create(&svc->traTask, NULL, trasvc_tra_task, (void*)svc);
	if(ret != 0)
	{
		LOG("Failed to create training task");
		ret = TRASVC_SYS_FAILED;
	}
	else
	{
		svc->traTaskStatus = 1;
	}

	LOG("exit");
	return ret;
}

void trasvc_stop(trasvc_t svc)
{
	LOG("enter");

	if(svc->traTaskStatus > 0)
	{
		// Set stop state
		svc->stop = 1;
		pthread_join(svc->traTask, NULL);
	}

	LOG("exit");
}

