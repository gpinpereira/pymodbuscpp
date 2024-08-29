/**
 * @file random_.h
 */

#ifndef _RANDOM_ //############################################################
#define _RANDOM_

#include "math.h"

#include "exception_.h"
#include "constanst_.h"
#include "math_.h"

namespace CMATH {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                     cRandom                               */
/*! \author Francisco Neves                                                  */
/*! \date 2006.05.12 ( Last modified 2019.03.28 )                            */
/*! \brief Random Number Generator                                           */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/// \details
/// Long period (>2e18) random number generator of L'Ecuyer with
/// Bays-Durham shuffle and added safeguards. RNMX should
/// approximate the largest foating value that is less than 1.
/// \see Numerical Recipes for: Gamma, Poisson & Binomial Deviates.
class cRandom{
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  long IM1,IM2,IMM1,NDIV;
  long IA1,IA2,IQ1,IQ2,IR1,IR2,NTAB;
  long FSeed,FSeed2,iy,*iv;
  double AM,EPS,RNMX;
  int Fiset; double Fgset; // See: gauss().
  void FInitialize(long Seed);
  double FRandom(void);
  void FSeedEvent(void);
  void FSeed2Event(void);
  double FLnFac(int n);
  double FPoissonLow(double L);
  double FPoissonInver(double L);
  double FPoissonRatioUniforms(double L);
  double FGammaFrac(double a);
  double FGammaLarge(double a);
  double FGammaInt (unsigned int a);
  cRandom(cRandom&){ } // disable copy constructor.
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  cRandom(long Seed=0);
  ~cRandom(void);
  double poisson (double L=1);
  double exponential(double t=1);
  double gauss(double sig=1, double x0=0);
  double cos(double Tmin=0, double Tmax=Pi);
  double gamma (double a=1, double b=1);
  double polya(double Mean=1, double Sigma=1);
  void circle(double *r, double *theta);
  double random(double a, double b); /*!< Return a uniform random in ]a,b[ */
  double random_log10(double a, double b); /*!< Return a uniform random in ]a,b[ in log10 */
  inline double random(){ return FRandom(); } /*!< Returns a uniform random deviate ]0,1[.*/
  inline long seed(){ return FSeed; } /*!< Returns the random seed.*/
};    
    
}

#endif // _RANDOM_ ############################################################
