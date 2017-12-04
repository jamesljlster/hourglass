#include <stdio.h>

#include "wclt.h"
#include "wclt_private.h"

#define WCLT_BUF_SIZE 100

#define WCLT_MAX_SPEED 510
#define WCLT_MIN_SPEED 0

int wclt_control(wclt_t wclt, int leftSpeed, int rightSpeed)
{
	int ret = WCLT_NO_ERROR;
	char buf[WCLT_BUF_SIZE] = {0};

	// Checking
	if(leftSpeed < WCLT_MIN_SPEED || leftSpeed > WCLT_MAX_SPEED ||
			rightSpeed < WCLT_MIN_SPEED || rightSpeed > WCLT_MAX_SPEED)
	{
		ret = WCLT_INVALID_ARG;
		goto RET;
	}

	// Make control string
	ret = snprintf(buf, WCLT_BUF_SIZE, "%c%03d%03d%c",
			WCLT_HEAD_CHAR,
			leftSpeed, rightSpeed,
			WCLT_END_CHAR
			);
	if(ret < 0)
	{
		ret = WCLT_INSUFFICIENT_BUF;
		goto RET;
	}

	// Send control string

RET:
	return ret;
}
