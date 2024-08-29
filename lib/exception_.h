/**
 * @file exception_.h
 */

#ifndef _EXCEPTION_ //#########################################################
#define _EXCEPTION_

#include <string>
#include <iostream>
#include <typeinfo>
#include <cstdlib>
#include <cxxabi.h>

namespace CEXCP {

#define THIS (*this) // easy operators manipulation on 'this' context.

class Exception;

template <class T> std::string cTypeID();
template <class T> std::string cTypeID(std::string);
template <class T> std::string cTypeID(T&, std::string);
template <class T> std::string cNameID(std::string);

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                   Exception                               */
/*! \author Francisco Neves                                                  */
/*! \date 2006.06.11 ( Last modified 2019.02.20 )                            */
/*! \brief Error Manipulation/handling                                       */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/// \details
/// Exception handling for personnal library objects/code.
/// <BR><BR><B>Example:<BR></B> @include Exception.cs
class Exception{
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    std::string FType, FOrigin, FComment;
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    Exception(std::string sType, std::string sOrigin, std::string sComment);
    Exception(const Exception &Arg);
    inline std::string Comment(){ return FComment; } ///< Error description.
    inline std::string Origin(){ return FOrigin; } ///< Error origin.
    inline std::string Type(){ return FType; } ///< Error type.
    std::string Message(); ///< "ERROR: Type (Comment) @ Origin"
    //.........................................................................
    friend inline std::ostream& operator<<(std::ostream& os, Exception& Arg){
      os << Arg.Message(); return os; } ///< "ERROR: Type (Comment) @ Origin"
    static std::ostream *debug; // set to output exceptions.
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
/// \brief Error string manipulation
/// \author Francisco Neves \date 2014.09.16 ( Last modified 2016.10.19 )
/// \details Creates a string corresponding to 'Type'.
template <class Type> inline std::string cTypeID(){ int status;
char *realname = abi::__cxa_demangle(typeid(Type).name(),0,0,&status);
std::string res(realname); free(realname); return res;
}

/*===========================================================================*/
/// \brief Error string manipulation
/// \author Francisco Neves \date 2014.09.16 ( Last modified 2016.10.19 )
/// \details Creates a string corresponding to 'Type::fName'.
/// <BR>Check '\_\_FUNCTION\_\_' for the name of the current function ('fName').
template <class Type> inline std::string cTypeID(std::string fName){
 return cTypeID<Type>()+"::"+fName;
}

/*===========================================================================*/
/// \brief Error string manipulation
/// \author Francisco Neves \date 2014.09.16 ( Last modified 2016.10.19 )
/// \details Creates a string corresponding to 'Type::fName'.
/// <BR>Check '\_\_FUNCTION\_\_' for the name of the current function ('fName').
template <class Type> std::string cTypeID(Type& /*Obj*/, std::string fName){
 return cTypeID<Type>()+"::"+fName;
}

/*===========================================================================*/
/// \brief Error string manipulation
/// \author Francisco Neves \date 2014.09.16 ( Last modified 2014.09.16 )
/// \details Replace all the occurrences of "%T" in 'f' by the corresponding
/// name of 'Type'. \see CUTIL::cReplace (after gcc4.7 the C++ compiler no
/// longer performs some extra unqualified lookups it had performed in the past).
template <class Type> inline std::string cNameID(std::string f){
std::string what="%T", with=cTypeID<Type>(); std::string::size_type pos = 0;
 while((pos=f.find(what,pos)) != std::string::npos){
  f.replace(pos,what.length(),with); pos+=with.length(); }
 return f;
}

} // end CEXP

#endif //######################################################################
