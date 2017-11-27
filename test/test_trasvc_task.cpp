#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <trasvc.h>
#include <trasvc_private.h>

#define DATA_LIMIT	3000

#define INPUTS 1
#define OUTPUTS 2
#define TIMEOUT	1000

#define BUF_LEN	100

int main()
{
	int ret;
	lstm_config_t cfg = NULL;
	trasvc_t ts = NULL;

	char buf[BUF_LEN] = {0};

	// Create lstm config
	ret = lstm_config_create(&cfg);
	if(ret < 0)
	{
		printf("lstm_config_create() failed with error: %d\n", ret);
		return -1;
	}

	lstm_config_set_inputs(cfg, INPUTS);
	lstm_config_set_outputs(cfg, OUTPUTS);

	// Create trasvc
	ret = trasvc_create(&ts, cfg, DATA_LIMIT);
	if(ret < 0)
	{
		printf("trasvc_create() failed with error: %d\n", ret);
		return -1;
	}

	// Start task
	ret = trasvc_start(ts);
	if(ret < 0)
	{
		printf("trasvc_start() failed with error: %d\n", ret);
		return -1;
	}

	while(1)
	{
		// Get user input
		ret = scanf("%99s", buf);
		if(ret == 0)
		{
			continue;
		}

		// Compare
		if(strcmp(buf, "stop") == 0)
		{
			break;
		}
	}

	// Stop task
	trasvc_stop(ts);

	trasvc_delete(ts);
	lstm_config_delete(cfg);

	return 0;
}

