#pragma once

#include <chrono>
namespace extras{
    double matlab_now(){

        std::chrono::time_point<std::chrono::system_clock> p1, p2;
        std::time_t epoch_time = std::chrono::system_clock::to_time_t(p1);
        p2 = std::chrono::system_clock::now();
        std::tm* epoch_tm = std::localtime(&epoch_time);

        //out = epoch time in matlab format
        double out = double(
            365*(epoch_tm->tm_year+1900)
            + epoch_tm->tm_yday
            + ((double)epoch_tm->tm_hour)/24.0
            + ((double)epoch_tm->tm_min)/(24.0*60.0)
            + ((double)epoch_tm->tm_sec)/(24*3600)
        );

        //add milliseconds since epoch
        out += double(
            std::chrono::duration_cast<std::chrono::milliseconds>(p2.time_since_epoch()).count()
    )/1000/(24*3600);

    return out;

    }
}
