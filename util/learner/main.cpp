#include <unistd.h>

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

        int sendCounter = 0;
        int sendLimit = tkr.get_data_limit();
        float targetMse = tkr.get_target_mse();

        // Start log
        tkr.start_new_log(string("_init"));

        // Tracking
        sendCounter = 0;
        while (1)
        {
            float ctrlErr;

            // Get feature
            ctrlErr = tkr.get_norm_feature();
            if (tkr.ft_kbin() == 27)
            {
                throw runtime_error("Program terminated by user");
            }

            // Send reinforcement data
            if (sendCounter >= sendLimit)
            {
                float mse;

                // Stop wheel
                tkr.wheel_stop();

                // Watiting for new model
                cout << "Waiting for new model" << endl;
                mse = tkr.get_training_mse();

                // Check if new model available
                if (mse < targetMse)
                {
                    lstm_t tmpLstm;

                    // Download model
                    cout << "New model available! Updating..." << endl;
                    tkr.get_model(&tmpLstm);

                    // Export new model
                    string pathTmp = tkr.make_model_fname();
                    ret = lstm_export(tmpLstm, pathTmp.c_str());
                    if (ret < 0)
                    {
                        throw runtime_error(
                            string("lstm_export() failed with error: ") +
                            to_string(ret));
                    }

                    // Replace model
                    tkr.replace_model(tmpLstm);

                    // Create new log
                    tkr.start_new_log();

                    // Reset sendCounter
                    sendCounter = 0;

                    cout << "Finished!" << endl;
                }
                else
                {
                    cout << "Current mse: " << mse << endl;
                    usleep(1000 * 1000);
                }

                cout << endl;
            }
            else
            {
                float sal, sar;
                float reSal, reSar;

                // Find speed
                tkr.get_norm_ctrl_speed(ctrlErr, &sal, &sar);

                // Get reinforcement data
                tkr.reinf_speed_norm(ctrlErr, sal, sar, &reSal, &reSar);

                // Set data
                tkr.send_data(ctrlErr, reSal, reSar);
                sendCounter++;

                // Control Wheel
                tkr.wheel_ctrl(sal, sar);

                // Dump log
                tkr.dump_info(ctrlErr, sal, sar);
            }
        }
    }
    catch (exception& ex)
    {
        cout << "Error occurred: " << ex.what() << endl;
        ret = -1;
    }

    return ret;
}
