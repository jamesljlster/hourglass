#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <trasvc.h>
#include <trasvc_private.h>

#define DATA_LIMIT 1000
#define DATA_AMOUNT	304

#define INPUTS 1
#define OUTPUTS 1

#define IT_FUNC		LSTM_SIGMOID
#define OT_FUNC		LSTM_SIGMOID

#define TIMEOUT 1000

#define TARGET_MSE 0.001

extern float dataset[];

int main()
{
	int ret;
	lstm_config_t cfg = NULL;
	trasvc_t ts = NULL;
	clock_t timeHold;

	// Create lstm config
	ret = lstm_config_create(&cfg);
	if(ret < 0)
	{
		printf("lstm_config_create() failed with error: %d\n", ret);
		return -1;
	}

	lstm_config_set_inputs(cfg, INPUTS);
	lstm_config_set_outputs(cfg, OUTPUTS);
	lstm_config_set_input_transfer_func(cfg, IT_FUNC);
	lstm_config_set_output_transfer_func(cfg, OT_FUNC);
	lstm_config_set_learning_rate(cfg, 0.1);
	lstm_config_set_momentum_coef(cfg, 0.0);

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

	// Insert dataset
	for(int i = 0; i < DATA_AMOUNT - 1; i++)
	{
		ret = trasvc_data_append(ts, &dataset[i], TIMEOUT);
		if(ret != 0)
		{
			printf("trasvc_data_append() failed with error: %d\n", ret);
			i--;
			continue;
		}
	}

	timeHold = clock();
	while(1)
	{
		// Delay
		usleep(1000 * 1000);

		double tmp = ts->mse;
		printf("mse: %f, time cost: %f\n", tmp, (float)(clock() - timeHold) / (float)CLOCKS_PER_SEC);
		if(tmp < TARGET_MSE)
		{
			break;
		}
	}

	// Stop task
	trasvc_stop(ts);

	// Export
	ret = lstm_export(ts->lstm, "test.lstm");
	if(ret < 0)
	{
		printf("lstm_export() failed with error: %d\n", ret);
		return -1;
	}

	trasvc_delete(ts);
	lstm_config_delete(cfg);

	return 0;
}

float dataset[] = {
	0.712162162,
	0.706756757,
	0.704054054,
	0.7        ,
	0.701351351,
	0.701351351,
	0.701351351,
	0.702702703,
	0.712162162,
	0.712162162,
	0.712162162,
	0.712162162,
	0.709459459,
	0.709459459,
	0.708108108,
	0.702702703,
	0.709459459,
	0.704054054,
	0.702702703,
	0.709459459,
	0.695945946,
	0.686486486,
	0.682432432,
	0.683783784,
	0.683783784,
	0.691891892,
	0.693243243,
	0.691891892,
	0.697297297,
	0.709459459,
	0.704054054,
	0.8        ,
	0.791891892,
	0.856756757,
	0.854054054,
	0.854054054,
	0.835135135,
	0.818918919,
	0.818918919,
	0.832432432,
	0.9        ,
	0.898648649,
	0.945945946,
	0.956756757,
	1          ,
	0.968918919,
	0.921621622,
	0.928378378,
	0.941891892,
	0.945945946,
	0.966216216,
	0.989189189,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.783783784,
	0.535135135,
	0.606756757,
	0.606756757,
	0.610810811,
	0.610810811,
	0.610810811,
	0.610810811,
	0.610810811,
	0.610810811,
	0.597297297,
	0.597297297,
	0.589189189,
	0.577027027,
	0.568918919,
	0.566216216,
	0.556756757,
	0.543243243,
	0.543243243,
	0.540540541,
	0.525675676,
	0.505405405,
	0.491891892,
	0.490540541,
	0.471621622,
	0.485135135,
	0.478378378,
	0.47972973 ,
	0.474324324,
	0.472972973,
	0.47972973 ,
	0.483783784,
	0.483783784,
	0.486486486,
	0.489189189,
	0.489189189,
	0.491891892,
	0.490540541,
	0.486486486,
	0.485135135,
	0.489189189,
	0.501351351,
	0.502702703,
	0.505405405,
	0.509459459,
	0.516216216,
	0.52027027 ,
	0.52027027 ,
	0.525675676,
	0.533783784,
	0.537837838,
	0.543243243,
	0.552702703,
	0.560810811,
	0.57027027 ,
	0.587837838,
	0.616216216,
	0.633783784,
	0.636486486,
	0.628378378,
	0.617567568,
	0.593243243,
	0.583783784,
	0.578378378,
	0.566216216,
	0.560810811,
	0.552702703,
	0.55       ,
	0.552702703,
	0.524324324,
	0.497297297,
	0.493243243,
	0.494594595,
	0.489189189,
	0.487837838,
	0.490540541,
	0.474324324,
	0.389189189,
	0.390540541,
	0.409459459,
	0.410810811,
	0.425675676,
	0.436486486,
	0.447297297,
	0.464864865,
	0.468918919,
	0.456756757,
	0.489189189,
	0.498648649,
	0.481081081,
	0.477027027,
	0.463513514,
	0.431081081,
	0.424324324,
	0.443243243,
	0.445945946,
	0.468918919,
	0.489189189,
	0.459459459,
	0.452702703,
	0.337837838,
	0.295945946,
	0.228378378,
	0.236486486,
	0.239189189,
	0.286486486,
	0.258108108,
	0.236486486,
	0.217567568,
	0.183783784,
	0.181081081,
	0.208108108,
	0.168918919,
	0.186486486,
	0.190540541,
	0.181081081,
	0.121621622,
	0.027027027,
	0.040540541,
	0.062162162,
	0.090540541,
	0.094594595,
	0.104054054,
	0.074324324,
	0.091891892,
	0.109459459,
	0.097297297,
	0.114864865,
	0.125675676,
	0.108108108,
	0.106756757,
	0.12027027 ,
	0.059459459,
	0.067567568,
	0.063513514,
	0.07027027 ,
	0.078378378,
	0.087837838,
	0.106756757,
	0.156756757,
	0.168918919,
	0.185135135,
	0.195945946,
	0.171621622,
	0.186486486,
	0.221621622,
	0.214864865,
	0.262162162,
	0.166216216,
	0.201351351,
	0.171621622,
	0.155405405,
	0.163513514,
	0.141891892,
	0.171621622,
	0.168918919,
	0.181081081,
	0.177027027,
	0.186486486,
	0.163513514,
	0.158108108,
	0.168918919,
	0.166216216,
	0.174324324,
	0.193243243,
	0.177027027,
	0.190540541,
	0.193243243,
	0.133783784,
	0.12972973 ,
	0.135135135,
	0.137837838,
	0.143243243,
	0.167567568,
	0.174324324,
	0.151351351,
	0.167567568,
	0.172972973,
	0.178378378,
	0.17972973 ,
	0.162162162,
	0.158108108,
	0.116216216,
	0.114864865,
	0.128378378,
	0.152702703,
	0.143243243,
	0.12972973 ,
	0.106756757,
	0.1        ,
	0.058108108,
	0.059459459,
	0.062162162,
	0.060810811,
	0.068918919,
	0.058108108,
	0.072972973,
	0.075675676,
	0.067567568,
	0.028378378,
	0.02972973 ,
	0.044594595,
	0.027027027,
	0          ,
	0.028378378,
	0.024324324,
	0.047297297,
	0.090540541,
	0.085135135,
	0.064864865,
	0.071621622,
	0.098648649,
	0.077027027,
	0.089189189,
	0.089189189,
	0.082432432,
	0.078378378,
	0.110810811,
	0.12027027 ,
	0.097297297,
	0.075675676,
	0.045945946,
	0.031081081,
	0.016216216,
	0.014864865,
	0.05       ,
	0.089189189
};


