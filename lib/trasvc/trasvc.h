#ifndef __TRASVC_H__
#define __TRASVC_H__

#include <lstm.h>

enum TRASVC_RET_VALUE
{
	TRASVC_BUF_FULL = 3,
	TRASVC_ACTIVE = 2,
	TRASVC_INACTIVE = 1,
	TRASVC_NO_ERROR = 0,
	TRASVC_MEM_FAILED = -1,
	TRASVC_SYS_FAILED = -2,
	TRASVC_TIMEOUT = -3
};

typedef struct TRASVC* trasvc_t;

#ifdef __cplusplus
extern "C" {
#endif

int trasvc_create(trasvc_t* svcPtr, lstm_config_t lstmCfg, int dataLimit);
void trasvc_delete(trasvc_t svc);

int trasvc_start(trasvc_t svc);
void trasvc_stop(trasvc_t svc);

int trasvc_data_append(trasvc_t svc, float* data, int timeout);
int trasvc_get_status(trasvc_t svc);

#ifdef __cplusplus
}
#endif

#endif
