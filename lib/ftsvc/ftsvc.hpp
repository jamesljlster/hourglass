#ifndef __FTSVC_HPP__
#define __FTSVC_HPP__

#include <pthread.h>

#include <laneft.hpp>

namespace hourglass
{
	class ftsvc
	{
		public:
			ftsvc();
			~ftsvc();

		protected:
			int thStatus;
			pthread_t th;
	};
}

#endif
