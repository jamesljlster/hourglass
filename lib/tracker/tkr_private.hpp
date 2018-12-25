#ifndef __TKR_PRIVATE_HPP__
#define __TKR_PRIVATE_HPP__

#define __run_chk(func, retVal, errLabel)                    \
    retVal = func;                                           \
    if (retVal < 0)                                          \
    {                                                        \
        printf("%s failed with error: %d\n", #func, retVal); \
        goto errLabel;                                       \
    }

#define __ts_run_chk(func, retVal, errLabel)        \
    retVal = func;                                  \
    if (retVal < 0)                                 \
    {                                               \
        printf("%s failed with error: %s\n", #func, \
               trasvc_get_error_msg(retVal));       \
        goto errLabel;                              \
    }

#endif
