#include "thread_.h"

using namespace CEXCP;

namespace CUTIL {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                   cThread                                 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
/// Thread execution function (static).
void* cThread::FMain(void *T){ // Main entry
cThread *trd=(cThread*)T; trd->mainExecute();
 if (trd->FDetach){ trd->delete_detached(); return NULL; }
 else return trd; // trd can be reused.
}

/*===========================================================================*/
/// Thread execution function (object).
/// Note that '::mainExecute' already runs in the child thread.
void* cThread::mainExecute(){ // executes user defined functions.
 lock(); FActive=true; unlock(); OnStart();
 lock(); FReady=true; unlock();  OnExecute();
 lock(); FReady=false; unlock(); OnStop();
 lock(); FActive=false; unlock();
 return this; // see: FMain.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// Change thread policy and priority. Have no effect after 'cThread::execute()'.
void cThread::operator()(int policy_, int priority_){
 FSchPolicy=policy_; FSchPriority=priority_;
}

/*===========================================================================*/
/// Default constructor (joinable)
cThread::cThread():FActive(false),FReady(false),
FDetach(false),FSchPolicy(-1),FSchPriority(-1){ }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// NEED ROOT ACCESS TO CHANGE POLICY/SCHEDULE (See also cSchedule() and nice()).
/// For threads scheduled under one of the normal scheduling policies:<BR>
///  -- SCHED_OTHER [0], SCHED_BATCH [4], SCHED_IDLE [5] sched_priority is
/// not used in scheduling decisions (it must be specified as 0).<BR>
/// Processes scheduled under one of the real-time policies:<BR>
///  -- SCHED_FIFO [1], SCHED_RR [2]) have a sched_priority value in the
/// range 1 (low) to 99 (high),
cThread::cThread(int policy_, int priority_):FActive(false),FReady(false),
FDetach(false),FSchPolicy(policy_),FSchPriority(priority_){ }

/*===========================================================================*/
/// When 'FDetach' is TRUE the class destructor IS invoked after the
/// thread ends and the system ALSO releases any related thread information.<BR>
/// !!!!!! NOTE THAT IN THIS CASE THE THREAD IS NOT JOINNABLE (see wait()) !!!<BR>
/// If changing schedule policy/priority using 'cSchedule()' all threads will
/// inherit and run under the same specifications. To Change threads relative
/// priorities of the threads, do it thread-by-thread.
void cThread::execute(){ sched_param param;
 pthread_attr_init(&FAttrib); FActive=false;
 if (FDetach) pthread_attr_setdetachstate(&FAttrib,PTHREAD_CREATE_DETACHED);
 if (FSchPolicy>=0 && FSchPriority>=0){ sched_param param; //..................
  pthread_attr_setinheritsched(&FAttrib,PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&FAttrib,FSchPolicy);
  pthread_attr_getschedparam (&FAttrib,&param);
  param.sched_priority=FSchPriority;
  pthread_attr_setschedparam (&FAttrib,&param);
 } //..........................................................................
 if (pthread_create(&FThread,&FAttrib,&cThread::FMain,this)!=0) throw
  Exception("Invalid Operation","cThread::execute()","Fail to start thread");
 // Update with values which were actually set ................................
 pthread_getschedparam(FThread,&FSchPolicy,&param);
 FSchPriority=param.sched_priority;
 pthread_attr_destroy(&FAttrib);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// Waits for the thread to finisih and join. Thread must not be detached!!
void cThread::wait(){
 if (FDetach) return; // if thread is FDetach is not joinable!
 if (pthread_join(FThread,nullptr)!=0) throw Exception("Invalid Operation",
  "cThread::wait()","Fail waiting for thread termination");
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cFuncThread                                */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
cFunctionThread::cFunctionThread(bool detach_):cThread(){
 if (detach_) create_detached();
}

/*===========================================================================*/
//! Use nullptr if no 'cEvent': '::(nullptr,[](cFuncThread&){ },nullptr)'
void cFunctionThread::operator ()(cEvent start_, cEvent exec_, cEvent stop_){
 if (active()) throw CEXCP::Exception("Invalid Operation",
  CEXCP::cTypeID(THIS,__FUNCTION__),"cThread must be inactive.");
 FOnStart=start_; FOnExecute=exec_; FOnStop=stop_;
}

}
 
