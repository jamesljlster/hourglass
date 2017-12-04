#include <iostream>
#include <cstring>

#include <ftsvc.hpp>

#include "tracker.hpp"

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
	struct TKR tkr;

	// Setup feature
	ftsvc ft;
	ft.set_line_height_filter(50);

	// Initialize
	if(!tkr_init(&tkr, argc, argv))
	{
		cout << "Initialization failed!" << endl;
		return -1;
	}

	// Cleanup
	tkr_delete(&tkr);

	return 0;
}
