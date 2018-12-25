#include <tracker.hpp>

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
    Tracker tkr;

    if (!tkr.init(argc, argv))
    {
        return -1;
    }

    tkr.arg_print();

    return 0;
}
