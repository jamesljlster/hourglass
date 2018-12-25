#include "tracker.hpp"
#include "tkr_private.hpp"

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

bool Tracker::svc_connect()
{
    int ret;

    // Connect wheel service
    __run_chk(wclt_connect(&this->wclt, this->wsvr.ip.c_str(), this->wsvr.port),
              ret, RET);
    this->wcltStatus = 1;

    if (this->ctrlMethod == Tracker::TKR_CTRL_METHOD::TKR_CTRL_METHOD_LEARN)
    {
        // Connect training service
        __ts_run_chk(trasvc_client_connect(&this->ts, this->trasvr.ip.c_str(),
                                           this->trasvr.port),
                     ret, ERR);
        this->tsStatus = 1;

        // Clear training data
        __ts_run_chk(trasvc_client_clear(this->ts), ret, ERR);

        // Upload lstm base model
        __ts_run_chk(trasvc_client_model_upload(
                         this->ts, this->learnArg.modelBase.c_str()),
                     ret, ERR);

        // Start training service
        __ts_run_chk(trasvc_client_start(this->ts), ret, ERR);
    }

ERR:
    this->svc_disconnect();

RET:
    return (ret >= 0);
}

void Tracker::svc_disconnect()
{
    if (this->wcltStatus)
    {
        wclt_disconnect(this->wclt);
        this->wcltStatus = 0;
    }

    if (this->tsStatus)
    {
        trasvc_client_disconnect(this->ts);
        this->tsStatus = 0;
    }
}

}  // namespace hourglass
