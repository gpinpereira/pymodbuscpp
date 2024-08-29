#include "volatile_.h"

using namespace std;
using namespace CEXCP;

namespace CMATH {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cVolatileBase                              */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*=============================================================================*/
unsigned cVolatileBase::FnVars=0;

/*===========================================================================*/
cVolatileBase::cVolatileBase(std::string name_){
 if (name_.empty()) FName=nullptr;
 else FName = new string(name_);
 ++FnVars;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cVolatileBase::~cVolatileBase(){
 delete FName; --FnVars;
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 cVolatiles                                */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
cVolatiles::~cVolatiles(){
 base_buffer::del_ptr();
}

/*===========================================================================*/
void cVolatiles::release(std::string name_){
 for (cVolatileBase *&v_: *this) if (v_->name()==name_){
  delete v_; v_=base_buffer::back();
  base_buffer::pop_back();
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cVolatiles::release(cVolatileBase *v){
 for (cVolatileBase *&v_: *this) if (v_==v){
  delete v_; v_=base_buffer::back();
  base_buffer::pop_back();
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cVolatiles::releaseAll(){
 base_buffer::del_ptr(); base_buffer::clear();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! ** A JSon Example:
//!   ...
//!   "name_varA":{            : name of the cVolatile
//!   		"type":"type_A", : type of var, ie, first in 'FJSonLoader' item.
//!           "par1":""
//!   },
//!   "name_varB":{
//!      "type":"type_B",
//!      "par1":""
//!   }
//!   ...
//! -----------------------------------
//! ** A Usage example:
//!   void load_A(CMATH::cVolatiles &v, nlohmann::json::iterator &j){
//!    std::cout << "name=" << j.key() << " = " << j.value() << std::endl;
//!    v.manage<A>(j.key(),true);
//!   }
//!   void main(){
//!    cVolatiles test;
//!    test.add_JSonAction("type_A",load_A);
//!    test.add_JSonAction("type_B",[](cVolatiles &v, cVolatiles::cJSonItem &j){
//!      std::cout << "name=" << j.key() << " = " << j.value() << std::endl;
//!      v.manage(new cVolatile<B>(j.key,true));
//!    } );
//!    for (CMATH::cVolatileBase *v: vv) std::cout << v->name() << std::endl;
//!   }
#ifdef CJSON_ENABLE //#########################################################
void cVolatiles::loadJSon(nlohmann::ordered_json &from){
std::map<std::string,cJSonAction>::iterator i;
 for (auto it=from.begin(); it!=from.end(); it++){
  if ((i=FJSonActions.find(it.value()[FType]))!=FJSonActions.end()){
   try { i->second(*this,it); } catch (...){
    throw CEXCP::Exception("Fail to load",
     CEXCP::cTypeID(THIS,__FUNCTION__),it.key());
} } } }
#endif //######################################################################

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! See '::loadJSon'.
#ifdef CJSON_ENABLE //#########################################################
void cVolatiles::add_JSonAction(std::string type, cJSonAction action){
 FJSonActions[type]=action; // replaced if existing.
}
#endif //######################################################################

} // CMATH
