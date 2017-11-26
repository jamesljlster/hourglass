#include <stdlib.h>

#include "trasvc.h"
#include "trasvc_private.h"

#include "debug.h"

void* trasvc_tra_task(void* arg)
{
	pthread_exit(NULL);
	return NULL;
}
