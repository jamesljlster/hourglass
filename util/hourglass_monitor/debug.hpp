#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#ifdef DEBUG
#include <cstdio>
#define LOG(msg, ...) 							\
	fprintf(stderr, "%s(): ", __FUNCTION__);	\
	fprintf(stderr, msg, ##__VA_ARGS__);		\
	fprintf(stderr, "\n");
#else
#define	LOG(msg, ...)
#endif

#endif
