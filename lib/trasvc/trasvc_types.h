#ifndef __TRASVC_TYPES_H__
#define __TRASVC_TYPES_H__

#include <pthread.h>
#include <lstm.h>

struct TRASVC_DATA
{
	int mutexStatus;
	pthread_mutex_t mutex;

	int condStatus;
	pthread_cond_t cond;

	float** data;
	int dataHead;
	int dataTail;
	int dataCols;
	int dataMemLen;
};

struct TRASVC
{
	int status;
	int stop;

	lstm_t lstm;
	lstm_state_t lstmState;
	lstm_config_t lstmCfg;

	float mse;

	int traTaskStatus;
	pthread_t traTask;	// LSTM training task
	struct TRASVC_DATA traData;

	struct TRASVC_DATA mgrData;
};

#endif
