#include <stdlib.h>
#include <stdio.h>

#include "trasvc.h"
#include "trasvc_private.h"
#include "trasvc_types.h"

#include "debug.h"

int trasvc_create(trasvc_t* svcPtr, lstm_config_t lstmCfg)
{
	int ret = TRASVC_NO_ERROR;
	struct TRASVC* tmpSvcPtr = NULL;

	LOG("enter");

	// Memory allocation
	trasvc_alloc(tmpSvcPtr, 1, struct TRASVC, ret, RET);

	// Clone lstm config
	trasvc_run(lstm_config_clone(&tmpSvcPtr->lstmCfg, lstmCfg), ret, ERR);

	// Create lstm state

	goto RET;

ERR:
	trasvc_delete(tmpSvcPtr);

RET:
	LOG("exit");
	return ret;
}

void trasvc_delete(trasvc_t svc)
{
	if(svc != NULL)
	{
		lstm_delete(svc->lstm);
		lstm_state_delete(svc->lstmState);
		lstm_config_delete(svc->lstmCfg);

		trasvc_free(svc);
	}
}

int trasvc_start(trasvc_t svc);
void trasvc_stop(trasvc_t svc);

