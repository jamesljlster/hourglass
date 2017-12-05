#ifndef __LSTM_TRAINER_H__
#define __LSTM_TRAINER_H__

#include <args.h>

enum LT_ARG
{
	LT_ARG_CSV_PATH,
	LT_ARG_CFG_PATH,
	LT_ARG_OUT_PATH,
	LT_ARG_MAX_ITER,
	LT_ARG_TARGET_MSE,
	LT_ARG_HELP
};

extern args_t arg_list[];

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
