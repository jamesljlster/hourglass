#include <exception>

#include <tracker.hpp>

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
    try
    {
        Tracker tkr(argc, argv);

        tkr.arg_print();
    }
    catch (exception& ex)
    {
        cout << "Error occurred: " << ex.what() << endl;
    }

    return 0;
}
