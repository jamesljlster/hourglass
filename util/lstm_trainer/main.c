#include <stdlib.h>
#include <stdio.h>

#include <lstm.h>
#include <csv_dataproc.h>

#include "lstm_trainer.h"

#define DELTA_LIMIT	30

int main(int argc, char* argv[])
{
	int i, j, iter;
	int inputs, outputs;
	int rows, cols;
	int ret;
	float lRate, mCoef;
	float mse;

	char* tmpPtr;
	char* parsePtr;

	const char* csvPath = NULL;
	const char* cfgPath = NULL;
	const char* outPath = NULL;

	int maxIter;
	float mseTarget;

	float* outBuf = NULL;
	float* errBuf = NULL;

	csv_t csv;
	lstm_config_t lstmCfg = NULL;
	lstm_t lstm = NULL;

	// Processing argument
	ret = args_parse(arg_list, argc, argv, NULL);
	if(ret < 0)
	{
		ret = -1;
		goto RET;
	}

	// Checking
	if(arg_list[LT_ARG_CSV_PATH].enable <= 0)
	{
		ret = -1;
		printf("'%s' argument not set!\n", arg_list[LT_ARG_CSV_PATH].name);
	}

	if(arg_list[LT_ARG_CFG_PATH].enable <= 0)
	{
		ret = -1;
		printf("'%s' argument not set!\n", arg_list[LT_ARG_CFG_PATH].name);
	}

	if(arg_list[LT_ARG_OUT_PATH].enable <= 0)
	{
		ret = -1;
		printf("'%s' argument not set!\n", arg_list[LT_ARG_OUT_PATH].name);
	}

	if(ret < 0 || arg_list[LT_ARG_HELP].enable > 0)
	{
		printf("\n");
		printf("Usage:\n");
		args_print_help(arg_list);
		printf("\n");
		goto RET;
	}

	// Parsing argument
	csvPath = arg_list[LT_ARG_CSV_PATH].leading[0];
	cfgPath = arg_list[LT_ARG_CFG_PATH].leading[0];
	outPath = arg_list[LT_ARG_OUT_PATH].leading[0];

	parsePtr = arg_list[LT_ARG_MAX_ITER].leading[0];
	maxIter = strtol(parsePtr, &tmpPtr, 10);
	if(parsePtr == tmpPtr)
	{
		printf("Failed to convert '%s' to maximum iteration setting!\n", tmpPtr);
		ret = -1;
		goto RET;
	}

	parsePtr = arg_list[LT_ARG_TARGET_MSE].leading[0];
	mseTarget = strtod(parsePtr, &tmpPtr);
	if(parsePtr == tmpPtr)
	{
		printf("Failed to convert '%s' to target error!\n", tmpPtr);
		ret = -1;
		goto RET;
	}

	// Print summary
	printf("Summary:\n");
	args_print_summary(arg_list);
	printf("\n");

	// Import lstm config
	ret = lstm_config_import(&lstmCfg, cfgPath);
	if(ret < 0)
	{
		printf("lstm_config_import() failed with error: %d\n", ret);
		ret = -1;
		goto RET;
	}

	// Read csv
	ret = csv_read(&csv, csvPath);
	if(ret < 0)
	{
		printf("csv_read() failed with error: %d\n", ret);
		ret = -1;
		goto RET;
	}

	// Checking
	inputs = lstm_config_get_inputs(lstmCfg);
	outputs = lstm_config_get_outputs(lstmCfg);
	rows = csv->rows;
	cols = csv->cols;
	if(inputs + outputs != csv->cols)
	{
		printf("Incompatible LSTM config and csv data!\n");
		ret = -1;
		goto RET;
	}

	// Allocate buffer
	errBuf = calloc(outputs, sizeof(float));
	if(errBuf == NULL)
	{
		printf("Memory allocation failed!\n");
		ret = -1;
		goto RET;
	}

	outBuf = calloc(outputs, sizeof(float));
	if(outBuf == NULL)
	{
		printf("Memory allocation failed!\n");
		ret = -1;
		goto RET;
	}

	// Create lstm
	ret = lstm_create(&lstm, lstmCfg);
	if(ret < 0)
	{
		printf("lstm_create() failed with error: %d\n", ret);
		ret = -1;
		goto RET;
	}

	lstm_rand_network(lstm);

	// Training
	lRate = lstm_config_get_learning_rate(lstmCfg);
	mCoef = lstm_config_get_momentum_coef(lstmCfg);
	for(iter = 1; iter <= maxIter; iter++)
	{
		mse = 0;
		for(i = 0; i < rows; i++)
		{
			// Forward computation
			lstm_forward_computation(lstm, &csv->data[i * cols], outBuf);

			// Find error
			for(j = 0; j < outputs; j++)
			{
				errBuf[j] = csv->data[i * cols + inputs + j] - outBuf[j];
				mse += errBuf[j] * errBuf[j];
			}

			// Backpropagation
			lstm_bptt_sum_gradient(lstm, errBuf);
		}

		// Find mse
		mse /= (double)(rows * outputs);
		if(mse < mseTarget)
		{
			break;
		}

		printf("Iter. %5d, mse: %lf\n", iter, mse);

		// Adjust netwrok
		lstm_bptt_adjust_network(lstm, lRate, mCoef, DELTA_LIMIT);

		// Erase lstm
		lstm_bptt_erase(lstm);
		lstm_forward_computation_erase(lstm);
	}

	// Export lstm
	ret = lstm_export(lstm, outPath);
	if(ret < 0)
	{
		printf("lstm_export() failed with error: %d\n", ret);
	}

RET:
	lstm_config_delete(lstmCfg);
	lstm_delete(lstm);

	free(outBuf);
	free(errBuf);

	return ret;
}
