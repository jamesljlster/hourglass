#ifndef __TRASVC_TYPES_H__
#define __TRASVC_TYPES_H__

#include <pthread.h>
#include <lstm.h>

struct TRASVC
{
	int stop;

	lstm_t lstm;
	lstm_state_t lstmState;
	lstm_config_t lstmCfg;

	int status;

	int thStatus;
	pthread_t th;

	double** data;
	int dataHead;
	int dataTail;
	int dataMemLen;
};

#endif
