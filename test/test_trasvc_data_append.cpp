#include <stdio.h>
#include <stdlib.h>

#include <trasvc.h>
#include <trasvc_private.h>

#define DATA_LIMIT	3000

#define INPUTS 1
#define OUTPUTS 2
#define TIMEOUT	1000

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

	lstm_config_set_inputs(cfg, INPUTS);
	lstm_config_set_outputs(cfg, OUTPUTS);

	// Create trasvc
	ret = trasvc_create(&ts, cfg, DATA_LIMIT);
	if(ret < 0)
	{
		printf("trasvc_create() failed with error: %d\n", ret);
		return -1;
	}

	// Test append data
	int cols = INPUTS + OUTPUTS;
	float* tmpData = new float[cols]();
	for(int i = 0; i < cols; i++)
	{
		tmpData[i] = (float)i / 10.0;
	}

	ret = trasvc_data_append(ts, tmpData, TIMEOUT);
	if(ret != 0)
	{
		printf("trasvc_data_append() failed with error: %d\n", ret);
	}
	else
	{
		printf("trasvc_data_append() succeed\n");
	}

	for(int i = 0; i < cols; i++)
	{
		tmpData[i] = (float)(i + cols) / 10.0;
	}

	ret = trasvc_data_append(ts, tmpData, TIMEOUT);
	if(ret != 0)
	{
		printf("trasvc_data_append() failed with error: %d\n", ret);
	}
	else
	{
		printf("trasvc_data_append() succeed\n");
	}

	// Lock mgr data
	ret = pthread_mutex_lock(&ts->mgrData.mutex);
	if(ret != 0)
	{
		printf("pthread_mutex_lock() failed with error: %d\n", ret);
	}

	ret = trasvc_data_append(ts, tmpData, TIMEOUT);
	if(ret != 0)
	{
		printf("trasvc_data_append() failed with error: %d\n", ret);
	}
	else
	{
		printf("trasvc_data_append() succeed\n");
	}

	// Verify data
	printf("\n");
	printf("Head: %d\n", ts->mgrData.dataHead);
	printf("Tail: %d\n", ts->mgrData.dataTail);

	int len = (ts->mgrData.dataHead - ts->mgrData.dataTail);
	if(len < 0)
	{
		len += ts->mgrData.dataMemLen;
	}

	for(int i = 0; i < len; i++)
	{
		int tmpIndex = (ts->mgrData.dataTail + 1 + i) % ts->mgrData.dataMemLen;
		printf("tmpIndex = %d\n", tmpIndex);
		for(int j = 0; j < cols; j++)
		{
			printf("%f, ", ts->mgrData.data[tmpIndex][j]);
		}
		printf("\n");
	}

	trasvc_delete(ts);
	lstm_config_delete(cfg);

	return 0;
}

