#include "time_.h"
#include "constanst_.h"
#include <sstream>
#include <cmath>

using namespace std;
using namespace CMATH;
using std::chrono::system_clock;

namespace CUTIL{

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cLoopTimer                                 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifdef CTHREAD_ENABLE //#######################################################

/*===========================================================================*/
cLoopTimer::cLoopTimer(timespec dt_):Fdt(dt_){ }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cLoopTimer::cLoopTimer(double dt_):Fdt(cTimeSpec(dt_)){ }

/*===========================================================================*/
//! If 'skip==true' and 'FEnd' has expired, then it waits until the next
//! multiple of 'Fdt' which is not less than now. If 'skip==false' and now
//! has expired, than it exits without waiting but 'FEnd' is only incremented
//! by 'Fdt', which can cause many exits which do not respect 'Fdt' once the
//! loop takes less time than 'Fdt' and 'FEnd' catches up with now.
bool cLoopTimer::waitForTimer(bool skip){
 base_cond::lock(); //#####################################################+1+2
 if (base_cond::operator*()){ base_cond::unlock(); return true; } //#########-1
 else { // not stopped ........................................................
  FEnd+=Fdt; if (skip){ Ft=CUTIL::cTimeSpec(); while (Ft>FEnd) FEnd+=Fdt ; }
  base_cond::waitForSignal(FEnd); FStopped=base_cond::operator*();
  base_cond::unlock(); //####################################################-2
  return FStopped;
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cLoopTimer::stopTimer(){
 base_cond::lock(); base_cond::sendSignal(); base_cond::unlock(); //#######+1-1
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cLoopTimer::reset(timespec dt_){ Fdt=dt_; reset(); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cLoopTimer::reset(double dt_){ Fdt=cTimeSpec(dt_); reset(); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cLoopTimer::reset(){
 base_cond::operator*()=false; FEnd=CUTIL::cTimeSpec();
}

#endif // CTHREAD_ENABLE ######################################################

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
//! Gets how many hours (H). minutes (M) and seconds (S) t (in sec) is.
void cHMSTime(double time, unsigned &H, unsigned &M, unsigned &S){
 if (time>=3600) time=time-(H=time/3600)*3600; else H=0;
 if (time>=60) time=time-(M=time/60)*60; else M=0;
 S=time;
}

/*===========================================================================*/
//! cHMSTimeStr(t,f) : Converts t (sec) to string ("H hour M min S s").
string cHMSTimeStr(double time){ ostringstream res;
unsigned H, M, S, i=0; cHMSTime(time,H,M,S);
 if (H>0){ res << H << " hour"; ++i; }
 if (M>0){ res << (i>0?" ":"") << M << " min"; ++i; }
 if (S>0){ res << (i>0?" ":"") << S << " s"; ++i; }
 if (i==0) res << "0 s"; return res.str();
}

/*===========================================================================*/
bool operator>(timespec &left, timespec &right){
 if (left.tv_sec>right.tv_sec) return true;
 else if (left.tv_sec<right.tv_sec) return false;
 else if (left.tv_nsec>right.tv_nsec) return true;
 else return false;
}

/*===========================================================================*/
timespec &operator+=(timespec &left, timespec &right){
 left.tv_sec+=right.tv_sec; left.tv_nsec+=right.tv_nsec;
 if (left.tv_nsec>=Us_ns){ ++left.tv_sec; left.tv_nsec-=Us_ns; }
 return left;
}

/*===========================================================================*/
timespec operator+(timespec &left, timespec &right){ timespec sum;
 sum.tv_sec=left.tv_sec+right.tv_sec; sum.tv_nsec=left.tv_nsec+right.tv_nsec;
 if (sum.tv_nsec>=Us_ns){ ++sum.tv_sec; sum.tv_nsec-=Us_ns; }
 return sum;
}

/*===========================================================================*/
//! 'left'>'right' must be true (user must check that explicitly) since
//! timespec cannot represent negative times/values.
timespec operator-(timespec &left, timespec &right){ timespec diff;
 if ((left.tv_nsec-right.tv_nsec)<0){
  diff.tv_sec = left.tv_sec-right.tv_sec-1;
  diff.tv_nsec = left.tv_nsec+Us_ns-right.tv_nsec;
 } else { //...................................................................
  diff.tv_sec = left.tv_sec-right.tv_sec;
  diff.tv_nsec = left.tv_nsec-right.tv_nsec;
 } return diff;
}

/*===========================================================================*/
//! convert 'timespec' to seconds (ns resolution).
float cSeconds(const timespec &t){
 return float(t.tv_sec)+float(t.tv_nsec)*1E-9;
}

/*===========================================================================*/
//! convert seconds to timespec ('s' must be non-negative).
timespec cTimeSpec(const double s){
double int_, frac_=modf(s,&int_);
 return {(time_t)(int_),(long)(frac_*Us_ns)};
}

/*===========================================================================*/
//! Get current time (CLOCK_REALTIME, CLOCK_MONOTONIC, ...).
timespec cTimeSpec(clockid_t c){
timespec t; clock_gettime(c,&t); return t;
}

/*===========================================================================*/
//! Convert 'timespec' to string ("X s Y ns").
string cString(timespec &val){
ostringstream out; out << val.tv_sec << " s " << val.tv_nsec << " ns";
return out.str();
}

/*===========================================================================*/
//! Gets current (now) local time.
std::tm cCurrentTime(){
auto now = system_clock::now();
std::time_t now_ts = system_clock::to_time_t(now);
 return *std::localtime(&now_ts);
}

/*===========================================================================*/
// Gets current (now) local time string ('tf' format).
std::string cCurrentTimeStr(std::string tf){
std::tm now_out = cCurrentTime(); std::stringstream ss;
 ss << std::put_time(&now_out,tf.c_str());
 return ss.str();
}

/*===========================================================================*/
//! Returns the linux time (in seconds) corresponding to 'when' ('tf' format).
//! if "%Z" is missing, UTC is assumed (see also 'time').
time_t cTime(std::string when, std::string tf){
std::tm tm = {}; std::stringstream ss(when);
 ss >> std::get_time(&tm,tf.c_str());
 if (!ss.fail()) return std::mktime(&tm);
 else throw CEXCP::Exception("Invalid Time Format",__FUNCTION__,when);
}

/*===========================================================================*/
//! Returns the tm structure corresponding to 'when' ('tf' format).
//! if "%Z" is missing, UTC is assumed (see also 'time').
std::tm cTime_tm(std::string when, std::string tf){
std::tm tm = {}; std::stringstream ss(when);
 ss >> std::get_time(&tm,tf.c_str());
 if (!ss.fail()) return tm; else throw CEXCP::Exception
  ("Invalid Time Format",__FUNCTION__,when);
}

/*===========================================================================*/
//! Returns time ('tf' format) of 'when' in seconds.
std::string cTimeStr(time_t when, std::string tf){
auto const tm = *std::localtime(&when);
std::ostringstream os; os << std::put_time(&tm,tf.c_str());
 return os.str();
}

/*===========================================================================*/
//! ** Returns the difference in seconds from UTC relative to 'when' (seconds)
//!   in LOCAL TIME -- use cTimeZoneOffset(std::time(nullptr)) for now.
//! ** Example:
//!   string tf=ft="%Y-%m-%d %H:%M:%S";
//!   string date=cCurrentTimeStr(tf);
//!   time_t user_ts=cTime(date,tf);
//!   user_ts-=cTimeZoneOffset(date,tf);
//! ** Then, user_ts-time(NULL) ==0.
long cTimeZoneOffset(time_t when){
auto const tm=*std::localtime(&when);
std::ostringstream os; os << std::put_time(&tm, "%z");
std::string s=os.str(); // s is in ISO 8601 format: "±HHMM"
int h=std::stoi(s.substr(0,3), nullptr, 10);
int m=std::stoi(s.substr(3,2), nullptr, 10);
 return h*3600+m*60;
}

/*===========================================================================*/
//! ** Returns the difference in seconds from UTC relative to 'when' ('tf' format)
//! See: cTimeZoneOffset(time_t) (if "%Z" is missing, UTC is assumed).
long cTimeZoneOffset(std::string when, std::string tf){
 return cTimeZoneOffset(cTime(when,tf));
}

}
