/**
 * @file math_.h
 */

#ifndef _MATH_ //##############################################################
#define _MATH_

#include <math.h>
#include <stdarg.h>
#include <limits>
#include <algorithm>

#include <exception_.h>
#include "constanst_.h"

namespace CMATH {

/* Classes ==================================================================*/
template <class keyType, class valueType> class cKeyValue;

/* Distributions ============================================================*/
//! cNormal         : Normal distribution function.

/* Functions ================================================================*/
//! cAbs(t)         : Returns 't' absolut value.
//! cArea(s,x*,y*)  : ** Same as cNorm ** Returns the area of 'Y(X)' for 's' elements.
//! cContains(V,t)  : Checks 'V' (vector,cBuffer) for the 1st occurrence of 't'.
//! cEven(n)        : Check if 'n' is even/odd.
//! cMin(t1,t2)     : Returns the minimum value of {'t1','t2'}.
//! cMax(t1,t2)     : Returns the maximum value of {'t1','t2'}.
//! cMaxIndex(V,s)  : Return the index of the maximum value in an array 'V' of size 's'
//! cMinIndex(V,s,e): Return the index of the minim value in the array 'V' starting at 's' and ending at 'e'
//! cMaxIndex(V,s,e): Return the index of the maximum value in the array 'V' starting at 's' and ending at 'e'
//! cModal(d,s,D,S) : returns the 'S' modal values 'D' in a list of values 'd' with size 's'.
//! cNorm(X*,s)     : Returns the sum of 's' elements of 'X'.
//! cNorm(V&)       : Returns the sum of all elemenets in 'V' (vector,cBuffer).
//! cNorm(X*,Y*,s)  : Returns the area of 'Y(X)' for 's' elements.
//! cNorm(X&,Y&)    : Returns the area of 'Y(X)' (vector,cBuffer).
//! cNorm(X*,t,s)   : Normalize the sum of 's' elements of 'X' to 't'.
//! cNorm(V&, t)    : Normalize the sum of 'V' (vector,cBuffer) to 't'.
//! cRange(t,l,r)   : Returns 'cMin(r,cMax(t,l))'.
//! cSmoothNeighbor(l,r,I,O,s) : Smooth buffer 'I' (size 's')into 'O' using a left and right window of size 'l' and 'r', respectively.

//! cSqr(t)         : Returns the square of 't'.
//! cSwap(t1*,t1*)  : Swaps '*t1' with '*t2'.
//! cSwap(t1&,t1&)  : Swaps 't1' and 't1'

/* Statistics ==============================================================*/
//! cStatistic(D*,n,&M, &V): Calculates the mean 'M' amd RMS 'V' from 'n' elements of 'B'

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                  cKeyValue                                */
/*! \author Francisco Neves                                                  */
/*! \date 2006.03.30 ( Last modified 2017.11.27 )                            */
/*! \brief associate a key to a value                                        */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/// \details
//! Similar to 'std::pair' but uses only key at operators.
//! See sorting and search sorted algorithms.
template <class keyType, class valueType>
struct cKeyValue{
    keyType key;
    valueType value;
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cKeyValue():key(),value(){ ; }
    cKeyValue(keyType sKey):key(sKey),value(){ ; }
    cKeyValue(keyType sKey, valueType sValue):key(sKey),value(sValue){ ; }
    cKeyValue(const cKeyValue &C):key(C.key),value(C.value){ ; }
    cKeyValue &operator=(cKeyValue C){ key=C.key,value=C.value; return *this; }
    cKeyValue &operator()(keyType k, valueType v){ key=k,value=v; return *this; }
    inline valueType &operator->(){ return value; }
    bool operator==(cKeyValue &C){ return key==C.key; }
    bool operator<(cKeyValue &C){ return key<C.key; }
    bool operator>(cKeyValue &C){ return key>C.key; }
    bool operator<=(cKeyValue &C){ return key<=C.key; }
    bool operator>=(cKeyValue &C){ return key>=C.key; }
};

/*===========================================================================*/
//! Used by 'std::find' (above operatos cannot be used by 'std::sort'!)
template <class keyType, class valueType>
bool operator==(const cKeyValue<keyType,valueType> &l,
const cKeyValue<keyType,valueType> &r){ return l.key==r.key; }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Used by 'std::sort', 'std::find' (above operatos cannot be used!)
template <class keyType, class valueType>
bool operator<(const cKeyValue<keyType,valueType> &l,
const cKeyValue<keyType,valueType> &r){ return l.key<r.key; }

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
//! Returns 'arg' absolut value.
template <class Type> inline Type cAbs(Type arg){
 if (arg<0) return -arg; else return arg;
}

/*============================================================================*/
template <class iType> inline bool cEven(iType v){ return (v%2==0); }

/*===========================================================================*/
//! Returns the minimum value of 'left' and 'right'.
template <class Type> inline
Type cMin(Type left, Type right){
 if (left<right) return left; else return right;
}

/*===========================================================================*/
//! Returns the maximum value of 'left' amd 'right'.
template <class Type> inline
Type cMax(Type left, Type right){
 if (left>right) return left; else return right;
}

/*===========================================================================*/
//! Return the position of the maximum value of the array 'target' of size 'size'.
template <class Type>
unsigned cMaxIndex(Type* target, unsigned size){
Type *pMax=target, *pEnd=target+size, *pData;
for (pData=pMax+1; pData<pEnd; ++pData)
 if (*pData>*pMax) pMax=pData; return pMax-target;
}

/*===========================================================================*/
//! Return the position of the minimum value of the array 'target'
//! starting at the index 'start' and ending at 'end' (inclusive).
template <class Type>
unsigned cMinIndex(Type* target, unsigned start, unsigned end){
Type *pMin=target+start, *pEnd=target+end, *pData;
 for (pData=pMin+1; pData<=pEnd; ++pData)
  if (*pData<*pMin) pMin=pData; return pMin-target;
}

/*===========================================================================*/
//! Return the position of the maximum value of the array 'target'
//! starting at the index 'start' and ending at 'end' (inclusive).
template <class Type>
unsigned cMaxIndex(Type* target, unsigned start, unsigned end){
Type *pMax=target+start, *pEnd=target+end, *pData;
 for (pData=pMax+1; pData<=pEnd; ++pData)
  if (*pData>*pMax) pMax=pData; return pMax-target;
}

/*===========================================================================*/
//! Sums over all 'buff' array elements.
template <class Type> Type cNorm(Type* buff, unsigned size){
Type sum=0; for (unsigned i=0; i<size; ++i) sum+=buff[i];
 return sum;
}

/*===========================================================================*/
//! Sums over all 'buff' (e.g. vector, cBuffer, ...) elements.
template <template <typename ...> class Container, typename Type>
inline Type cNorm(Container<Type> &buff){
 return cNorm(buff.data(),buff.size());
}

/*===========================================================================*/
//! Integrate over Y(X).
template <class Type> Type cNorm(Type *X, Type *Y, unsigned size){
 if (size>1){ Type S=0; // Area.
  for (unsigned l=0, r=1; r<size; ++l, ++r) // integrate.
  S+=0.5*(Y[r]+Y[l])*(X[r]-X[l]); return S;
 } else throw CEXCP::Exception("Invalid Operation",
  __FUNCTION__,"size must be >1");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! See cNorm(X,Y,size).
template <class Type> inline Type cArea(unsigned size, Type *x, Type *y){
 return cNorm(x,y,size);
}

/*===========================================================================*/
//! Searches inside 'tgt' vector/cBuffer/... container for an occurrence of 'arg'.
template <template <typename...> class Container, typename Type>
int cContains(Container<Type> &tgt, Type arg){
 for (unsigned i=0; i<tgt.size(); ++i) if (tgt[i]==arg) return i; return -1;
}

/*===========================================================================*/
//! NOTE: 'data' is modified (re-ordered)!
//! ** Gets the modal from a discrete distribution of values 'data'. Values
//!   are considered to match (or to be equal) within 'delta'. Set 'delta' to
//!   zero for an exact matching.
//! ** In case 'delta>0' use for instance:
//!    'cBuffer<Type> t(data,start_mod,size_mod);'
//!    to get the ordered values and check minimum, maximum or calculate
//!    the mean of the modal values.
template <class Type> unsigned cModal(Type *data, unsigned size,
Type delta, unsigned &start_mod, unsigned &size_mod){
 start_mod=size_mod=0;
 if (size<2) return size_mod;
 else { //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Type diff=std::numeric_limits<Type>::max(), d;
  unsigned l=0, r=0, sz; // tmp
//  cSortAscending(size,data);
  std::sort(data,data+size);
  do { //......................................................................
   while (++r<size && data[r]-data[l]<=delta);
   if ((sz=r-l)>size_mod){ // check new interval
    start_mod=l; size_mod=sz; diff=data[r-1]-data[l];
   } else if (sz==size_mod && (d=data[r-1]-data[l])<diff){
    start_mod=l; size_mod=sz; diff=d;
   } ++l;
 } while (r<size);
 } return size_mod;
}

/*============================================================================*/
//! Integrate over Y(X) (e.g vector, cBuffer).
template <template <typename ...> class Container, typename Type>
Type cNorm(Container<Type> &X, Container<Type> &Y){
 if (X.size()==Y.size()) return cNorm(X.data(),Y.data(),X.size());
 else throw CEXCP::Exception("Invalid Operation",
  __FUNCTION__,"X and Y have differente sizes");
}

/*===========================================================================*/
//! Normalize the sum of all 'buff' array elements to 'norm'.
template <class Type> void cNorm(Type* buff, Type norm, unsigned size){
Type sum=cNorm(buff,size); norm/=sum; // 1. / norm factor.
 for (unsigned i=0; i<size; ++i) buff[i]*=norm;
}

/*===========================================================================*/
//! Normalize 'buff' (e.g vector, cBuffer) vector to 'norm'.
template <template <typename ...> class Container, typename Type>
void cNorm(Container<Type> &buff, Type norm){
 cNorm(buff.data(),norm,buff.size());
}

/*===========================================================================*/
//! Normal distribution with mean='mean', sigma='sigma' and area=1 (-inf,inf).
double cNormal(double mean, double sigma, double x);

/*===========================================================================*/
//! returns 'val' if 'left<=val<=right', 'left' is 'val<left'
//! and 'right' if 'val>right' (i.e. 'cMin(right,cMax(value,left))')
template <class Type> Type cRange(Type val, Type left, Type right){
 if (val<left) return left; else if (val>right) return right; else return val;
}

/*===========================================================================*/
//! Smooth 'source' buffer averaging 'sampleLeft' items to the left and
//! 'sampleRight' items to the right side of each item and storing the output
//! into 'target'. Using different sampling windows to the left and right is
//! useful when we have very different rise and decay times.
template <class Type> void cSmoothNeighbor(
unsigned sampleLeft, unsigned sampleRight,
Type* source, Type* target, unsigned bufferSize){
Type sum=source[0]; unsigned rsize=bufferSize-1;
 for (unsigned pos=0, l=0, r=0; pos<bufferSize; ++pos){
  while (r-pos<sampleRight && r<rsize) sum+=source[++r];
  while (pos-l>sampleLeft) sum-=source[l++];
  target[pos]=sum/(r-l+1);
} }

/*===========================================================================*/
//! Smooth 'source' buffer averaging 'sampleLeft' items to the left and
//! 'sampleRight' items to the right side of each item and storing the output
//! into 'target'. Using different sampling windows to the left and right is
//! useful when we have very different rise and decay times.
template <template <typename ...> class Container, typename Type>
void cSmoothNeighbor(unsigned sampleLeft, unsigned sampleRight,
Container<Type> &in, Container<Type> &out){ out.resize(in.size());
 cSmoothNeighbor(sampleLeft,sampleRight, in.data(),out.data(),in.size());
}

/*===========================================================================*/
//! Calculates data*data.
template <class Type> inline
Type cSqr(Type data){ return data*data; }

/*===========================================================================*/
//! Swap contents pointed to 'Arg1' and 'Arg2': Not that this will invoque the
//! copy constructor for Type. In case we just want to swap pointers then
//! explicitly write that at template argument, eg: TYPE=cWheelBuffer<double>
//! TYPE *A, *B; cSwap<TYPE*>(A,B) (this corresponds to callings the version
//! cSwap(Type&, Type&) with Type==TYPE*).
//! <BR><BR><B>Example:<BR></B> @include cSwap.cs <BR>
template <class Type> void cSwap(Type *Arg1, Type *Arg2){
Type swap(*Arg1); *Arg1=*Arg2; *Arg2=swap;
}

/*===========================================================================*/
//! Swap contents of Arg1 and Arg2 - See 'cSwap(Type*, Type*)'.
//! This will also invoque the copy constructor for Type.
template <class Type> void cSwap(Type &Arg1, Type &Arg2){
Type swap(Arg1); Arg1=Arg2; Arg2=swap;
}

/*===========================================================================*/
//! Return 'Mean' and 'RMS' of 'nData' elements contained in buffer 'Data'.
template <class Type> void cStatistic
(Type* Data, unsigned nData, Type &Mean, Type &RMS){
unsigned idx; for (Mean=RMS=0, idx=0; idx<nData; ++idx){
 Mean+=Data[idx]; RMS+=cSqr(Data[idx]); }
 RMS = sqrt(RMS/nData-cSqr(Mean/=nData));
}

}

#endif // _MATH_ ############################################################## 

