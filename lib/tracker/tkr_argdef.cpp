#include <cstdlib>

#include <args.h>
#include <modcfg.h>

#include "tkr_argdef.hpp"
#include "tracker.hpp"

#define __run_chk(func, retVal, errLabel)                    \
    retVal = func;                                           \
    if (retVal < 0)                                          \
    {                                                        \
        printf("%s failed with error: %d\n", #func, retVal); \
        goto errLabel;                                       \
    }

args_t arg_list[] = {
    {0, "cfg-path", 'C', 1, NULL, NULL, "Tracker config file path"},
    {0, "lstm-model", 'M', 1, NULL, NULL,
     "LSTM control model base (override config)"},
    {0, "help", 'H', 0, NULL, NULL, "Help"},
    ARGS_TERMINATE};

namespace hourglass
{
bool Tracker::arg_parse(int argc, char* argv[])
{
    int ret = 0;
    MODCFG cfg = NULL;

    // Processing arguments
    __run_chk(args_parse(arg_list, argc, argv, NULL), ret, RET);

    // Check argument
#define __arg_chk(index)                                          \
    if (arg_list[index].enable <= 0)                              \
    {                                                             \
        printf("'%s' argument not set!\n", arg_list[index].name); \
        ret = -1;                                                 \
    }

    __arg_chk(TKRARG_LIST::TKRARG_CFG_PATH);

    if (ret < 0 || arg_list[TKRARG_LIST::TKRARG_HELP].enable > 0)
    {
        printf("\nUsage:\n");
        args_print_help(arg_list);
        printf("\n");

        goto RET;
    }

    // Read config file
    __run_chk(
        modcfg_create(&cfg, arg_list[TKRARG_LIST::TKRARG_CFG_PATH].leading[0]),
        ret, RET);

    // Parse control method

RET:
    modcfg_delete(cfg);
    return (ret >= 0);
}

}  // namespace hourglass
