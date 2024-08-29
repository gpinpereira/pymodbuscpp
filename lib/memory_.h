/**
 * @file memory_.h
 */

#ifndef _MEMORY_ //############################################################
#define _MEMORY_

#include "exception_.h"
#include "constanst_.h"

namespace CMATH {

//! cAllocate     : (Re)Allocate memory block.
//! cAllocate_aligned: Allocate an align memory block.
//! cAByteSwap(V) : Converts 'V' to its litle/big indian representation.
//! cArraySize(A) : Returns the size of a C-style array (e.g. A[]={}, A[N])
//! cNByteSwap(*P,N): Converts 'N' elements of 'V' to its litle/big indian representation.
//! cByteSwap     : Returns FALSE for a little-endian system, TRUE for a big-endian system.
//! cByteSwap(V)  : Returns The litle/big indian representation of 'V'.
//! cCopy(D,S,sz) : Copy 'sz' items from 'S' to 'D'
//! cDelete       : delete and null a pointer.
//! cRealloc      : Reallocate a memory block.

//! Build a 16 or 32 bits integer .
#define cMake16(msb, lsb)  ((msb << 8) | lsb)
#define cMake32(msb, sb3, sb2, lsb) ((msb << 24) | (sb3 << 16) | (sb2 << 8) | (lsb))

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                           cRawFileHandlerDescriptor                       */
/*! \author Francisco Neves                                                  */
/*! \date 2022.02.22 ( Last modified 2022.02.22 )                            */
/*! \brief Multiple Init Checker                                             */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! ** When needing to init a var that should be have the same value in
//!   multiple initialization instances for e.g. in a loop.:
//! cInitCheck init(var);
//! for ( ... multiple possibilitite to init var ...; ++i){
//!   init=var_possibility_i;
//!   if (!init.is_good()) ... not all init values are equal ...
template <class Type> class cMultipleInitsCheck{
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    Type &FValue;
    bool FGood, FInit;
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cMultipleInitsCheck(Type &var_);
    cMultipleInitsCheck(Type &var_, const Type &val_);
    bool operator=(const Type &val_);
    //.........................................................................
    inline bool is_init(){ return FInit; }
    inline bool is_good(){ return FGood; }
};

/*===========================================================================*/
template <class Type> cMultipleInitsCheck<Type>
::cMultipleInitsCheck(Type &var_)
:FValue(var_),FGood(true),FInit(false){ }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMultipleInitsCheck<Type>
::cMultipleInitsCheck(Type &var_, const Type &val_)
:FValue(var_),FGood(true),FInit(true){ FValue=val_; }

/*===========================================================================*/
template <class Type> bool
cMultipleInitsCheck<Type>::operator=(const Type &val_){
 if (FInit){ // was already set once
  if (FValue!=val_) FGood=false; // trying to assign a different value
 } else { FValue=val_; FGood=FInit=true; }
 return FGood;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

bool cByteSwap();

/*===========================================================================*/
//! Allocate (Ptr==NULL) or reallocate (Ptr!=NULL) memory block.
template <class Type, class size_type>
inline Type* cAllocate(size_type size, Type* Ptr){
 if (size==0 && Ptr==nullptr) return nullptr; else // man: realloc.
 return (Type*)realloc((void*)Ptr,size*sizeof(Type));
}

/*===========================================================================*/
//! Allocate 'size' 'Type' items whose alignment is specified by 'Alignment'.
//! The size in bytes must be an integral multiple of Alignment.
template <class Type, unsigned Alignment> // must be a multiple of 2
Type* cAllocate_aligned(unsigned size){
Type *ptr=(Type*)aligned_alloc(Alignment,size*sizeof(Type));
 if (ptr) return ptr; else throw CEXCP::Exception("Invalid Operation",
   __FUNCTION__,"Fail to allocate Memory");
}

/*===========================================================================*/
template <class Type> //! litle/big indian conversion.
Type cByteSwap(const Type& in){ Type out;
char *pin=(char*)&in, *pout=(char*)&out, *pend=pin+sizeof(Type);
 for (pout+=sizeof(Type)-1; pin<pend; ++pin, --pout) *pout=*pin;
 return out; // copy constructor.
}

/*===========================================================================*/
template <class Type> //! litle/big indian conversion.
inline void cAByteSwap(Type& A){ A=cByteSwap(A); }

/*===========================================================================*/
//! ** Get the size of an array: double a[N], b[]={1,2};
//! ** C++17 can be used std:.size(instead).
template <class T, unsigned N>
constexpr unsigned cArraySize(const T (&)[N]){ return N; }

/*===========================================================================*/
template <class Type> // litle/big indian conversion.
inline Type* cNByteSwap(Type* tgt, unsigned N){
Type *end=tgt+N; while (tgt<end){ cAByteSwap(*tgt); ++tgt; }
 return tgt;
}

/*===========================================================================*/
template <class T> //! Wrapper to 'memcpy'.
void cCopy(T* dest, const T* src, unsigned size){
 memcpy(dest,src,sizeof(T)*size);
}

/*===========================================================================*/
//! Delete and null 'ptr', which must have been created with new.
template <class T> void cDelete(T* &ptr){ delete ptr; ptr=nullptr; }

/*===========================================================================*/
//! Allocate 'buffer' memory block and throw if it fails (keeping existing block).
template <class T> void cRealloc(T& buffer, unsigned size){
T bff=cAllocate(size,buffer); // See: NULL && 0 behaviour.
 if (bff==nullptr && size>0) throw CEXCP::Exception("Invalid Operation",
  __FUNCTION__,"Fail to reallocate Memory - contents unchanged");
 else buffer=bff;
}

}

#endif // _MEMORY_ ############################################################ 
