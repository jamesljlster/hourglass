#include <iostream>
#include <cstring>

#include "tracker.hpp"

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
	int ret;
	struct TKR tkr;

	// Zero memory
	memset(&tkr, 0, sizeof(struct TKR));

	// Processing arguments
	ret = args_parse(arg_list, argc, argv, NULL);
	if (ret < 0)
	{
		ret = -1;
		goto RET;
	}

RET:
	return ret;
}
