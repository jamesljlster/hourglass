#include <tracker.hpp>

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
    Tracker tkr;

    tkr.init(argc, argv);
    tkr.arg_print();

    return 0;
}
