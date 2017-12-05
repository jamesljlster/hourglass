#include <stdlib.h>

#include "lstm_trainer.h"

char* arg_iter[] = {
	"10000"
};

char* arg_mse[] = {
	"0.0001"
};

args_t arg_list[] = {
	{0, "csv", 'D', 1, NULL, NULL, "Training csv file path"},
	{0, "cfg", 'C', 1, NULL, NULL, "LSTM config file path"},
	{0, "out", 'O', 1, NULL, NULL, "LSTM model output path"},
	{1, "iter", 'I', 1, arg_iter, NULL, "Maximum training iteration"},
	{1, "mse", 'M', 1, arg_mse, NULL, "Target mse"},
	ARGS_TERMINATE
};
