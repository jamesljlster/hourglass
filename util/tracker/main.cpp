#include <exception>
#include <iostream>
#include <stdexcept>

#include <tracker.hpp>

using namespace std;
using namespace hourglass;

int main(int argc, char* argv[])
{
    int ret = 0;

    try
    {
        // Initial tracker class
        Tracker tkr(argc, argv);

        // Start log
        tkr.start_new_log();

        // Tracking
        while (1)
        {
            float ctrlErr, sal, sar;

            // Get feature
            ctrlErr = tkr.get_norm_feature();
            if (tkr.ft_kbin() == 27)
            {
                throw runtime_error("Program terminated by user");
            }

            // Find speed
            tkr.get_norm_ctrl_speed(ctrlErr, &sal, &sar);

            // Control Wheel
            tkr.wheel_ctrl(sal, sar);

            // Dump log
            tkr.dump_info(ctrlErr, sal, sar);
        }
    }
    catch (exception& ex)
    {
        cout << "Error occurred: " << ex.what() << endl;
        ret = -1;
    }

    return ret;
}
