/**
 * @file buffer_.h
 */

#ifndef _BUFFER_ //############################################################
#define _BUFFER_

#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <initializer_list>
#include <limits> // ::numeric_limits
#include <limits.h> // integer types
#include <cstring> // memcpy
#include <stdarg.h>

#include "exception_.h"
#include "memory_.h"
#include "math_.h"
#include "string_.h"

namespace CMATH {

template <class T, class S, class A> class cBuffer;
template <class T> class cMathBuffer;

//! c1stDerivative(Y,dY,dx): Calculates the 1st derivative 'dY' of 'Y' for a XX increment of 'dx'.
//! c2ndDerivative(Y,ddY,dx): Calculates the 2nd derivative 'ddY' of 'Y' for a XX increment of 'dx'.
//! cAbs(V)        : Replaces all 'V' elements by their absolute value.
//! cContractAxis(A,c,s,f): contracts an axis 'A' by 'f' around 'c' with a regular step of 's'.
//! cGetAxis(d,a)  : Extracts into 'a' periodic axis values/ticks from 'd'.
//! cLoad(F,N,...) : Loads 'N' buffers (cBuffer/vector) from file 'F' (column wise).
//! cMax(V)        : Get the maximum element in 'V' (cBuffer/vector);
//! cMin(V)        : Get the minimum element in 'V' (cBuffer/vector);
//! cSave(F,N,...) : Saves 'N' buffers (cBuffer/vector) into file 'F' (column wise).
//! cString (V,S,p): Prints 'V' elements (with precision 'p') separated by "S".

//! Note that the copy constructor for 'cBuffer' is not the standard
//! which limits the use of 'cBuffer<Type> operator*/+-', for instance.
//! See 'cMathBuffer' which inherites all 'cBuffer' capabilities
//! and implement the operators*/+= for cBuffer, vector, etc
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator<<(cBuffer<Type,Size,Alloc>&, Type);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator<<(Type, cBuffer<Type,Size,Alloc>&);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator*=(cBuffer<Type,Size,Alloc>&, Type);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator/=(cBuffer<Type,Size,Alloc>&, Type);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator-=(cBuffer<Type,Size,Alloc>&, Type);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator+=(cBuffer<Type,Size,Alloc>&, Type);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator*=(cBuffer<Type,Size,Alloc>&, cBuffer<Type,Size,Alloc>&);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator/=(cBuffer<Type,Size,Alloc>&, cBuffer<Type,Size,Alloc>&);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator+=(cBuffer<Type,Size,Alloc>&, cBuffer<Type,Size,Alloc>&);
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
    operator-=(cBuffer<Type,Size,Alloc>&, cBuffer<Type,Size,Alloc>&);

template <class Type> cMathBuffer<Type> &operator<<(cMathBuffer<Type>&, Type);
template <class Type> cMathBuffer<Type> &operator<<(Type, cMathBuffer<Type>&);

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 cBuffer                                   */
/*! \author Francisco Neves                                                  */
/*! \date 2008.05.06 ( Last modified 2021.03.23 )                            */
/*! \brief ??? Missing Content ???                                           */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! Supplies the static entry point for (re-)allocation of memory.
struct cAllocate_dft { // Default allocator: see 'cAllocate'
    template <class Type, class size_type> static inline
    Type* alloc(size_type size, Type* Ptr){ return cAllocate(size,Ptr); }
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! -- NOTE1: 'cBuffer' does not have a standard copy constructor (by design).
//! -- NOTE2: there is a minimum of indexes and size checkings. That must be
//!  done when inheriting/using the base 'cBuffer' class;
//! -- NOTE3: The owned members must be explictly deleted ('del_ref', '')
//! -- NOTE4: Unless using explicitly the 'capacity' family of functions,
//!  when using only 'size' (or functions calling 'size' as 'push_back', etc)
//!  the capacity of 'cBuffer' will stay equal to the maximum previous requested
//!  size (being only changed if the size increases). If using 'size' with
//!  granularity, the capacity will grow/decrease with the requested granularity.
//! -- NOTE5: '::copy', etc from other classes (e.g vector) assume that the
//!  owned memory is a single block, similarly to cBuffer.
template <class Type, class Size=unsigned, typename Allocate=cAllocate_dft>
class cBuffer {
public: typedef Size size_type;
public: typedef Type value_type;
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    bool FOwner;
    Type* FBuff;
    Size FCapacity, FSize;
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    void reallocate(Size capacity);
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cBuffer();
    cBuffer(Size size);
    cBuffer(const cBuffer &bhost); // ref.
    cBuffer(Type* phost, Size size); // ref.
    cBuffer(const cBuffer &bhost, Size start, Size size); // ref.
    cBuffer(std::initializer_list<Type> iVals); // copy.
    virtual ~cBuffer(){ if (FOwner) free(FBuff); }
    //.........................................................................
    void operator()();
    void operator()(Size size);
    void operator()(const cBuffer &bhost); // ref.
    void operator()(Type* phost, Size size); // ref.
    void operator()(const cBuffer &bhost, Size start, Size size); // ref.
    //.........................................................................
    cBuffer& operator=(std::initializer_list<Type> iVals); // copy
    cBuffer& operator=(const cBuffer &src); // copy.
    cBuffer& operator=(Type V); // copy.
    //.........................................................................
    void capacity(Size capacity, Size granular);
    void size(Size size, Size granular);
    void capacity(Size capacity);
    void size(Size size);
    inline void reserve(Size C){ capacity(C); } // std::vector tmplt.
    inline void resize(Size S){ size(S); } // std::vector tmplt.
    inline void clear(){ size(0); }
    void reset();
    //.........................................................................
    void size_ptr(Size S); // new items init to 'nullptr'
    void capacity_ptr(Size C);
    void del_ptr(Size idx, Size count);
    inline void del_ptr(){ del_ptr(0,FSize); }
    void size_ref(Size S); // new items init to default constructor see '*_ref'.
    void capacity_ref(Size C);
    void construct_ref(Size idx, Size count);
    inline void construct_ref(){ construct_ref(0,FSize); }
    inline void construct_refFront(){ construct_ref(0,FSize); }
    inline void construct_refBack(){ construct_ref(FSize-1,1); }
    void del_ref(Size idx, Size count);
    inline void del_ref(){ del_ref(0,FSize); }
    //.........................................................................
    bool containsSub(const cBuffer &sub);
    int first_of(Type val, Size start=0);
    int last_of(Type val,Size start=std::numeric_limits<Size>::max());
    Size compact(Type empty);
    //.........................................................................
    void copy(Type *src, Size size);
    inline void copy(cBuffer& src){ copy(src.data(),src.size()); }
    inline void copyto(cBuffer &tgt){ tgt.copy(THIS); }
    //.........................................................................
    void init_arg(Size nArgs, ...);
    void init(Type val, Type inc);
    void init(Type val);
    //.........................................................................
    void remove(Size at, Size n);
    void insert(Size at, Size n);
    void insert(Size at, cBuffer& src);
    void insert(Size at, Size n, Size G);
    void insert(Size at, cBuffer& src, Size G);
    //.........................................................................
    inline Type& push_front(Type V, Size G){ return *(new (&add_front(G)) Type(V)); }
    inline Type& push_front(Type V){ return *(new (&add_front()) Type(V)); }
    inline Type& push_front(){ return *(new (&add_front()) Type()); }
    inline void push_front(cBuffer &src){ insert(0,src); }
    //.........................................................................
    inline Type& push_back(Type V, Size G){ return *(new (&add_back(G)) Type(V)); }
    inline Type& push_back(Type V){ return *(new (&add_back()) Type(V)); }
    inline Type& push_back(){ return *(new (&add_back()) Type()); }
    inline void push_back(cBuffer &src){ insert(FSize,src); }
    //.........................................................................
    inline void pop_back(Size G){ if (FSize>0) size(FSize-1,G); }
    inline void pop_back(){ if (FSize>0) size(FSize-1); }
    //.........................................................................
    Type& add_front(Size G); //> no init
    Type& add_front(); //> no init
    Type& add_back(Size G); //> no init
    Type& add_back(); //> no init
    //.........................................................................
    inline Type& front(){ return *FBuff; }
    inline Type& back(){ return *(FBuff+FSize-1); }
    inline Type& operator[](Size idx){ return FBuff[idx]; }
    inline Type& item(Size idx){ return FBuff[idx]; }
    inline Type& itemInv(Size idxInv){ return FBuff[FSize-(++idxInv)]; }
    inline Type* rbegin(){ return FBuff+FSize-1; }
    inline Type* end(){ return FBuff+FSize; }
    inline Type* rend(){ return FBuff-1; }
    inline Type* begin(){ return FBuff; }
    //.........................................................................
    inline bool empty() const { return FSize==0; }
    inline Size size() const { return FSize; }
    inline Size sizeBytes() const { return FSize*sizeof(Type); }
    inline Size sizeItem() const { return sizeof(Type); }
    inline Size capacityBytes() const { return FCapacity*sizeof(Type); }
    inline Size capacity() const { return FCapacity; }
    inline Type* buffer() const { return FBuff; }
    inline Type* data() const { return FBuff; }
    inline bool owner() const { return FOwner; }
    //.........................................................................
    template <template <typename ...> class C> cBuffer(const C<Type> &src); // ref.
    template <template <typename ...> class C> void operator()(const C<Type> &src); // ref.
    template <template <typename ...> class C> cBuffer& operator=(const C<Type> &src); // copy.
    template <template <typename ...> class C> void copy(C<Type>& src);
    template <template <typename ...> class C> void copyto(C<Type>& src);
    template <template <typename ...> class C> void insert(Size at, C<Type>& src);
    template <template <typename ...> class C> void insert(Size at, C<Type>& src, Size G);
    template <template <typename ...> class C> void push_front(C<Type> &src);
    template <template <typename ...> class C> void push_back(C<Type> &src);
};

/*===========================================================================*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::reallocate(Size C){
 if (C!=FCapacity){ Type *tmp=Alloc::alloc(C,FBuff); // safe.
  if (tmp==nullptr && C>0) throw CEXCP::Exception("Invalid Operation",
   CEXCP::cTypeID(THIS,__FUNCTION__),"Fail to (re)allocate buffer");
  else { FCapacity=C; FBuff=tmp; }
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Clears & takes onwnership of memory.
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::operator()(){ // default.
 if (FOwner) free(FBuff); FBuff=nullptr, FSize=FCapacity=0, FOwner=true; // safe.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Clears & makes 'FBuffer' a reference to 'H' buffer (do not onwn the memory).
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::operator()(const cBuffer &H){
 if (FOwner) free(FBuff); FBuff=H.FBuff; // safe.
 FCapacity=H.FCapacity, FSize=H.FSize, FOwner=false;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Clears & takes onwnership of memory.
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::operator()(Size SZ){ // allocate.
 if (!FOwner) FBuff=nullptr, FSize=FCapacity=0, FOwner=true; // safe.
 reallocate(SZ), FSize=SZ;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Clears & makes 'FBuffer' a reference to 'H' buffer (do not onwn the memory).
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::operator()(Type* H, Size SZ){
 if (FOwner) free(FBuff); FBuff=H, FCapacity=FSize=SZ, FOwner=false; // safe.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Clears & makes 'FBuffer' a reference to 'H' buffer (do not onwn the memory).
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::operator()(const cBuffer &H, Size S, Size SZ){
 if (FOwner) free(FBuff),FBuff=nullptr,FSize=FCapacity=0,FOwner=false;
 if (S+SZ>H.FSize) throw CEXCP::Exception("Invalid Operation", // safe.
  CEXCP::cTypeID(THIS,__FUNCTION__),"Fail to reuse buffer");
 else FBuff=H.FBuff+S,FSize=SZ,FCapacity=H.FCapacity-S;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
cBuffer<Type,Size,Alloc>::operator=(std::initializer_list<Type> iVals){
 if (!FOwner) FBuff=nullptr, FSize=FCapacity=0, FOwner=true; // safe.
 reallocate(iVals.size()); FSize=iVals.size();
 for (auto val=iVals.begin(); val<iVals.end(); ++val)
  FBuff[val-iVals.begin()]=*val;
 return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Copy 'src'. Needs to onwn the buffer!
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
cBuffer<Type,Size,Alloc>::operator=(const cBuffer<Type,Size,Alloc> &src){
 copy((cBuffer<Type>&)src); return *this;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Sets all elements to V. Do not need to own de buffer.
template <class Type, class Size, class Alloc> cBuffer<Type,Size,Alloc>&
cBuffer<Type,Size,Alloc>::operator=(Type V){
 for (Size i=0; i<FSize; ++i) FBuff[i]=V; return THIS;
}

/*===========================================================================*/
template <class Type, class Size, class Alloc>
cBuffer<Type,Size,Alloc>::cBuffer(): // default.
FOwner(true),FBuff(nullptr),FCapacity(0),FSize(0){ }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
cBuffer<Type,Size,Alloc>::cBuffer(const cBuffer& H): // reference.
FOwner(false),FBuff(H.FBuff),FCapacity(H.FCapacity),FSize(H.FSize){ }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
cBuffer<Type,Size,Alloc>::cBuffer(Size SZ): // allocate.
FOwner(true),FBuff(nullptr),FCapacity(0),FSize(0){ reallocate(SZ), FSize=SZ; }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
cBuffer<Type,Size,Alloc>::cBuffer(Type* H, Size SZ):
FOwner(false),FBuff(H),FCapacity(SZ),FSize(SZ){ } // reference.

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
cBuffer<Type,Size,Alloc>::cBuffer(const cBuffer &H, // reference
Size S, Size SZ):FOwner(false),FBuff(nullptr),FCapacity(0),FSize(0){
 if (S+SZ>H.FSize) throw CEXCP::Exception("Invalid Operation", // safe.
  CEXCP::cTypeID(THIS,__FUNCTION__),"Fail to reuse buffer");
 else FBuff=H.FBuff+S,FSize=SZ,FCapacity=H.FCapacity-S;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// Construct and initialize the vector.
template <class Type, class Size, class Alloc>
cBuffer<Type,Size,Alloc>::cBuffer(std::initializer_list<Type> iVals)
:FOwner(true),FBuff(nullptr),FCapacity(0),FSize(0){
 reallocate(iVals.size()), FSize=iVals.size();
 for (auto val=iVals.begin(); val<iVals.end(); ++val)
  FBuff[val-iVals.begin()]=*val;
}

/*===========================================================================*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::capacity(Size C, Size G){
 if (FOwner){ reallocate((C/G+1)*G); if (FSize>FCapacity) FSize=FCapacity; }
 else if (C!=FCapacity) // allow if the new capacity == old capacity.
  throw CEXCP::Exception("Invalid Operation",CEXCP::cTypeID(THIS,
   __FUNCTION__),"cBuffer do not own the memory buffer."); // safe.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::size(Size S, Size G){
 if (FOwner){ reallocate((S/G+1)*G); FSize=S; }
 else if (S!=FSize) // allow if the new size == old size.
  throw CEXCP::Exception("Invalid Operation",CEXCP::cTypeID(THIS,
   __FUNCTION__),"cBuffer do not own the memory buffer."); // safe.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::capacity(Size C){
 if (FOwner){ reallocate(C); if (FSize>FCapacity) FSize=FCapacity; }
 else if (C!=FCapacity) // allow if the new capacity == old capacity.
  throw CEXCP::Exception("Invalid Operation",CEXCP::cTypeID(THIS,
   __FUNCTION__),"cBuffer do not own the memory buffer."); // safe.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::size(Size S){
 if (FOwner){ if (S>FCapacity) reallocate(S); FSize=S; }
 else if (S!=FSize) // allow if the new size == old size.
  throw CEXCP::Exception("Invalid Operation",CEXCP::cTypeID(THIS,
   __FUNCTION__),"cBuffer do not own the memory buffer."); // safe.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::size_ptr(Size S){ Size oldS=FSize; size(S);
 if (FSize>oldS) for (Size i=oldS; i<FSize; ++i) FBuff[i]=nullptr;
 else for (Size i=FSize; i<oldS; ++i) delete FBuff[i]; // del
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::size_ref(Size S){ Size oldS=FSize; size(S);
 if (FSize>oldS) for (Size i=oldS; i<FSize; ++i) new (FBuff+i) Type();
 else for (Size i=FSize; i<oldS; ++i) (FBuff+i)->~Type(); // del
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::capacity_ptr(Size C){
 for (Size i=C; i<FSize; ++i) delete FBuff[i]; // del if (C<FSize)
 capacity(C);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::capacity_ref(Size C){
 for (Size i=C; i<FSize; ++i) (FBuff+i)->~Type(); // del if (C<FSize)
 capacity(C);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::reset(){
 if (FOwner) free(FBuff); else FOwner=true; FBuff=nullptr; FSize=FCapacity=0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::del_ptr(Size S, Size N){
 for (Type *item_=FBuff+S, *end_=item_+N; item_<end_; ++item_) delete *item_;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::construct_ref(Size S, Size N){
 for (Type *item_=FBuff+S, *end_=item_+N; item_<end_; ++item_) new (item_) Type();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::del_ref(Size S, Size N){
 for (Type *item_=FBuff+S, *end_=item_+N; item_<end_; ++item_) item_->~Type();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
bool cBuffer<Type,Size,Alloc>::containsSub(const cBuffer &subB){
 return subB.FBuff>=FBuff && subB.end()<=end();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
int cBuffer<Type,Size,Alloc>::first_of(Type val, Size start){
 for (Size i=start; i<FSize; ++i) if (val==FBuff[i])
  return i; return -1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
int cBuffer<Type,Size,Alloc>::last_of(Type val, Size start){
 for (int i=int(cMin(start,FSize))-1; i>=0; --i) if (val==FBuff[i])
  return i; return -1;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
Size cBuffer<Type,Size,Alloc>::compact(Type empty_){
Type *fill=FBuff, *tot=FBuff, *end_=FBuff+FSize; // marks.
 for (; tot<end_; ++tot) if (*tot!=empty_){ *fill=*tot; ++fill; }
 return FSize-Size(tot-fill);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::copy(Type *H, Size SZ){
 size(SZ); memcpy(FBuff,H,FSize*sizeof(Type));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::init_arg(Size nArgs, ...){
va_list list; va_start(list,nArgs); size(nArgs);
 for (Size i=0; i<nArgs; ++i) FBuff[i]=va_arg(list,Type);
 va_end(list);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! An increment 'V[i+1]=val[i]+inc' creates roundoff errors for big vectors
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::init(Type val, Type inc){
 for (Size i=0; i<FSize; ++i) FBuff[i]=val+inc*i;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::init(Type val){
 for (Size i=0; i<FSize; ++i) FBuff[i]=val;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::remove(Size at, Size n){
Size cp=at+n, scp, sdl; // safe operation even with out of size.
 if (cp<FSize){ scp=FSize-cp; sdl=n; } else { scp=0; sdl=n+FSize-cp; }
 memmove(FBuff+at,FBuff+cp,scp*sizeof(Type)); size(FSize-sdl);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::insert(Size at, Size n){
Size cp=at<FSize?FSize-at:0; size(FSize+n);
 memmove(FBuff+at+n,FBuff+at,cp*sizeof(Type));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::insert(Size at, cBuffer& src){
 insert(at=cMin(at,FSize),src.size());
 memcpy(FBuff+at,src.FBuff,src.FSize*sizeof(Type));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::insert(Size at, Size n, Size G){
Size cp=at<FSize?FSize-at:0; size(FSize+n,G);
 memmove(FBuff+at+n,FBuff+at,cp*sizeof(Type));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
void cBuffer<Type,Size,Alloc>::insert(Size at, cBuffer& src, Size G){
 insert(at=cMin(at,FSize),src.size(),G);
 memcpy(FBuff+at,src.FBuff,src.FSize*sizeof(Type));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
Type& cBuffer<Type,Size,Alloc>::add_front(Size G){
Size P=FSize; size(FSize+1,G); memcpy(FBuff+1,FBuff,P*sizeof(Type));
 return *FBuff;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
Type& cBuffer<Type,Size,Alloc>::add_front(){
Size P=FSize; size(FSize+1); memcpy(FBuff+1,FBuff,P*sizeof(Type));
 return *FBuff;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
Type& cBuffer<Type,Size,Alloc>::add_back(Size G){
Size P=FSize; size(FSize+1,G); return *(FBuff+P);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
Type& cBuffer<Type,Size,Alloc>::add_back(){
Size P=FSize; size(FSize+1); return *(FBuff+P);
} 

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Makes 'FBuffer' a reference to 'src' data (do not onwn the memory).
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
cBuffer<Type,Size,Alloc>::cBuffer(const Container<Type> &src){ // ref
 FOwner=false; FBuff=(Type*)src.data(); FSize=src.size();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Clears & makes 'FBuffer' a reference to 'src' data (do not onwn the memory).
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
void cBuffer<Type,Size,Alloc>::operator()(const Container<Type> &src){ // ref
 if (FOwner) free(FBuff); FOwner=false;
 FBuff=(Type*)src.data(); FSize=src.size();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Copy 'src'. Needs to onwn the buffer!
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
inline cBuffer<Type,Size,Alloc>& cBuffer<Type,Size,Alloc>
::operator=(const Container<Type> &src){ // copy
 copy((std::vector<Type>&)src);
 return *this;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
inline void cBuffer<Type,Size,Alloc>::copy(Container<Type>& src){
 copy(src.data(),src.size());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
void cBuffer<Type,Size,Alloc>::copyto(Container<Type> &tgt){
 tgt.resize(FSize); memmove(tgt.data(),FBuff,FSize*sizeof(Type));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
void cBuffer<Type,Size,Alloc>::insert(Size at, Container<Type>& src){
 insert(at=cMin(at,FSize),src.size());
 memcpy(FBuff+at,src.data(),src.size()*sizeof(Type));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
void cBuffer<Type,Size,Alloc>::insert(Size at, Container<Type> &src, Size G){
 insert(at=cMin(at,FSize),src.size(),G);
 memcpy(FBuff+at,src.data(),src.size()*sizeof(Type));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
inline void cBuffer<Type,Size,Alloc>::push_front(Container<Type> &src){
 insert(0,src);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type, class Size, class Alloc>
template <template <typename ...> class Container>
inline void cBuffer<Type,Size,Alloc>::push_back(Container<Type> &src){
 insert(FSize,src);
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                               cMathBuffer                                 */
/*! \author Francisco Neves                                                  */
/*! \date 2008.05.06 ( Last modified 2017.10.10 )                            */
/*! \brief cBuffer extension with implementation of standard operators       */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/// \details
//! Same capabilities as 'cBuffer' but implements standard copy constructors
//! for the definition of the mathematical operators. Also, 'cMathBuffers'
//! reimplements all the 'cBuffer' functions/operators which would loose
//! the ownership of the buffer to keep consistency.
template <class Type> class cMathBuffer: public cBuffer<Type>{
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cMathBuffer();
    cMathBuffer(unsigned size);
    cMathBuffer(const cMathBuffer &bhost);
    cMathBuffer(const cBuffer<Type> &bhost);
    cMathBuffer(Type* phost, unsigned size);
    cMathBuffer(const cMathBuffer &bhost, unsigned start, unsigned size);
    cMathBuffer(const cBuffer<Type> &bhost, unsigned start, unsigned size);
    // Rewrite not to loose ownership of memory (ALWAYS COPY)..................
    void operator()();
    void operator()(unsigned size);
    void operator()(Type* phost, unsigned size);
    void operator()(const cMathBuffer &bhost);
    void operator()(const cBuffer<Type> &bhost);
    void operator()(const cMathBuffer &bhost, unsigned start, unsigned size);
    void operator()(const cBuffer<Type> &bhost, unsigned start, unsigned size);
    cMathBuffer& operator=(const cMathBuffer &src);
    //.........................................................................
    cMathBuffer operator+(Type V);
    cMathBuffer &operator+=(Type V);
    cMathBuffer operator+(cMathBuffer &V);
    cMathBuffer &operator+=(cMathBuffer &V);
    cMathBuffer operator+(cBuffer<Type> &V);
    cMathBuffer &operator+=(cBuffer<Type> &V);
    cMathBuffer operator+(std::vector<Type> &V);
    cMathBuffer &operator+=(std::vector<Type> &V);
    cMathBuffer operator-(Type V);
    cMathBuffer &operator-=(Type V);
    cMathBuffer operator-(cMathBuffer &V);
    cMathBuffer &operator-=(cMathBuffer &V);
    cMathBuffer operator-(cBuffer<Type> &V);
    cMathBuffer &operator-=(cBuffer<Type> &V);
    cMathBuffer operator-(std::vector<Type> &V);
    cMathBuffer &operator-=(std::vector<Type> &V);
    cMathBuffer operator*(Type V);
    cMathBuffer &operator*=(Type V);
    cMathBuffer operator*(cMathBuffer &V);
    cMathBuffer &operator*=(cMathBuffer &V);
    cMathBuffer operator*(cBuffer<Type> &V);
    cMathBuffer &operator*=(cBuffer<Type> &V);
    cMathBuffer operator*(std::vector<Type> &V);
    cMathBuffer &operator*=(std::vector<Type> &V);
    cMathBuffer operator/(Type V);
    cMathBuffer &operator/=(Type V);
    cMathBuffer operator/(cMathBuffer &V);
    cMathBuffer &operator/=(cMathBuffer &V);
    cMathBuffer operator/(cBuffer<Type> &V);
    cMathBuffer &operator/=(cBuffer<Type> &V);
    cMathBuffer operator/(std::vector<Type> &V);
    cMathBuffer &operator/=(std::vector<Type> &V);
};

/*===========================================================================*/
template <class Type> // Default constructor.
cMathBuffer<Type>::cMathBuffer():cBuffer<Type>(){ }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> // Allocates memory only.
cMathBuffer<Type>::cMathBuffer(unsigned size):cBuffer<Type>(size){ }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> // Copy Constructor.
::cMathBuffer(const cMathBuffer &bhost):cBuffer<Type>(bhost.size()){
 memcpy(cBuffer<Type>::buffer(),bhost.buffer(),cBuffer<Type>::sizeBytes());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> // Copy Constructor.
::cMathBuffer(const cBuffer<Type> &bhost) :cBuffer<Type>(bhost.size()){
 memcpy(cBuffer<Type>::buffer(),bhost.buffer(),cBuffer<Type>::sizeBytes());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> // Copy Constructor.
::cMathBuffer(Type* phost, unsigned size):cBuffer<Type>(size){
 memcpy(cBuffer<Type>::buffer(),phost,cBuffer<Type>::sizeBytes());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type>::cMathBuffer // copy constructor.
(const cMathBuffer &bhost, unsigned start, unsigned size):cBuffer<Type>(size){
 memcpy(cBuffer<Type>::buffer(),bhost.buffer()+start,cBuffer<Type>::sizeBytes());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type>::cMathBuffer // copy constructor.
(const cBuffer<Type> &bhost, unsigned start, unsigned size):cBuffer<Type>(size){
 memcpy(cBuffer<Type>::buffer(),bhost.buffer()+start,cBuffer<Type>::sizeBytes());
}

/*===========================================================================*/
template <class Type> inline void cMathBuffer<Type> // Clear.
::operator()(){ cBuffer<Type>::operator()(); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> inline void cMathBuffer<Type> // allocs memory.
::operator()(unsigned size){ cBuffer<Type>::operator()(size); }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> inline // Copy.
void cMathBuffer<Type> ::operator()(Type* phost, unsigned size){
 cBuffer<Type>::copy(phost,size);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> inline // Copy.
void cMathBuffer<Type>::operator()(const cMathBuffer &H){
 cBuffer<Type>::copy(H.buffer(),H.size());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> inline // Copy.
void cMathBuffer<Type>::operator()(const cBuffer<Type> &H){
 cBuffer<Type>::copy(H.buffer(),H.size());
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> inline void cMathBuffer<Type> // Copy.
::operator()(const cMathBuffer &H, unsigned S, unsigned SZ){
 cBuffer<Type>::copy(H.buffer()+S,SZ);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> inline void cMathBuffer<Type> // Copy
::operator()(const cBuffer<Type> &H, unsigned S, unsigned SZ){
 cBuffer<Type>::copy(H.buffer()+S,SZ);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type>
&cMathBuffer<Type>::operator=(const cMathBuffer &src){
 cBuffer<Type>::copy((cBuffer<Type>&)src); return THIS; }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>
::operator+(Type V){ cMathBuffer tmp(THIS.size()); // uses copy constructor.
 for (unsigned i=0; i<THIS.size(); ++i) tmp[i]=THIS[i]+V; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator+=(Type V){
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]+=V; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator+
(cMathBuffer<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]+V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator+=
(cMathBuffer<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]+=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator+
(cBuffer<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]+V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator+=
(cBuffer<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]+=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator+
(std::vector<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),(unsigned)V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]+V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator+=
(std::vector<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]+=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>
::operator-(Type V){ cMathBuffer tmp(THIS.size()); // uses copy constructor.
 for (unsigned i=0; i<THIS.size(); ++i) tmp[i]=THIS[i]-V; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator-=(Type V){
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]-=V; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator-
(cMathBuffer<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]-V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator-=
(cMathBuffer<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]-=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator-
(cBuffer<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]-V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator-=
(cBuffer<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]-=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator-
(std::vector<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),(unsigned)V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]-V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator-=
(std::vector<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]-=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>
::operator*(Type V){ cMathBuffer tmp(THIS.size()); // uses copy constructor.
 for (unsigned i=0; i<THIS.size(); ++i) tmp[i]=THIS[i]*V; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator*=(Type V){
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]*=V; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator*
(cMathBuffer<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]*V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator*=
(cMathBuffer<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]*=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator*
(cBuffer<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]*V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator*=
(cBuffer<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]*=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator*
(std::vector<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),(unsigned)V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]*V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator*=
(std::vector<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]*=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>
::operator/(Type V){ cMathBuffer tmp(THIS.size()); // uses copy constructor.
 for (unsigned i=0; i<THIS.size(); ++i) tmp[i]=THIS[i]/V; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator/=(Type V){
 for (unsigned i=0; i<cBuffer<Type>::size(); ++i) THIS[i]/=V; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator/
(cMathBuffer<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]/V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator/=
(cMathBuffer<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]/=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator/
(cBuffer<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]/V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator/=
(cBuffer<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]/=V[i]; return THIS;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> cMathBuffer<Type>::operator/
(std::vector<Type> &V){ cMathBuffer tmp(cMin(THIS.size(),(unsigned)V.size()));
 for (unsigned i=0; i<tmp.size(); ++i) tmp[i]=THIS[i]/V[i]; return tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
template <class Type> cMathBuffer<Type> &cMathBuffer<Type>::operator/=
(std::vector<Type> &V){ if (V.size()<THIS.size()) THIS.size(V.size());
 for (unsigned i=0; i<THIS.size(); ++i) THIS[i]/=V[i]; return THIS;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
template <class Type> //! Insertion operator for cBuffer
cBuffer<Type> &operator<<(cBuffer<Type> &Target, Type Arg){
 Target.push_back(Arg); return Target;
}

/*===========================================================================*/
template <class Type> //! Insertion operator for cBuffer
cBuffer<Type> &operator<<(Type Arg, cBuffer<Type> &Target){
 Target.insert(0,1); Target[0]=Arg; return Target;
}

/*===========================================================================*/
template <class Type> //! inplace multiplication (*=) operator for cBuffer.
cBuffer<Type> &operator*=(cBuffer<Type> &L, Type R){
 for (unsigned i=0; i<L.size(); ++i) L[i]*=R; return L; }

/*===========================================================================*/
template <class Type> //! inplace division (/=) operator for cBuffer.
cBuffer<Type> &operator/=(cBuffer<Type> &L, Type R){
 for (unsigned i=0; i<L.size(); ++i) L[i]/=R; return L; }

/*===========================================================================*/
template <class Type> //! inplace subtration (-=) operator for cBuffer.
cBuffer<Type> &operator-=(cBuffer<Type> &L, Type R){
 for (unsigned i=0; i<L.size(); ++i) L[i]-=R; return L; }

/*===========================================================================*/
template <class Type> //! inplace sum (+=) operator for cBuffer.
cBuffer<Type> &operator+=(cBuffer<Type> &L, Type R){
 for (unsigned i=0; i<L.size(); ++i) L[i]+=R; return L; }

/*===========================================================================*/
template <class Type> //! inplace multiplication (*=) operator for cBuffer.
cBuffer<Type> &operator*=(cBuffer<Type> &L, cBuffer<Type> &R){
 for (unsigned i=0; i<L.size(); ++i) L[i]*=R[i]; return L;  }

/*===========================================================================*/
template <class Type> //! inplace division (/=) operator for cBuffer.
cBuffer<Type> &operator/=(cBuffer<Type> &L, cBuffer<Type> &R){
 for (unsigned i=0; i<L.size(); ++i) L[i]/=R[i]; return L; }

/*===========================================================================*/
template <class Type> //! inplace sum (+=) operator for cBuffer.
cBuffer<Type> &operator+=(cBuffer<Type>& L, cBuffer<Type> &R){
 for (unsigned i=0; i<L.size(); ++i) L[i]+=R[i]; return L; }

/*===========================================================================*/
template <class Type> //! inplace subtration (-=) operator for cBuffer.
cBuffer<Type> &operator-=(cBuffer<Type>& L, cBuffer<Type> &R){
 for (unsigned i=0; i<L.size(); ++i) L[i]-=R[i]; return L; }

/*===========================================================================*/
template <class Type> inline
cMathBuffer<Type> &operator<<(cMathBuffer<Type> &Target, Type Arg){
 Target.push_back(Arg); return Target;
}

/*===========================================================================*/
template <class Type> inline
cMathBuffer<Type> &operator<<(Type Arg, cMathBuffer<Type> &Target){
 Target.insert(0,1); Target[0]=Arg; return Target;
}

/*===========================================================================*/
//! Calculates the 1st derivative 'dY' of 'Y'. The xx increment is 'dx' and
//! is assumed to be constant
template <template <typename...> class Container, typename Type>
void c1stDerivative(Container<Type> &Y, Container<Type> &dY, Type dx=1.){
 dY.resize(Y.size()); c1stDerivative(Y.data(),dY.data(),Y.size(),dx);
}

/*===========================================================================*/
//! Calculates the 2nd derivative 'ddY' of 'Y'. The xx increment is 'dx'
//! and is assumed to be constant
template <template <typename...> class Container, typename Type>
void c2ndDerivative(Container<Type> &Y, Container<Type> &ddY, Type dx=1.){
 ddY.resize(Y.size()); c2ndDerivative(Y.data(),ddY.data(),Y.size(),dx);
}

/*===========================================================================*/
template <template <typename...> class Container, typename Type>
void cAbs(Container<Type> &tgt){
 for (Type& arg:tgt) if (arg<0) arg*=-1;
}

/*===========================================================================*/
//! ** Contracts an (ascendent) 'axis' domain by 'fact' (]0,1[) around 'center'
//!   with a regular step down to 'step'. Returns true if the axis is changed
//!   and false otherwise.
//! ** The contracted boundaries can be limited by the axis '::front' and
//!   '::back' limits or by 'min' and/or 'max'
template <class Type> bool cContractAxis(
cBuffer<Type> &axis, Type center, Type step, double fact,
Type min=std::numeric_limits<Type>::max(),
Type max=-std::numeric_limits<Type>::max()){
Type range_=axis.back()-axis.front(), step_;
 if (axis.size()<=1 || step*(axis.size()-1)>=range_
  || fact<=0 || fact>=1 ) return false; // safety (keep axis)
 else { //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  range_*=fact, step_=range_/(axis.size()-1);
  Type range2_=range_*0.5, start_=center-range2_;
  min=cMin(min,axis.front()); max=cMax(max,axis.back());
  if (center+range2_>max) start_=max-range_;
  else if (start_<min) start_=min;
  axis.init(start_,step_);
  return step_>step;
} }

/*===========================================================================*/
//! Gets regular 'axis' data/ticks from 'sadata'. This function assumes 'saData'
//! is sorted ascending, eventually repeating periodically and throw otherwise:
//! - (1 2 3 4 5) (1 2 3 4 5 ) (1 2 3 4 5 ) (1 2 3 4 5 ) (1 2 3 4 5 ) ...;
//! - (1 1 2 2 3 3 4 4 5 5 1 1 2 2 3 3 4 4 5 5) (1 1 2 2 3 3 4 4 5 5 ... ) ...;
//! - (1 1 1 2 2 2 3 3 3 4 4 4 5 5 5 1 1 1 2 2 2 3 3 3 4 4 4 5 5 5) ...;
//! The axis repetition can happen in case of ordering of multiple columns,
//! for instance, in 'cDataLoader::sort(...)'.
template <class Type> void cGetAxis(cBuffer<Type> &saData, cBuffer<Type> &axis){
unsigned sa, ax; // 'sa' -> 'saSata' iterator, 'ax' -> 'axis' iterator
 for (axis.clear(), sa=0; sa<saData.size(); sa++){ // fill 1st axis accurrence
  if (axis.empty() || saData[sa]>axis.back()){ axis.push_back(saData[sa]); }
  else if (saData[sa]<axis.back()) break; } // finish 1st axis accurrence.
 for (ax=0; sa<saData.size(); sa++){ // check further occurrences of the axis +
  if (saData[sa]==axis[ax]) continue; else { //................................
   if (++ax>=axis.size()) ax=0; // finish another accurrence of the axis.
   if (saData[sa]==axis[ax]) continue; else throw CEXCP::Exception
    (">> Axis Not Regular",__FUNCTION__,"Check row "+CUTIL::cString(sa));
} } }

/*===========================================================================*/
//! Loads 'nV' buffers (cBuffer/vector) from 'filename' (column wise). '...'
//! are the 'nV' pointers for the Container buffers. The buffers are '::clear'
//! before starting filling from 'filename'.
template <class Container> unsigned cLoad(std::string fileName, unsigned nV,...){
std::ifstream in(fileName.c_str(),std::ifstream::in);
 if (in.is_open()){ //.........................................................
  va_list vList; unsigned iV; double data;
  cBuffer<Container*> list; list.reserve(nV);
  for (va_start(vList,nV), iV=0; iV<nV; ++iV){ // Load column list.
   list.push_back(va_arg(vList,Container*)); list.back()->clear(); }
  while (!in.eof()){ // Load Data for each column.
   in >> data; if (iV==nV) iV=0; if (!in.fail()) list[iV++]->push_back(data); }
  for (va_end(vList), in.close(), iV=1; iV<nV; ++iV) // Check.
   if (list[0]->size()!=list[iV]->size()) throw CEXCP::Exception
    ("Invalid Operation",__FUNCTION__,"Check Columns size");
   return list[0]->size(); // return ref/success size.
} else throw CEXCP::Exception("File Operation",__FUNCTION__,
  "Fail to Open File: "+fileName);
}

/*===========================================================================*/
//! Return the maximum value of vector 'target' (cBuffer,vector).
template <template <typename...> class Container, typename Type>
Type cMax(Container<Type> &target){
Type max=-std::numeric_limits<Type>::max();
 for (unsigned idx=0; idx<target.size(); ++idx)
  if (target[idx]>max) max=target[idx]; return max;
}

/*===========================================================================*/
//! Return the minimum value of vector 'target' (cBuffer,vector).
template <template <typename...> class Container, typename Type>
Type cMin(Container<Type> &target){
Type min=std::numeric_limits<Type>::max();
 for (unsigned idx=0; idx<target.size(); ++idx)
  if (target[idx]<min) min=target[idx]; return min;
}

/*===========================================================================*/
//! Saves 'nV' buffers (cBuffer/vector) from 'filename' (column wise). '...'
//! are the 'nV' pointers for the Container buffers.
template <class Container> void cSave(std::string fileName, unsigned nV,...){
std::ofstream out(fileName.c_str(),std::ofstream::out);
 if (out.is_open()){ //........................................................
  va_list vList; unsigned iV, iData;
  cBuffer<Container*> list; list.reserve(nV);
  for (va_start(vList,nV), iV=0; iV<nV; ++iV)  // Load list columns.
   list.push_back(va_arg(vList,Container*)); va_end(vList);
  for (iV=1; iV<nV; ++iV) if (list[0]->size()!=list[iV]->size()) throw
   CEXCP::Exception("Invalid Operation",__FUNCTION__,"Check Columns size");
  for (iData=0; iData<list[0]->size(); ++iData) // Save data.
   for (iV=0; iV<nV; ++iV){ out << (*list[iV])[iData];
    iV<nV-1? out << "\t": out << std::endl; } out.close();
 } else throw CEXCP::Exception("File Operation",__FUNCTION__,
  "Fail to Create File: "+fileName);
}

/*===========================================================================*/
/// 'prec=0': default precision; 'prec<0': maximum precision for 'Type'
template <class Type, class Size>
std::string cString(cBuffer<Type,Size> &Data, std::string C, int prec=0){
std::ostringstream out; unsigned last=Data.size()-1, idx=0;
 if (prec<0) out << std::setprecision(std::numeric_limits<Type>::digits10);
 else if (prec>0) out << std::setprecision(prec); // else use default value.
 for (; idx<Data.size(); ++idx) if (idx<last) out << Data[idx] << C;
  else out << Data[idx]; return out.str();
}

}

#endif // _BUFFER_ ############################################################
