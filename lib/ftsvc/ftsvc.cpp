
#include "ftsvc.hpp"

#include "debug.h"

namespace hourglass
{
	ftsvc::ftsvc()
	{
		this->thStatus = 0;
	}

	ftsvc::~ftsvc()
	{
		if(thStatus > 0)
		{
			pthread_cancel(this->th);
			pthread_join(this->th, NULL);
		}
	}
}
