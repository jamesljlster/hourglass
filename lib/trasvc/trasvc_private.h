#ifndef __TRASVC_PRIVATE_H__
#define __TRASVC_PRIVATE_H__

#include "trasvc_types.h"

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

#ifdef DEBUG
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

#ifdef __cplusplus
extern "C" {
#endif

int trasvc_data_struct_init(struct TRASVC_DATA* dataPtr, int rowLimit, int cols);
void trasvc_data_struct_cleanup(struct TRASVC_DATA* dataPtr);

void* trasvc_tra_task(void* arg);

#ifdef __cplusplus
}
#endif

#endif
