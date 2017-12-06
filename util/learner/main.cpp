#include <unistd.h>
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

#define MODEL_BASE "learner_model_"
#define MODEL_EXT ".lstm"

#define SPEED_UP_INTERVAL			0.17
#define SPEED_DELTA_UP_INTERVAL		0.33
#define SPEED_PRESERVE_INTERVAL		0.66
#define SPEED_BASE_DOWN_INTERVAL	0.83
#define SPEED_DOWN_INTERVAL			1.0

#define SPEED_LRATE	0.01

#define SEND_LIMIT	2500
#define TARGET_MSE	0.0001

#define BUF_SIZE 128

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
	char strBuf[BUF_SIZE] = {0};
	struct tm date = get_current_date();

	snprintf(strBuf, BUF_SIZE, "%d%02d%02d_%02d%02d", date.tm_year, date.tm_mon, date.tm_mday, date.tm_hour, date.tm_min);

	string ret = strBuf;
	return ret;
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
	int sendCounter;
	struct TKR tkr;
	int sal, sar;
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

	// Start training service
	ret = trasvc_client_start(tkr.ts);
	if(ret < 0)
	{
		cout << "trasvc_client_start() failed with error: " << trasvc_get_error_msg(ret) << endl;
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
	sendCounter = 0;
	while(tkr.stop == 0)
	{
		float ctrlErr;

		// Get feature
		ctrlErr = ft.get_norm_feature();
		inputList[0] = ft.get_norm_feature();
		if(ft.kbin == 27)
		{
			tkr.stop = 1;
		}

		// Send reinforcement data
		if(sendCounter >= SEND_LIMIT)
		{
			float mse;

			// Stop wheel
			ret = wclt_control(tkr.wclt, 255, 255);
			if(ret < 0)
			{
				cout << "wclt_control() failed with error: " << ret << endl;
				goto RET;
			}

			// Watiting for new model
			cout << "Waiting for new model" << endl;
			ret = trasvc_client_get_mse(tkr.ts, &mse);
			if(ret < 0)
			{
				cout << "trasvc_client_get_mse() failed with error: " << trasvc_get_error_msg(ret) << endl;
				goto RET;
			}

			// Check if new model available
			if(mse < TARGET_MSE)
			{
				lstm_t tmpLstm;

				// Download model
				cout << "New model available! Updating..." << endl;
				ret = trasvc_client_model_download(tkr.ts, &tmpLstm);
				if(ret < 0)
				{
					cout << "trasvc_client_model_download() failed with error: " << trasvc_get_error_msg(ret) << endl;
					goto RET;
				}

				// Export new model
				pathTmp = MODEL_BASE + make_time_str() + MODEL_EXT;
				ret = lstm_export(tmpLstm, pathTmp.c_str());
				if(ret < 0)
				{
					cout << "lstm_export() failed with error: " << ret << endl;
					goto RET;
				}

				// Replace model
				lstm_delete(tkr.model);
				tkr.model = tmpLstm;

				// Create new log
				fLog.close();
				pathTmp = LOG_BASE + make_time_str() + LOG_EXT;
				fLog.open(pathTmp.c_str(), ios_base::out);
				if(!fLog.is_open())
				{
					cout << "Failed to open log file with path: " << pathTmp.c_str() << endl;
					goto RET;
				}
				fLog << "error,sal,sar" << endl;

				// Reset sendCounter
				sendCounter = 0;
			}
			else
			{
				cout << "Current mse: " << mse << endl;
				usleep(1000 * 1000);
			}
		}
		else
		{
			int reSal, reSar;
			float dataTmp[INPUTS + OUTPUTS] = {0};

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

			// Get reinforcement data
			reinf_speed(ctrlErr, sal, sar, &reSal, &reSar);

			// Set data
			dataTmp[0] = ctrlErr;
			dataTmp[1] = reSal / (float)SPEED_MAX;
			dataTmp[2] = reSar / (float)SPEED_MAX;

			// Send data
			while(1)
			{
				ret = trasvc_client_datasend(tkr.ts, dataTmp, INPUTS + OUTPUTS);
				if(ret < 0)
				{
					// Stop Wheel
					ret = wclt_control(tkr.wclt, 255, 255);
					if(ret < 0)
					{
						cout << "wclt_control() failed with error: " << ret << endl;
						goto RET;
					}

					cout << "trasvc_client_datasend() failed with error: " << trasvc_get_error_msg(ret) << endl;
					if(ret == TRASVC_TIMEOUT)
					{
						continue;
					}
					else
					{
						goto RET;
					}
				}
				else
				{
					break;
				}
			}

			sendCounter++;

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
	}

RET:
	// Stop Wheel
	ret = wclt_control(tkr.wclt, 255, 255);
	if(ret < 0)
	{
		cout << "wclt_control() failed with error: " << ret << endl;
		goto RET;
	}

	// Stop training service
	trasvc_client_stop(tkr.ts);

	// Cleanup
	fLog.close();
	tkr_delete(&tkr);

	return ret;
}
