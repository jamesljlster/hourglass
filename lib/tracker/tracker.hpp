#ifndef __TRACKER_HPP__
#define __TRACKER_HPP__

#include <fstream>
#include <string>

#include <SPID.h>
#include <lstm.h>
#include <trasvc.h>
#include <wclt.h>
#include <ftsvc.hpp>

class Tracker
{
   public:
    // Member variables
    int stop;

    // Member functions
    bool init(int argc, char* argv[]);
    void cleanup();

    bool svc_connect();
    bool svc_disconnect();

   protected:
    // Member class
    class TkrNet
    {
       public:
        std::string ip;
        int port;
    };

    // Member variables
    hourglass::ftsvc ft;
    std::fstream fLog;

    TkrNet wsvr;
    TkrNet trasvr;

    lstm_t model;
    SPID sPid;

    int wcltStatus;
    wclt_t wclt;

    int tsStatus;
    trasvc_client_t ts;
};

#endif
