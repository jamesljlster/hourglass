#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

int trasvc_data_append(trasvc_t svc, float* data, int timeout)
{
	int ret = TRASVC_NO_ERROR;
	int lockStatus = 0;
	int tmpIndex;
	struct timespec timeTmp;

	LOG("enter");

	// Setup cleanup handler
	pthread_cleanup_push(trasvc_mutex_unlock, &svc->mgrData.mutex);

	// Lock mgr data mutex
	timeTmp.tv_sec = 0;
	timeTmp.tv_nsec = timeout * 1000000;
	ret = pthread_mutex_timedlock(&svc->mgrData.mutex, &timeTmp);
	/*
	if(ret == ETIMEDOUT)
	{
		LOG("pthread_mutex_timedlock() timed out!");
		ret = TRASVC_TIMEOUT;
		goto RET;
	}
	else if(ret != 0)
	{
		LOG("pthread_mutex_timedlock() failed with error: %d", ret);
		ret = TRASVC_SYS_FAILED;
		goto RET;
	}
	*/
	if(ret != 0)
	{
		LOG("pthread_mutex_timedlock() failed with error: %d", ret);
		ret = TRASVC_TIMEOUT;
		goto RET;
	}
	else
	{
		lockStatus = 1;
	}

	// Check buffer status
	tmpIndex = (svc->mgrData.dataHead + 1) % svc->mgrData.dataMemLen;
	if(tmpIndex == svc->mgrData.dataTail)
	{
		LOG("Buffer full");
		ret = TRASVC_MGRDATA_FULL;
		goto RET;
	}

	// Append data
	memcpy(svc->mgrData.data[tmpIndex], data, sizeof(float) * svc->mgrData.dataCols);

	// Set head
	svc->mgrData.dataHead = tmpIndex;

	// Update flag
	trasvc_flag_update(svc);

RET:
	pthread_cleanup_pop(lockStatus);

	LOG("exit");
	return ret;
}
