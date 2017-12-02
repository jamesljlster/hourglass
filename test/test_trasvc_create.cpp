#include <stdio.h>
#include <stdlib.h>

#include <trasvc.h>

#define DATA_LIMIT	1000

int main()
{
	int ret;
	lstm_config_t cfg = NULL;
	trasvc_t ts = NULL;

	// Create lstm config
	ret = lstm_config_create(&cfg);
	if(ret < 0)
	{
		printf("lstm_config_create() failed with error: %d\n", ret);
		return -1;
	}

	while(1)
	{
		// Create trasvc
		ret = trasvc_create(&ts, cfg, DATA_LIMIT);
		if(ret < 0)
		{
			printf("trasvc_create() failed with error: %d\n", ret);
			return -1;
		}

		trasvc_delete(ts);
	}

	lstm_config_delete(cfg);

	return 0;
}

