#include <iostream>
#include <cstring>
#include <fstream>

#include <ftsvc.hpp>

#include "tracker.hpp"

#define SPEED_MAX 510
#define SPEED_MIN 0

#define INPUTS 1
#define OUTPUTS 2

#define CAM_PATH 0
#define CAM_WIDTH 320
#define CAM_HEIGHT 240

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
	int ret = 0;
	struct TKR tkr;
	int sal, sar;
	ftsvc ft;
	fstream fLog;

	float inputList[INPUTS] = {0};
	float outputList[OUTPUTS] = {0};

	// Initialize
	if(!tkr_init(&tkr, argc, argv))
	{
		cout << "Initialization failed!" << endl;
		return -1;
	}

	if(lstm_config_get_inputs(lstm_get_config(tkr.model)) != INPUTS ||
			lstm_config_get_outputs(lstm_get_config(tkr.model)) != OUTPUTS)
	{
		cout << "Incompatible LSTM control model!" << endl;
		goto RET;
	}

	if(!ft.open_cam(CAM_PATH, CAM_WIDTH, CAM_HEIGHT))
	{
		cout << "Failed to open camera!" << endl;
		goto RET;
	}

	fLog.open(tkr.logPath, ios_base::out);
	if(!fLog.is_open())
	{
		cout << "Failed to open log file with path: " << tkr.logPath << endl;
		goto RET;
	}

	// Setup feature
	ft.set_line_height_filter(10);
	ft.set_image_show(1);

	// Dump csv header
	fLog << "error,sal,sar" << endl;

	// Tracking
	while(tkr.stop == 0)
	{
		// Get feature
		inputList[0] = ft.get_norm_feature();
		if(ft.kbin == 27)
		{
			tkr.stop = 1;
		}

		// Find speed
		lstm_forward_computation(tkr.model, inputList, outputList);
		sal = outputList[0] * SPEED_MAX;
		sar = outputList[1] * SPEED_MAX;

		if(sal > SPEED_MAX)
		{
			sal = SPEED_MAX;
		}
		else if(sal < SPEED_MIN)
		{
			sal = SPEED_MIN;
		}

		if(sar > SPEED_MAX)
		{
			sar = SPEED_MAX;
		}
		else if(sar < SPEED_MIN)
		{
			sar = SPEED_MIN;
		}

		// Control Wheel
		ret = wclt_control(tkr.wclt, sal, sar);
		if(ret < 0)
		{
			cout << "wclt_control() failed with error: " << ret << endl;
			goto RET;
		}

		// Dump log
		fLog << inputList[0] << "," << ((float)sal / (float)SPEED_MAX) << "," << ((float)sar / (float)SPEED_MAX) << endl;
	}

RET:
	// Stop Wheel
	ret = wclt_control(tkr.wclt, 255, 255);
	if(ret < 0)
	{
		cout << "wclt_control() failed with error: " << ret << endl;
		goto RET;
	}

	// Cleanup
	fLog.close();
	tkr_delete(&tkr);

	return ret;
}
