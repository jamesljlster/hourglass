#ifndef __WCLT_H__
#define __WCLT_H__

typedef int wclt_t;

enum WCLT_RETURN_VALUE
{
	WCLT_NO_ERROR = 0,
	WCLT_CONTROL_FAILED = -1,
	WCLT_CONNECT_FAILED = -2,
	WCLT_SYS_FAILED = -3,
	WCLT_INSUFFICIENT_BUF = -4,
	WCLT_INVALID_ARG = -5
};

#ifdef __cplusplus
extern "C" {
#endif

int wclt_connect(wclt_t* wcltPtr, const char* serverIP, int serverPort);
int wclt_control(wclt_t wclt, int leftSpeed, int rightSpeed);
int wclt_lock(wclt_t wclt);
int wclt_unlock(wclt_t wclt);
void wclt_disconnect(wclt_t wclt);

#ifdef __cplusplus
}
#endif

#endif
