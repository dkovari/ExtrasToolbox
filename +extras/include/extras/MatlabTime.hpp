#pragma once

#include <ctime>
#include <chrono>
#include <string>
#include <cmath>

//#include<mex.h>

namespace extras{
    double matlab_now(){
        std::chrono::time_point<std::chrono::system_clock> p1, p2;
        std::time_t epoch_time = std::chrono::system_clock::to_time_t(p1);
        p2 = std::chrono::system_clock::now();
        std::tm* epoch_tm = std::localtime(&epoch_time);

        //out = epoch time in matlab format


        double y_2001 = epoch_tm->tm_year - 101; //years since 2001
        double out = double(
              693961 //days from 0->1900
            + 36524+366 // days from 1900->2001
            + y_2001*365 + floor(y_2001/4.0) - floor(y_2001/100.0) + floor(y_2001/400.0) // days from 2001->start of current year
            + ((double)epoch_tm->tm_yday) + 1 //days since jan 1, (including today)
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

    template <class durationType>
    double matlab_datenum(std::chrono::time_point < std::chrono::system_clock, durationType> tp) {
        std::chrono::time_point < std::chrono::system_clock, durationType> p0;
        std::time_t epoch_time = std::chrono::system_clock::to_time_t(p0);

        std::tm* epoch_tm = std::localtime(&epoch_time);
        double y_2001 = epoch_tm->tm_year - 101; //years since 2001
        double out = double(
            693961 //days from 0->1900
            + 36524 + 366 // days from 1900->2001
            + y_2001 * 365 + floor(y_2001 / 4.0) - floor(y_2001 / 100.0) + floor(y_2001 / 400.0) // days from 2001->start of current year
            + ((double)epoch_tm->tm_yday) + 1 //days since jan 1, (including today)
            + ((double)epoch_tm->tm_hour) / 24.0
            + ((double)epoch_tm->tm_min) / (24.0 * 60.0)
            + ((double)epoch_tm->tm_sec) / (24 * 3600)
            );
        //add milliseconds since epoch
        out += double(
            std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count()
            ) / 1000 / (24 * 3600);

        return out;
    }

    // Representation of standard (Gregorian) Date in Common Era (C.E. aka AD)
    class DateCE{
    protected:
        int _year; //year in the Common era
        int _month; //month [1-12]
        int _mday; //day in the month (starting at 1)
        int _wday; //day of the week (starting at 0)
        int _yday; //day in the year (starting a 0)
        double _sec; //seconds since start of min [0-59.99999...]
        int _hour; // _hour in day [0-23]
        int _min; // min in hour [0-59]

        void from_tm(const std::tm& _tm){
            _year = _tm.tm_year+1900;
            _month = _tm.tm_mon+1;
            _mday = _tm.tm_mday;
            _wday = _tm.tm_wday;
            _yday = _tm.tm_yday;
            _hour = _tm.tm_hour;
            _min = _tm.tm_min;
            _sec = _tm.tm_sec;
        }
    public:
        //Construct from standard posix time_t
        DateCE(const time_t& Time){
            from_tm(*std::localtime(&Time));
        }

        // Create DateCE structure from MATLAB datenum
        DateCE(double datenum){

            //mexPrintf("datenum: %g\n",datenum);

            int year = floor((datenum-1.0)/(365.0+97.0/400.0));
            //mexPrintf("year: %04d\n",year);
            // days to start of this year
            double d2 = 1+(year)*365 + floor((year-1)/4) - floor((year-1)/100.0) + floor((year-1)/400.0);

            //mexPrintf("days to start of year: %g\n",d2);

            double days_this_year = datenum-d2;

            //mexPrintf("days_this_year: %g\n",days_this_year);

            // Convert time to second-level precision
            struct tm tmp_tm;
            memset(&tmp_tm, '\0', sizeof(struct tm));
            tmp_tm.tm_year = year-1900;
            time_t time= std::mktime(&tmp_tm);
            time+=days_this_year*24*3600;

            from_tm(*std::localtime(&time));

            // add sub-second precision
            double res;
            double frac = std::modf(datenum, &res); //get frac of this day
            frac = std::modf(frac*24,&res); //get frac of this hour
            frac = std::modf(frac*3600,&res); //get frac of this second;
            _sec+=frac;
        }

        int year() const{return _year;} ///return year [####] (CE)
        int month() const{ return _month;} //return month [1-12]
        int mday() const{ return _mday;} // return day of month [1-...];
        int wday() const{ return _wday;} // return day of week [0-6];
        int yday() const{ return _yday;} //return day of year [0-364/365];
        double sec() const{ return _sec;} //return second after minute [0-59.9999...]
        int hour() const{ return _hour;} //return hour of day [0-23]
        int min() const{ return _min;} //return min after hour [0-59]
    };
}
