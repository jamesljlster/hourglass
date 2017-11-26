#ifndef __TRASVC_TYPES_H__
#define __TRASVC_TYPES_H__

#include <lstm.h>

struct TRASVC
{
	lstm_t lstm;
	lstm_state_t lstmState;
	lstm_config_t lstmCfg;
};

#endif
