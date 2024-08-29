#include "math_.h"

namespace CMATH {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
// Normal distribution with mean?'mean', sigma='sigma' and area=1 (-inf,inf).
// See also 'cERF::normal_acceptance()', 'cERF::normal_wings()' and
// 'cERF::normal_cdf()'.
double cNormal(double mean, double sigma, double x){
 return expl(-0.5*cSqr((x-mean)/sigma))/(sqrt(twoPi)*sigma);
}

}
