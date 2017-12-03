#include <stdio.h>
#include <stdlib.h>

#include <args.h>
#include <lstm.h>

args_t arg_list[] = {
	{0, "cfg-path", 'C', 1, NULL, NULL, "LSTM config file path"},
	{0, "out-path", 'O', 1, NULL, NULL, "Output model path"},
	ARGS_TERMINATE
};

int main(int argc, char* argv[])
{
	int ret = 0;
	lstm_config_t lstmCfg = NULL;
	lstm_t lstm = NULL;

	// Process arguments
	ret = args_parse(arg_list, argc, argv, NULL);
	if(ret < 0)
	{
		goto RET;
	}

	if(arg_list[0].enable == 0 || arg_list[1].enable == 0)
	{
		printf("Usage:\n");
		args_print_help(arg_list);
		goto RET;
	}

	// Generate random model
	ret = lstm_config_import(&lstmCfg, arg_list[0].leading[0]);
	if(ret < 0)
	{
		printf("Import LSTM config with path '%s' failed with error: %d\n",
				arg_list[0].leading[0], ret);
		ret = -1;
		goto RET;
	}

	ret = lstm_create(&lstm, lstmCfg);
	if(ret < 0)
	{
		printf("lstm_create() failed with error: %d\n", ret);
		ret = -1;
		goto RET;
	}

	ret = lstm_export(lstm, arg_list[1].leading[0]);
	if(ret < 0)
	{
		printf("lstm_export() failed with error: %d\n", ret);
		ret = -1;
		goto RET;
	}

RET:
	lstm_config_delete(lstmCfg);
	lstm_delete(lstm);

	return ret;
}
