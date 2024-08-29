#include "exception_.h"

using namespace std;

namespace CEXCP {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                   Exception                               */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
ostream *Exception::debug=NULL;

/*===========================================================================*/
/// Constructor: ::Message() => "ERROR: Type (Comment) @ Origin".
Exception::Exception(string sType, string sOrigin, string sComment){
 FType=sType; FOrigin=sOrigin; FComment=sComment;
 //............................................................................
 if (debug) (*debug) << Message() << endl << endl << flush;

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// Copy Constructor.
Exception::Exception(const Exception &Arg){
 FType=Arg.FType; FOrigin=Arg.FOrigin; FComment=Arg.FComment;
 //............................................................................
 if (debug) (*debug) << Message() << endl << endl << flush;
}

/*===========================================================================*/
/// Compose error message: "ERROR: Type (Comment) @ Origin".
string Exception::Message(){
 return "ERROR: "+FType+" ("+FComment+") @ '"+FOrigin+"'.";
}

} // end CEXP.
