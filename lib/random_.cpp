#include "random_.h"

using namespace CEXCP;

namespace CMATH {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                  cRandom                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
/// Returns a uniform random deviate ]0,1[
/// (exclusion of the endpoint values).
double cRandom::FRandom(void){ int j; double temp;
 FSeedEvent(); FSeed2Event();
 j=iy/NDIV; iy=iv[j]-FSeed2; iv[j] = FSeed;
 if (iy < 1) iy += IMM1;
 if ((temp=AM*iy) > RNMX) return RNMX; else return temp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// Shuffling procedure used to break up sequential correlations
// in the Minimal Standard generator. Load the shue table
// (after 8 warm-ups).
void cRandom::FInitialize(long Seed){ int j;
 if ((FSeed=Seed)<1) FSeed=1;
 for (FSeed2=FSeed, j=NTAB+7; j>=0; j--){
  FSeedEvent();
  if (j < NTAB) iv[j] = FSeed;
 } iy=iv[0];
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// Compute 'FSeed' without overflows by Schrage's method.
void cRandom::FSeedEvent(void){ long k;
 k=FSeed/IQ1; FSeed=IA1*(FSeed-k*IQ1)-k*IR1;
 if (FSeed < 0) FSeed += IM1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// Compute 'FSeed2' without overflows by Schrage's method.
void cRandom::FSeed2Event(void){ long k;
 k=FSeed2/IQ2; FSeed2=IA2*(FSeed2-k*IQ2)-k*IR2;
 if (FSeed2 < 0) FSeed2 += IM2;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// log factorial function. gives natural logarithm of n!
double cRandom::FLnFac(int n) {
const int FAK_LEN=1024;
static const double        // coefficients in Stirling approximation
 C0 =  0.918938533204672722, C1 =  1./12., C3 = -1./360.;
static double fac_table[FAK_LEN]; // table of ln(n!):
static int initialized = 0;   // remember if fac_table has been initialized
 if (n < FAK_LEN) { //.............................................
  if (n <= 1) {
   if (n < 0) throw Exception("Invalid Operation","cRandom::FLnFac(int)",
    "Parameter negative in LnFac function"); return 0; }
  if (!initialized) { // first time. Must initialize table.........
   // make table of ln(n!)
   double sum = fac_table[0] = 0.;
   for (int i=1; i<FAK_LEN; i++){ sum += log(i); fac_table[i] = sum; }
   initialized = 1;
  } return fac_table[n];
 } // not found in table. use Stirling approximation............
 double  n1, r; n1 = n;  r  = 1. / n1;
 return (n1 + 0.5)*log(n1) - n1 + C0 + r*(C1 + r*r*C3);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// Generates a random variate with the poisson distribution for
// extremely low values of L. The method is a simple calculation
// of the probabilities of x = 1 and x = 2. Higher values are
// ignored. The reason for using this method is to
// avoid the numerical inaccuracies in other methods.
double cRandom::FPoissonLow(double L){ double d, r;
 d = sqrt(L);
 if (FRandom() >= d) return 0.0;
 r = FRandom() * d;
 if (r > L * (1.-L)) return 0.0;
 if (r > 0.5*L*L*(1.-L)) return 1.0;
 return 2.0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// This subfunction generates a random variate with the poisson
// distribution using inversion by the chop down method (PIN).
// Execution time grows with L. Gives overflow for L > 80. The
// value of bound must be adjusted to the maximal value of L.
double cRandom::FPoissonInver(double L){
const int bound = 130; // safety bound. Must be > L + 8*sqrt(L).
static double p_L_last = -1.; // previous value of L
static double p_f0; // value at x=0
double r, f; int x;
 if (L != p_L_last) { p_L_last = L; p_f0 = exp(-L); }
 while (1) { //...................................................
  r = FRandom();  x = 0;  f = p_f0;
  do { // recursive calculation: f(x) = f(x-1) * L / x ..........
   r -= f; if (r <= 0) return x;
   x++; f *= L; r *= x;
  } while (x <= bound);
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// This subfunction generates a random variate with the poisson
// distribution using the ratio-of-uniforms rejection method (PRUAt).
// Execution time does not depend on L, except that it matters
// whether L is within the range where ln(n!) is tabulated. Reference:
// E. Stadlober: "The ratio of uniforms approach for generating
// discrete random variates". Journal of Computational and Applied
// Mathematics, vol. 31, no. 1, 1990, pp. 181-189.
double cRandom::FPoissonRatioUniforms(double L){
const double SHAT1 = 2.943035529371538573;    // 8/e
const double SHAT2 = 0.8989161620588987408;   // 3-sqrt(12/e)
static double p_L_last = -1.0; // previous L
static double p_a; // hat center
static double p_h; // hat width
static double p_g; // ln(L)
static double p_q; // value at mode
static int p_bound; // upper bound
int mode; // mode
double u; // uniform random
double lf; // ln(f(x))
double x; // real sample
int k; // integer sample
 if (p_L_last != L) {
  p_L_last = L;  // Set-up
  p_a = L + 0.5; // hat center
  mode = (int)L; // mode
  p_g  = log(L);
  p_q = mode * p_g - FLnFac(mode); // value at mode
  p_h = sqrt(SHAT1 * (L+0.5)) + SHAT2; // hat width
  p_bound = (int)(p_a + 6.0 * p_h); // safety-bound
 }
 while(1) {
  u = FRandom();
  if (u == 0) continue; // avoid division by 0
  x = p_a + p_h * (FRandom() - 0.5) / u;
  if (x < 0 || x >= p_bound) continue; // reject if outside valid range
  k = (int)(x);
  lf = k * p_g - FLnFac(k) - p_q;
  if (lf >= u * (4.0 - u) - 3.0) break;   // quick acceptance
  if (u * (u - lf) > 1.0) continue;       // quick rejection
  if (2.0 * log(u) <= lf) break;}         // final acceptance
 return k;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// This is exercise 16 from Knuth; see page 135, and the
// solution is on page 551.
double cRandom::FGammaFrac(double a){
double p, q, x, u, v;
 p = Euler / (a + Euler);
 do {
  u = FRandom(); v = FRandom();
  if (u < p){ x=exp((1.0/a)*log(v)); q=exp(-x); }
  else{ x=1.0-log(v); q=exp((a-1.0)*log(x)); }
 } while (FRandom()>=q);
 return x;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// Works only if a > 1, and is most efficient if a is large
// This algorithm, reported in Knuth, is attributed to Ahrens. A
// faster one, we are told, can be found in: J. H. Ahrens and
// U. Dieter, Computing 12 (1974) 223-246.  */
double cRandom::FGammaLarge(double a){
double sqa=sqrt (2.0*a-1.0), x, y, v;
 do { do {
  y = tan (Pi*FRandom()); x = sqa * y + a - 1;
 } while (x <= 0.0); v = FRandom();
 } while (v>(1.0+y*y)*exp((a-1.0)*log(x/(a-1.0))-sqa*y));
 return x;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// The algorithm below is from Knuth, vol 2, 2nd ed, p. 129.
double cRandom::FGammaInt (unsigned int a){
 if (a < 12){
  unsigned int i; double prod = 1.0;
  for (i = 0; i < a; i++) prod *= FRandom();
  return -log(prod);
 } else return FGammaLarge((double)a);
}

/*===========================================================================*/
/// Initialization of 'Seed' Independent's.
cRandom::cRandom(long Seed){
 IA1=40014; IA2=40692; IQ1=53668; IQ2=52774; IR1=12211; IR2=3791; NTAB=32;
 IM1=2147483563; IM2=2147483399; IMM1=IM1-1; NDIV=1+IMM1/NTAB;
 AM=1.0/IM1; EPS=1.2e-7; RNMX=1.0-EPS;
 iv=new long[NTAB];
 FInitialize(Seed);
 Fiset=0; // See: gauss()
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cRandom::~cRandom(void){ delete [] iv; }

/*===========================================================================*/
//! This function generates a random variate with the poisson
//! distribution. Uses inversion by chop-down method for L < 17,
//! and ratio-of-uniforms method for L >= 17. For L < 1.E-6 numerical
//! inaccuracy is avoided by direct calculation.
double cRandom::poisson (double L) {
 if (L < 17) { //................................................
  if (L < 1.E-6) {
   if (L == 0) return 0;
   if (L < 0) throw Exception("Invalid Operation","TRandom::poisson(double)",
    "Parameter negative in poisson function");
// For extremely small L we calculate the probabilities of x = 1
// and x = 2 (ignoring higher x). The reason for using this method
// is to prevent numerical inaccuracies in other methods.
   return FPoissonLow(L);
  } else {
// The computation time for this method grows with L.
// Gives overflow for L > 80
   return FPoissonInver(L);
 } } else { //..................................................
  if (L > 2.E9) throw Exception("Invalid Operation","TRandom::poisson(double)",
   "Parameter too big in poisson function");
  // The computation time for this method does not depend on L.
  // Use where other methods would be slower.
   return FPoissonRatioUniforms(L);
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*! Returns a random deviate distribution: exp(-x/t).*/
double cRandom::exponential(double t){
double dum; do dum=FRandom(); while (dum == 0.0);
 return -log(dum)*t;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*! Returns a random deviate distribution: exp(((x-x0)/2*sig)).*/
double cRandom::gauss(double sig, double x0){
double fac,rsq,v1,v2;
 if (Fiset == 0){
  do {
   v1=2.0*FRandom()-1.0; v2=2.0*FRandom()-1.0;
   rsq=v1*v1+v2*v2;
  } while (rsq >= 1.0 || rsq == 0.0);
  fac=sqrt(-2.0*log(rsq)/rsq);
  Fgset=v1*fac;
  Fiset=1;
  return x0+v2*fac*sig;
 } else {
  Fiset=0;
  return x0+Fgset*sig;
 }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*! Return a 'cos' uniform random deviate distribution.*/
double cRandom::cos(double Tmin, double Tmax){
double cTmax=cos(Tmax), ct=cTmax+(cos(Tmin)-cTmax)*FRandom(); return acos(ct);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! The Gamma distribution of order a>0 is defined by:
//! p(x) dx = {1 / Gamma(a) b^a } x^{a-1} e^{-x/b} dx
//! \see: 'polya' distribution.
double cRandom::gamma (double a, double b){
unsigned int na = (unsigned)floor (a); // assume a > 0
 if (a == na) return b*FGammaInt(na);
 else if (na == 0) return b*FGammaFrac(a);
 else return b*(FGammaInt(na) + FGammaFrac(a - na) );
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*! detail Draw a random number from a Polya distribution.*/
double cRandom::polya(double Mean, double Sigma){
 return gamma(cSqr(Mean/Sigma),cSqr(Sigma)/Mean);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! uniform random distribution inside a unit radius circle.
//! (x=r*cos(theta)*R ; y=r*sin(theta)*R)
void cRandom::circle(double *r, double *theta){
 *r=sqrt(random()); *theta=random()*twoPi;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! uniform random in ]a,b[
double cRandom::random(double a, double b){
 return a+(b-a)*FRandom();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! uniform random in the domain ]a,b[ in a log10-scale axis.
double cRandom::random_log10(double a, double b){
double logA=log10(a); // log is expensive ...
return pow(10,logA+(log10(b)-logA)*FRandom());
}

}
 
