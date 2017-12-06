#include <iostream>
#include <cstring>
#include <fstream>
#include <ctime>

#include <ftsvc.hpp>

#include "tracker.hpp"

#define SPEED_MAX 510
#define SPEED_MIN 0

#define INPUTS 1
#define OUTPUTS 2

#define CAM_PATH 0
#define CAM_WIDTH 320
#define CAM_HEIGHT 240

#define LOG_BASE "learner_log_"
#define LOG_EXT ".log"

#define SPEED_UP_INTERVAL			0.17
#define SPEED_DELTA_UP_INTERVAL		0.33
#define SPEED_PRESERVE_INTERVAL		0.66
#define SPEED_BASE_DOWN_INTERVAL	0.83
#define SPEED_DOWN_INTERVAL			1.0

#define SPEED_LRATE	0.01

using namespace std;
using namespace hourglass;

struct tm get_current_date()
{
	time_t t;
	struct tm tm;

	t = time(NULL);
	tm = *localtime(&t);

	tm.tm_year += 1990;
	tm.tm_mon += 1;

	return tm;
}

string make_time_str()
{
	struct tm date = get_current_date();
	return to_string(date.tm_year) + to_string(date.tm_mon) + to_string(date.tm_mday) + "_" + to_string(date.tm_hour) + to_string(date.tm_min);
}

void reinf_speed(float err, int sal, int sar, int* reSalPtr, int* reSarPtr)
{
	int tmpSal, tmpSar;

	int baseSpeed = (sal + sar) / 2.0;
	int speedDelta = sar - baseSpeed;
	float absErr = fabs(err);

	if(absErr < SPEED_UP_INTERVAL)
	{
		tmpSal = sal * (1.0 + SPEED_LRATE);
		tmpSar = sar * (1.0 + SPEED_LRATE);
	}
	else if(absErr < SPEED_DELTA_UP_INTERVAL)
	{
		speedDelta = speedDelta * (1.0 + SPEED_LRATE);
		tmpSal = baseSpeed - speedDelta;
		tmpSar = baseSpeed + speedDelta;
	}
	else if(absErr < SPEED_PRESERVE_INTERVAL)
	{
		// No change
		tmpSal = sal;
		tmpSar = sar;
	}
	else if(absErr < SPEED_BASE_DOWN_INTERVAL)
	{
		speedDelta = speedDelta * (1.0 - SPEED_LRATE);
		tmpSal = baseSpeed - speedDelta;
		tmpSar = baseSpeed + speedDelta;
	}
	else
	{
		tmpSal = sal * (1.0 - SPEED_LRATE);
		tmpSar = sar * (1.0 - SPEED_LRATE);
	}

	// Assign value
	*reSalPtr = tmpSal;
	*reSarPtr = tmpSar;
}

int main(int argc, char* argv[])
{
	int ret = 0;
	struct TKR tkr;
	int sal, sar;
	int reSal, reSar;
	ftsvc ft;
	fstream fLog;

	float inputList[INPUTS] = {0};
	float outputList[OUTPUTS] = {0};

	string pathTmp;

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

	// Upload lstm model
	ret = trasvc_client_model_upload(tkr.ts, arg_list[TKR_ARG_MODEL_PATH].leading[0]);
	if(ret < 0)
	{
		cout << "trasvc_client_model_upload() failed with error: " << trasvc_get_error_msg(ret) << endl;
		goto RET;
	}

	// Setup feature
	ft.set_line_height_filter(10);
	ft.set_image_show(1);

	// Dump csv header
	pathTmp = LOG_BASE + make_time_str() + "_init" + LOG_EXT;
	fLog.open(pathTmp.c_str(), ios_base::out);
	if(!fLog.is_open())
	{
		cout << "Failed to open log file with path: " << pathTmp.c_str() << endl;
		goto RET;
	}
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
