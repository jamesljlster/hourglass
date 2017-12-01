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

// TRASVC command flag
#define TRASVC_CMD_HEAD_FLAG	0x01
#define TRASVC_CMD_APPEND_FLAG	0x02
#define TRASVC_CMD_OK_FLAG		0x04
#define TRASVC_CMD_ERR_FLAG		0x08
#define TRASVC_CMD_TIMEOUT_FLAG	0x10
#define TRASVC_CMD_START_FLAG	0x20
#define TRASVC_CMD_STOP_FLAG	0x40

enum TRASVC_RET_VALUE
{
	//TRASVC_BUF_FULL = 3,
	//TRASVC_ACTIVE = 2,
	//TRASVC_INACTIVE = 1,
	TRASVC_NO_ERROR = 0,
	TRASVC_MEM_FAILED = -1,
	TRASVC_SYS_FAILED = -2,
	TRASVC_TIMEOUT = -3,
	TRASVC_CONNECTION_CLOSED = -4,
	TRASVC_CONNECT_FAILED = -5,
	TRASVC_INVALID_CMD = -6,
	TRASVC_INSUFFICIENT_BUF = -7
};

typedef struct TRASVC* trasvc_t;
typedef int trasvc_client_t;

#ifdef __cplusplus
extern "C" {
#endif

// Server functions
int trasvc_create(trasvc_t* svcPtr, lstm_config_t lstmCfg, int dataLimit);
void trasvc_delete(trasvc_t svc);

int trasvc_start(trasvc_t svc);
void trasvc_stop(trasvc_t svc);

int trasvc_data_append(trasvc_t svc, float* data, int timeout);
int trasvc_get_status(trasvc_t svc);

void trasvc_client_task(void* arg, int sock);

// Client functions
int trasvc_client_connect(trasvc_client_t* clientPtr, const char* serverIP, int serverPort);
int trasvc_client_datasend(trasvc_client_t client, float* data, int dataLen);
void trasvc_client_disconnect(trasvc_client_t client);

// Common functions
const char* trasvc_get_error_msg(int ret);

#ifdef __cplusplus
}
#endif

#endif
