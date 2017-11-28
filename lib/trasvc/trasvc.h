#ifndef __TRASVC_H__
#define __TRASVC_H__

#include <lstm.h>

// TRASVC status flag
#define TRASVC_ACTIVE			0x0001
#define TRASVC_TRADATA_EMPTY	0x0002
#define TRASVC_TRADATA_FULL		0x0004
#define TRASVC_MGRDATA_EMPTY	0x0008
#define TRASVC_MGRDATA_FULL		0x0010
#define TRASVC_FATAL_ERROR		0x0020
#define TRASVC_WARNING			0x0040

enum TRASVC_RET_VALUE
{
	TRASVC_BUF_FULL = 3,
	//TRASVC_ACTIVE = 2,
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

void trasvc_client_task(void* arg, int sock);

#ifdef __cplusplus
}
#endif

#endif
