#include <cstdlib>

#include "tracker.hpp"

args_t arg_list[] = {
	{0, "cfg-path", 'C', 1, NULL, NULL, "Tracker config file path"},
	{0, "lstm-model", 'M', 1, NULL, NULL, "LSTM control model base"},
	{0, "help", 'H', 0, NULL, NULL, "Help"},
	ARGS_TERMINATE
};
