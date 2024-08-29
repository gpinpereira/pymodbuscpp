/**
 * @file string_.h
 */

#ifndef _STRING_ //############################################################
#define _STRING_

#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <limits>
#include <stdarg.h>

#include "exception_.h"
#include "random_.h"
#include "math_.h"
#include "time_.h"

typedef std::vector<std::string> cStringList;

//! Converts an enum case to its string representation. If the enum is
//! represented by a an object (e.g. variable) the representation will be the
//! same as in 'cNameToStr', i.e. it will represent the variable name instead.
//! e.g.: enum DIRECTION{NORTH,SOUTH}; std::cout << cEnumToStr(NORTH);
#define cEnumToStr(ENUM_CASE) std::string(#ENUM_CASE)
//! Converts an variable name to its string representation.
//! e.g.: double b; std::cout << cNameToStr(NORTH);
#define cNameToStr(VAR_NAME) std::string(#VAR_NAME)

namespace CUTIL {

//! cCenter(T,s,p)  : Centers 'T' padding text with 'p' up to 's' chars.
//! cConcate(L,s)   : concatenate a list of string 'l' using 's' as separator
//! cDateToStr()    : Returns a string representation of the current date/time.
//! cDateToStr(t)   : Returns a string representation 't'
//! cDateToStr(f)   : Returns a string representation accordingly to 'f' of current time.
//! cDateStrtoTime(t,f,l): Converts to time the string 't' formatted sccordingly to 'f' (using daylight 'd').
//! cHasOnlySpaces(S): Checks if 'S' is empty or contains only white space characters.
//! cLeft(T,s,p)    : Left aligns 'T' padding text with 'p' up to 's' chars.
//! cLowerCase(s)   : Convert 's' into lower case characters,
//! cPressAnyKey()  : Wait for the user to press a key.
//! cReadPassword() : Reads a password from the std:in.
//! cRandomString(s,n,A,a): Generates a randon string of size 's' containning numeric(n)/upper case(A)/alpha(a) chars.
//! cReplace(T,w,b) : Finds all the occurrences of 'w' in 'T' and replaces each of them by 'b'.
//! cReplace(i,o,w) : Finds all the occurrences of 'w' in file 'i' and outputs the result to file 'o';
//! cRight(T,s,p)   : Right aligns 'T' padding text eith 'p' up to 's' chars.
//! cSeparateValues(k,T,s,e):  Separates 'T' by 's' excluding '[e]s' and return then using 'k'.
//! cSeparateValues(T,s,e,n,...): Separates 'T' by 's' excluding '[s]s' and feeds it to the list of 'n' args.
//! cString(v,p)    : Converts 'v' (with a numeric precision of 'p') to string its string representation.
//! cString(V,s,)   : Converts vector elements 'V' (with a numeric precision of 'p') to a string separated by 's'.
//! cString(Cs,Rs,n,...): Converts a list of vectors/buffers into a text table representation.
//! cUpperCase(s)   : Convert 's' into upper case characters.
//! cValue(T):      : Reads text 'T' into its numeric format.
//! cValues(F,T,n,...): Get 'n' parameters from 'T' formated accordingly to 'F'.
//! operator<<(S, T): Similar to cString(v);


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

std::string cConcate(cStringList&, std::string sep="");
std::string cDateToStr();
std::string cDateToStr(time_t&);
std::string cDateToStr(std::string);
time_t cDateStrtoTime(std::string, std::string, int isDayLighSaving=0);
bool cHasOnlySpaces(std::string &);
int cPressAnyKey(bool info=false);
std::string cReadPassword(bool newline=false);
std::string cRandomString(unsigned, bool, bool, bool);
std::string cReplace(std::string, std::string, std::string);
void cReplace(std::string infile, std::string outfile, const std::vector<std::pair<std::string,std::string>> &what);
void cSeparateValues(std::vector<std::string>&, std::string&, std::string, std::string no="");
unsigned  cSeparateValues(std::string&, std:: string, std:: string, unsigned,...);

/*===========================================================================*/
//! Write 'arg' into a string with 'length' shifting text to center.
//! If 'length<0' the text is truncated accordingly to the aligment,
//! otherwise it is completely filled with '#'
template <class Type> std::string cCenter(Type arg, int length, char leading){
unsigned cLength=CMATH::cAbs(length); std::string text; text << arg;
 if (text.length()>cLength){ // no need to insert 'leading'
  if (length>0){ text=""; leading='#'; } // cannot be completely written!
  else return text.substr((text.length()-cLength)/2,cLength);
 } //..........................................................................
 text.insert(0,(cLength-text.length())>>1,leading);
 text.insert(text.length(),cLength-text.length(),leading);
 return text;
}

/*===========================================================================*/
//! Write 'arg' into a string with 'length' shifting text left.
//! If 'length<0' the text is truncated accordingly to the aligment,
//! otherwise it is completely filled with '#'
template <class Type> std::string cLeft(Type arg, int length, char leading){
unsigned cLength=CMATH::cAbs(length); std::string text; text << arg;
 if (text.length()>cLength){ // no need to insert 'leading'
  if (length>0){ text=""; leading='#'; } // cannot be completely written!
  else return text.substr(0,cLength);
 } //..........................................................................
 text.insert(text.length(),cLength-text.length(),leading);
 return text;
}

/*===========================================================================*/
//! Write 'arg' into a string with 'length' shifting text right.
//! If 'length<0' the text is truncated accordingly to the aligment,
//! otherwise it is completely filled with '#'
template <class Type> std::string cRight(Type arg, int length, char leading){
unsigned cLength=CMATH::cAbs(length); std::string text; text << arg;
 if (text.length()>cLength){ // no need to insert 'leading'
  if (length>0){ text=""; leading='#'; } // cannot be completely written!
  else return text.substr(text.length()-cLength,cLength);
 } //..........................................................................
 text.insert(0,cLength-text.length(),leading);
 return text;
}

/*===========================================================================*/
/// Writes to 'std::string' through 'std::ostringstream'.
/// 'prec=0': default precision; 'prec<0': maximum precision for 'Type'
template <class Type> std::string cString(Type Data, int prec=0){
std::ostringstream out; // Output stream class to operate on strings.
 if (prec<0) out << std::setprecision(std::numeric_limits<Type>::digits10);
 else if (prec>0) out << std::setprecision(prec); // else use default value.
 out << Data; return out.str();
}

/*===========================================================================*/
/// 'prec=0': default precision; 'prec<0': maximum precision for 'Type'
template <class Type> std::string cString(std::vector<Type> &Data, std::string C, int prec=0){
std::ostringstream out; unsigned last=Data.size()-1, idx=0;
 if (prec<0) out << std::setprecision(std::numeric_limits<Type>::digits10);
 else if (prec>0) out << std::setprecision(prec); // else use default value.
 for (; idx<Data.size(); ++idx) if (idx<last) out << Data[idx] << C;
  else out << Data[idx]; return out.str();
}

/*===========================================================================*/
//! Output a list of (pointers to) vectors/cBuffers in the table format.
//! Columns are separated by 'sep_C' and row by 'sep_R'. For instance:
//! ** cout << cString<vector<double>*>("\t","\n",2,&aa,&bb); // or
//! ** cout << cString<cBuffer<double>*>("\t","\n",2,&aa,&bb);
template <class Type> std::string cString(std::string sep_C, std::string sep_R, unsigned nV,...){
va_list vList; unsigned iV, size; std::vector<Type> list; std::string output;
 for (list.reserve(nV), va_start(vList,nV), iV=0; iV<nV; ++iV)
  list.push_back(va_arg(vList,Type)); va_end(vList); // Load list columns.
 for (size=list[0]->size(), iV=1; iV<nV; ++iV) if (size!=list[iV]->size()) throw
  CEXCP::Exception("Invalid Operation",__FUNCTION__,"Check Columns size");
 for (unsigned iD=0; iD<size; ++iD) for (iV=0; iV<nV; ++iV){
  output << (*list[iV])[iD] << (iV<nV-1?sep_C:(iD<size-1?sep_R:""));
 } return output;
}

/*===========================================================================*/
//! Writes to 'string' through 'ostringstream'. Use 'stringstream'
//! instead of 'std::string' to read more than one field.
template <class inType> std::string &operator<<(std::string &str, inType Data){
std::ostringstream out; out << Data; return str+=out.str();
}

/*=============================================================================*/
//! Converts text into type. Throws if text is not a valid representation
//! for type accordingly to 'istringstream': not that cValue will search for
//! non space chars following the end of a valid conversion.
template <class Type> Type cValue(std::string text){
Type value; std::istringstream in(text); in >> value;
 if (in.fail()) throw CEXCP::Exception("Invalid Conversion",__FUNCTION__,text);
 else while (!in.eof()) if (!std::isspace(in.get()) && !in.eof()) // extra ch.
  throw CEXCP::Exception("Invalid Convertion", __FUNCTION__,text);
 return value; // success.
}

/*===========================================================================*/
//! Alias to sscanf (vsscanf) which will through in case of failure
//! (See also cValues defined in tokenParser_.h)
void cValues(std::string format, std::string from, unsigned n, ...);

/*===========================================================================*/
//! convert std::string to lower case.
std::string cLowerCase(std::string source);

/*===========================================================================*/
//! convert std::string to upper case.
std::string cUpperCase(std::string source);

}

#endif // _STRING_ ############################################################ 
