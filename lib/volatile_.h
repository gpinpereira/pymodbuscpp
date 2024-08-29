#ifndef _VOLATILE_ //##########################################################
#define _VOLATILE_

#include "exception_.h"
#include "buffer_.h"

#include <string>

#ifdef CJSON_ENABLE //#########################################################
#include <map>
#include <functional>
#include "json.hpp"
#endif //######################################################################

namespace CMATH {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cVolatileBase                              */
/*! \author Francisco Neves                                                  */
/*! \date 2020.10.01 ( Last modified 2020.10.01 )                            */
/*! \brief Parameters (name, etc) to keep track of volatiles                 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! See 'cVolatile' and 'cVolatiles'.
class cVolatileBase {
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    std::string *FName;
    static unsigned FnVars;
    cVolatileBase(cVolatileBase&){ } //> Disable
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cVolatileBase(std::string name_);
    virtual ~cVolatileBase();
    //.........................................................................
    static unsigned nVolatiles(){ return FnVars; }
    inline std::string name(){ return FName?(*FName):""; }
    inline bool has_name(){ return FName; }
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                  cVolatile                                */
/*! \author Francisco Neves                                                  */
/*! \date 2020.10.01 ( Last modified 2021.05.26 )                            */
/*! \brief Holds a variable that can be shared non-locally                   */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! ** In case the managed entity ('val') is a pointer, it will automatically
//!   be deleted at '~cVolatile' if 'autodelptr==true'. Otherwise the user is
//!   responsible for calling delete for 'val' whenever relevant.
//! ** For non-pointer types 'autodelptr' has no effect (see: FDelete).
//! ** 'cVolatile' and 'cVolatiles' (manager) is particularly useful to pass
//!   parameters/references to lambda functions with a life time longer than
//!   the scope of the function where they are created, for instance.
//! ** See an example at LZ UPM (outputdock_).
template <class T>
class cVolatile: public CMATH::cVolatileBase {
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    bool FAutoDelPtr;
    //.........................................................................
    template <typename ot = T> // required for SFINAE
    inline typename std::enable_if<std::is_pointer<ot>::value, void>::type
     FDelete (ot &ptr){ delete ptr; }
    //.........................................................................
    template <typename ot = T> // required for SFINAE
    inline typename std::enable_if<!std::is_pointer<ot>::value, void>::type
     FDelete (ot&){ } // dft. for all non-pointer types (compl. for pointers).
    //.........................................................................
    cVolatile(cVolatile&){ } //> disable.
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cVolatile(std::string name_, bool autodelptr_);
    template <class T_> cVolatile(std::string name_, bool autodelptr_, T_ value_);
    explicit cVolatile():cVolatileBase(""),FAutoDelPtr(true){ }
    virtual ~cVolatile(){ if (FAutoDelPtr) FDelete(val); }
    //.........................................................................
    inline cVolatile& operator=(T value_){ val=value_; return *this; }
    inline T& operator*(){ return val; }
    inline operator T&(){ return val; }
    T val;
};

/*===========================================================================*/
template <class T> cVolatile<T>
::cVolatile(std::string name_, bool autodelptr_)
:cVolatileBase(name_),FAutoDelPtr(autodelptr_){ }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! 'T_' is something that 'T' constructor can take (May need to force type).
template <class T> template <class T_> cVolatile<T>
::cVolatile(std::string name_, bool autodelptr_, T_ value_)
:cVolatileBase(name_),FAutoDelPtr(autodelptr_),val(value_){ }


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 cVolatiles                                */
/*! \author Francisco Neves                                                  */
/*! \date 2020.10.01 ( Last modified 2021.11.12 )                            */
/*! \brief Holds a set of cVolatile variables                                */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! ** 'cVolatiles' make sure all managed 'cVolatiles' are properly disposed.
//! ** To ensure that a name is unique (and eventually change it) use '::get'
//!   before using one of the '::manage' procedures
class cVolatiles: public cBuffer<cVolatileBase*>{
protected: typedef cBuffer<cVolatileBase*> base_buffer;
#ifdef CJSON_ENABLE //#########################################################
public: using cJSonItem=nlohmann::ordered_json::iterator;
public: using cJSonAction=std::function<void(cVolatiles&, cJSonItem&)>;
#endif //######################################################################
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    #ifdef CJSON_ENABLE //#####################################################
    std::string FType="type";
    std::map<std::string,cJSonAction> FJSonActions;
    #endif //##################################################################
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cVolatiles(){ }
    virtual ~cVolatiles();
    #ifdef CJSON_ENABLE //#####################################################
    explicit cVolatiles(std::string type):FType(type){ }
    void loadJSon(nlohmann::ordered_json &from);
    inline void clear_JSonActions(){ FJSonActions.clear(); }
    void add_JSonAction(std::string type, cJSonAction action);
    #endif //##################################################################
    inline unsigned size(){ return base_buffer::size(); }
    template <class T> cVolatile<T>* get(std::string name_);
    inline void manage(cVolatileBase *v){ base_buffer::push_back(v); }
    template <class T> cVolatile<T>& manage(std::string name_,bool autodelptr);
    template <class T> cVolatile<T>& manage();
    void release(std::string name_);
    void release(cVolatileBase *v);
    void releaseAll();
};

/*===========================================================================*/
//! return null if 'name_' do not exist or have the wrong type.
template <class T> cVolatile<T>* cVolatiles::get(std::string name_){
 for (cVolatileBase *v: *this) if (v->has_name() && v->name()==name_)
  return dynamic_cast<cVolatile<T>*>(v); return nullptr;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! myset.manage<int>("test",true)=10;
template <class T> cVolatile<T>&
cVolatiles::manage(std::string name_, bool autodelptr){
cVolatile<T> *tmp = new cVolatile<T>(name_,autodelptr);
 manage(tmp); return *tmp;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! myset.manage<int>()=10;
template <class T> cVolatile<T>& cVolatiles::manage(){
cVolatile<T> *tmp = new cVolatile<T>();
 manage(tmp); return *tmp;
}

} // CMATH

#endif // _VOLATILE_ ##########################################################
