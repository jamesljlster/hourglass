#include <iostream>
#include <cstring>

#include <ftsvc.hpp>
#include <SPID.h>

#include "tracker.hpp"

#define SPEED_MAX 510
#define SPEED_MIN 0
#define SPEED_BASE 320

#define KP 0.0
#define KD 0.0
#define KI 0.0

#define CAM_PATH 0
#define CAM_WIDTH 320
#define CAM_HEIGHT 240

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
	int ret = 0;
	float ftInput, delta;
	int sal, sar;
	struct TKR tkr;
	SPID sPid;
	ftsvc ft;

	// Initialize
	if(!tkr_init(&tkr, argc, argv))
	{
		cout << "Initialization failed!" << endl;
		return -1;
	}

	SPID_ZeroMemory(&sPid);
	SPID_SetWeight(&sPid, KP, KI, KD);

	if(!ft.open_cam(CAM_PATH, CAM_WIDTH, CAM_HEIGHT))
	{
		cout << "Failed to open camera!" << endl;
		goto RET;
	}

	// Setup feature
	ft.set_line_height_filter(50);
	ft.set_image_show(1);

	// Tracking
	while(tkr.stop == 0)
	{
		// Get feature
		ftInput = ft.get_norm_feature();
		delta = SPID_Control(&sPid, 0, ftInput);

		// Find speed
		sal = SPEED_BASE - delta;
		sar = SPEED_BASE + delta;

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
	}

RET:
	// Cleanup
	tkr_delete(&tkr);

	return ret;
}
