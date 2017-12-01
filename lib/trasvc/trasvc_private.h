#ifndef __TRASVC_PRIVATE_H__
#define __TRASVC_PRIVATE_H__

#include "trasvc_types.h"

#define TRASVC_CMD_SEP_CHAR ' '
#define TRASVC_CMD_END_CHAR 0x0A

#define TRASVC_CMD_HEAD_STR "TS"
#define TRASVC_CMD_APPEND_STR "APPEND"
#define TRASVC_CMD_OK_STR "OK"
#define TRASVC_CMD_ERR_STR "ERR"
#define TRASVC_CMD_TIMEOUT_STR "TIMEOUT"
#define TRASVC_CMD_START_STR "START"
#define TRASVC_CMD_STOP_STR "STOP"
#define TRASVC_CMD_MSE_STR "MSE"
#define TRASVC_CMD_STATUS_STR "STATUS"
#define TRASVC_CMD_UPLOAD_STR "UPLOAD"
#define TRASVC_CMD_DOWNLOAD_STR "DOWNLOAD"

// Macros
#ifdef DEBUG
#include <stdio.h>
#define trasvc_free(ptr)	fprintf(stderr, "%s(): free(%s), %p\n", __FUNCTION__, #ptr, ptr); free(ptr)
#else
#define trasvc_free(ptr)	free(ptr)
#endif

#define trasvc_alloc(ptr, len, type, retVar, errLabel) \
	ptr = calloc(len, sizeof(type)); \
	if(ptr == NULL) \
	{ \
		ret = TRASVC_MEM_FAILED; \
		goto errLabel; \
	}

//#ifdef DEBUG
#if 1
#define trasvc_run(func, retVal, errLabel) \
	retVal = func; \
	if(retVal != TRASVC_NO_ERROR) \
	{ \
		fprintf(stderr, "%s(): %s failed with error: %d\n", __FUNCTION__, #func, retVal); \
		goto errLabel; \
	}
#else
#define trasvc_run(func, retVal, errLabel) \
	retVal = func; \
	if(retVal != TRASVC_NO_ERROR) \
	{ \
		goto errLabel; \
	}
#endif

extern const char* trasvc_errmsg[];
extern struct TRASVC_FLAG trasvc_flag_list[];

#ifdef __cplusplus
extern "C" {
#endif

int trasvc_data_struct_init(struct TRASVC_DATA* dataPtr, int rowLimit, int cols);
void trasvc_data_struct_cleanup(struct TRASVC_DATA* dataPtr);

void* trasvc_tra_task(void* arg);

void trasvc_mem_free(void* arg);
void trasvc_mutex_unlock(void* arg);

int trasvc_str_recv(int sock, char* buf, int bufLen);
int trasvc_data_recv(int sock, char* buf, int bufLen, int recvLen, int timeout);
int trasvc_model_send(int sock, lstm_t lstmSrc);
int trasvc_model_recv(int sock, lstm_t* lstmDstPtr);

int trasvc_cmd_parse(char* buf);

void trasvc_flag_update(trasvc_t svc);

#ifdef __cplusplus
}
#endif

#endif
