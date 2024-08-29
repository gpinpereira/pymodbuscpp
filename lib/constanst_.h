/**
 * @file constant_.h
 */

#ifndef _CONSTANTS_ //#########################################################
#define _CONSTANTS_

namespace  CMATH {

const double Euler=             2.718281828459; /* exp(1) */
const double cEulerMascheroni=  0.577215664901532860;
const double Plank=             4.13570e-21; /* H_Plank - MeV.s */
const double LightSpeed=        299792458; /* m/s */
const double Pi=                3.1415926535897932384626433832795;
const double halfPi=            1.5707963267948966192313216916397;
const double twoPi=             6.2831853071795864769252867665590;
const double Rad=               0.0174532925199432957692369076848; /* Deg to Rad (mult fact) */
const double Deg=               57.295779513082320876798154814105; /* Rad to Deg (mult fact) */
const double Uns_ns=            1; /* ns in ns */
const double Uus_ns=            1000; /* us in ns */
const double Ums_ns=            1000000; /* ms in ns */
const double Us_ns=             1000000000; /* s in ns */
const double Uns_s=             1000000000; /* ns in s */
const double Uus_s=             1000000; /* us in s */
const double Ums_s=             1000; /* ms in s */
const double Us_s=              1; /* s in s */

const double  NORMDIST_1SIG=    0.682689492137; // [mu-1sig ; mu+1sig]
const double  NORMDIST_2SIG=    0.954499736104; // [mu-2sig ; mu+2sig]
const double  NORMDIST_3SIG=    0.997300203937; // [mu-3sig ; mu+3sig]
const double  NORMDIST_4SIG=    0.999936657516; // [mu-4sig ; mu+4sig]
const double  NORMDIST_5SIG=    0.999999426697; // [mu-5sig ; mu+5sig]
const double  NORMDIST_6SIG=    0.999999998027; // [mu-6sig ; mu+6sig]

}

#define PROPERTY_R(TYPE,NAME,VAR) inline TYPE NAME(){ return VAR; }
#define PROPERTY_W(TYPE,NAME,VAR) inline void NAME(TYPE V){ VAR=V; }
#define PROPERTY_RW(T,N,VAR) PROPERTY_R(T,N,VAR) PROPERTY_W(T,N,VAR)
#define PROPERTY_PR(TYPE,NAME,VAR) inline TYPE &NAME(){ return *VAR; }
#define PROPERTY_PW(TYPE,NAME,VAR) inline void NAME(TYPE &V){ VAR=&V; }
#define PROPERTY_PRW(T,N,VAR) PROPERTY_PR(T,N,VAR) PROPERTY_PW(T,N,VAR)

#endif // _CONSTANTS_ #########################################################
