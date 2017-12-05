#ifndef __WCLT_PRIVATE_H__
#define __WCLT_PRIVATE_H__

#define WCLT_HEAD_CHAR 'W'
#define WCLT_END_CHAR 0x0A

#define WCLT_OK_STR "WOK"
#define WCLT_ERR_STR "WERR"
#define WCLT_LOCK_STR "WLOCK"
#define WCLT_UNLOCK_STR "WUNLOCK"
#define WCLT_GET_STR "WGET"

#ifdef __cplusplus
extern "C" {
#endif

int wclt_str_recv(int sock, char* buf, int bufLen);

#ifdef __cplusplus
}
#endif

#endif
