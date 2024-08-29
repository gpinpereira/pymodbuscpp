/**
 * @file time_.h
 */

#ifndef _TIME_ //##############################################################
#define _TIME_

#include <string>
#include <ctime>
#include <chrono>
#include <iomanip>

#include "exception_.h"

#ifdef CTHREAD_ENABLE //#######################################################
#include "mutex_.h"
#endif // CTHREAD_ENABLE ######################################################

namespace CUTIL{
    
//! ctTime      : Measure time between events.
//! cLoopTimer  : timed loop iterations
//! -----------------------------------
//! cHMSTime(t,H,M,S) : Gets how many hours (H). minutes (M) and seconds (S) t (in sec) is.
//! cHMSTimeStr(t,f) : Converts t (sec) to string (H hour M min S s).
//! -----------------------------------
//! cSeconds(t) : convert 't' (timespec) to seconds (ns resolution)
//! cTimeSpec(s): Convert 's' (seconds) to timespec
//! cTimeSpec(c): Get current timespect (c is type of click).
//! cString(t)  : convert 't' (timespec) to string ("X s Y ns").
//! -----------------------------------
//! ** Check also string_h for more date related functions
//! cCurrentTime()     : gets local time.
//! cCurrentTimeStr(f) : Gets local time (now) string in 'f' format.
//! cTime(w,f)         : Gets time in secs corresponding to string 'w' in 'f' format.
//! cTime_tm(w,f)      : Gets time in std::tm corrresponding to string 'w' in 'f' format.
//! cTimeStr(w,f)      : Gets a time str in 'f' format for the time 'w' (in secs).
//! cTimeZoneOffset(w) : Gets the time offset between 'w' in sec and the local time.
//! cTimeZoneOffset(w) : Gets the time offset between 'w' in 'f' format and the local time.
//! -----------------------------------
//! t1>t2       : 't1' and 't2' are 'timespec'
//! t1+=t2      : 't1' and 't2' are 'timespec'
//! t1+t2       : 't1' and 't2' are 'timespec'
//! t1-t2       : 't1' and 't2' are 'timespec'

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                  ctTime                                   */
/*! \author Francisco Neves                                                  */
/*! \date 2006.04.24 ( Last modified 2013.11.15 )                            */
/*! \brief Helper to measure time between events.                            */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! 'FGetTime': return time since program started (in s).
class ctTime{
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  time_t FtStart, FtMark;
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  ctTime(void){ FtStart=FtMark=time(NULL); }
  inline time_t Mark(void){ return FtMark=time(NULL); }
  inline double tFromStart(void){ return difftime(time(NULL),FtStart); }
  inline double tFromMark(void){ return difftime(time(NULL),FtMark); }
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cLoopTimer                                 */
/*! \author Francisco Neves                                                  */
/*! \date 2020.09.03 ( Last modified 2020.09.03 )                            */
/*! \brief Timer for loop (generic)                                          */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifdef CTHREAD_ENABLE //#######################################################

class cLoopTimer: protected cConditionalWaitType<bool> {
protected: typedef cConditionalWaitType<bool> base_cond;
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    timespec Fdt, Ft, FEnd; bool FStopped;
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cLoopTimer(timespec dt_);
    explicit cLoopTimer(double dt_);
    virtual ~cLoopTimer(){ }
    //.........................................................................
    bool waitForTimer(bool skip=true); // THREAD SAFE
    void stopTimer(); // THREAD SAFE
    void reset(timespec dt_);
    void reset(double dt_);
    void reset();
};

#endif // CTHREAD_ENABLE ######################################################

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

void cHMSTime(double, unsigned&, unsigned&, unsigned&);
std::string cHMSTimeStr(double);

float cSeconds(const timespec&);
timespec cTimeSpec(const double s);
timespec cTimeSpec(clockid_t c=CLOCK_REALTIME);
std::string cString(timespec&);

std::tm cCurrentTime();
std::string cCurrentTimeStr(std::string tf="%Y-%m-%d %H:%M:%S");
time_t cTime(std::string when, std::string tf="%Y-%m-%d %H:%M:%S");
std::tm cTime_tm(std::string when, std::string tf="%Y-%m-%d %H:%M:%S");
std::string cTimeStr(time_t when, std::string tf="%Y-%m-%d %H:%M:%S");
long cTimeZoneOffset(std::string when, std::string tf="%Y-%m-%d %H:%M:%S");
long cTimeZoneOffset(time_t when);

bool operator>(timespec&, timespec&);
timespec &operator+=(timespec&, timespec&);
timespec operator+(timespec&, timespec&);
timespec operator-(timespec&, timespec&);

}

#endif // _TIME_ //############################################################
