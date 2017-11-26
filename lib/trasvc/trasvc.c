#include <stdlib.h>
#include <stdio.h>

#include "trasvc.h"
#include "trasvc_private.h"
#include "trasvc_types.h"

#include "debug.h"

int trasvc_create(trasvc_t* svcPtr, lstm_config_t lstmCfg, int dataLimit)
{
	int i;
	int rows, cols;
	int ret = TRASVC_NO_ERROR;
	struct TRASVC* tmpSvcPtr = NULL;

	LOG("enter");

	// Find data rows, cols
	rows = dataLimit + 1;
	cols = lstm_config_get_inputs(lstmCfg) + lstm_config_get_outputs(lstmCfg);

	// Memory allocation
	trasvc_alloc(tmpSvcPtr, 1, struct TRASVC, ret, RET);
	trasvc_alloc(tmpSvcPtr->data, rows, float*, ret, ERR);
	for(i = 0; i < rows; i++)
	{
		trasvc_alloc(tmpSvcPtr->data[i], cols, float, ret, ERR);
	}

	// Clone lstm config
	trasvc_run(lstm_config_clone(&tmpSvcPtr->lstmCfg, lstmCfg), ret, ERR);

	// Create lstm state
	trasvc_run(lstm_state_create(&tmpSvcPtr->lstmState, lstmCfg), ret, ERR);

	// Assign value
	tmpSvcPtr->dataMemLen = rows;
	*svcPtr = tmpSvcPtr;

	goto RET;

ERR:
	trasvc_delete(tmpSvcPtr);

RET:
	LOG("exit");
	return ret;
}

void trasvc_delete(trasvc_t svc)
{
	int i;

	if(svc != NULL)
	{
		lstm_delete(svc->lstm);
		lstm_state_delete(svc->lstmState);
		lstm_config_delete(svc->lstmCfg);

		if(svc->data != NULL)
		{
			for(i = 0; i < svc->dataMemLen; i++)
			{
				trasvc_free(svc->data[i]);
			}

			trasvc_free(svc->data);
		}

		trasvc_free(svc);
	}
}

int trasvc_start(trasvc_t svc);
void trasvc_stop(trasvc_t svc);

