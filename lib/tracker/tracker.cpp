#include "tracker.hpp"

using namespace std;

namespace hourglass
{
bool Tracker::init(int argc, char* argv[])
{
    bool ret = true;

    // Parse argument
    ret &= this->arg_parse(argc, argv);

    return ret;
}

}  // namespace hourglass
