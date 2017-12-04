#include <iostream>
#include <cstring>

#include "tracker.hpp"

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
	struct TKR tkr;

	if(!tkr_init(&tkr, argc, argv))
	{
		cout << "Initialization failed!" << endl;
		return -1;
	}

	return 0;
}
