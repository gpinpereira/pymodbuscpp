#include "mutex_.h"
#include "string_.h"

#include <unistd.h>

using namespace CEXCP;

namespace CUTIL {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                    cMutex                                 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
cMutex::cConditionalWaiting::cConditionalWaiting()
:FCond(PTHREAD_COND_INITIALIZER),FIsWaiting(false){ }

/*===========================================================================*/
cMutex::cMutex(){ pthread_mutexattr_t mattr; // constructor.
 if (pthread_mutexattr_init(&mattr )!=0) throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),"Fail to initialise mutex attributes object");
 if (pthread_mutex_init(&FMutex,&mattr)!=0) throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),"Fail to initialise mutex object");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cMutex::~cMutex(){ /// throw here will always call terminate() [-Wterminate]
 pthread_mutex_destroy(&FMutex); // !=0 -> Fail to destroy mutex object
}

/*===========================================================================*/
/// Locked by the calling thread.
void cMutex::lock(){ if (pthread_mutex_lock(&FMutex)!=0) throw Exception
 ("Invalid Operation",cTypeID(THIS,__FUNCTION__),"Fail to lock mutex object");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// Unlocked by the calling thread.
void cMutex::unlock(){ if (pthread_mutex_unlock(&FMutex)!=0) throw Exception
 ("Invalid Operation",cTypeID(THIS,__FUNCTION__),"Fail to unlock mutex object");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// ex: 'cMutex::lock(); ... cMutex::waitForSignal(C); ... cMutex::unlock();'<BR>
/// The use of 'cMutex::waitForSignal' must always be used in conjunction with
/// a conditional variable to prevent allowed spurious wakeup ('waiting').
void cMutex::waitForSignal(pthread_cond_t &cond, bool &waiting){
 waiting=true; // set atomically within waitForSignal/sendSignal.
 if (pthread_cond_init(&cond,NULL)!=0) throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),"pthread_cond_init");
 while (waiting) // spurious wakeups ..........................................
  if (pthread_cond_wait(&cond,&FMutex)!=0) throw Exception
   ("Invalid Operation",cTypeID(THIS,__FUNCTION__),"pthread_cond_wait");
 if (pthread_cond_destroy(&cond)!=0) throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),"pthread_cond_destroy");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// ex: 'cMutex::lock(); ... cMutex::waitForSignal(C); ... cMutex::unlock();'
void cMutex::waitForSignal(cConditionalWaiting &cond){
 waitForSignal(cond.FCond,cond.FIsWaiting);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// ex: 'cMutex::lock(); ... cMutex::waitForSignal(C,t); ... cMutex::unlock();'<BR>
/// ** The use of 'cMutex::waitForSignal' must always be used in conjunction with
///   a conditional variable to prevent allowed spurious wakeup ('waiting').
/// ** 't' is absolute time. Use 'CUTIL::cTimeSpec' to get current time.
void cMutex::waitForSignal(pthread_cond_t &cond, timespec &t, bool &waiting){
int r=0; waiting=true; // set atomically within waitForSignal/sendSignal.
 if (pthread_cond_init(&cond,NULL)!=0) throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),"pthread_cond_init");
 while (waiting && r==0) // spurious wakeups ..................................
  if ((r=pthread_cond_timedwait(&cond,&FMutex,&t))!=0 && r!=ETIMEDOUT) throw
   Exception("Invalid Operation",cTypeID(THIS,__FUNCTION__),"pthread_cond_timedwait");
 if (pthread_cond_destroy(&cond)!=0) throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),"pthread_cond_destroy");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// ex: 'cMutex::lock(); ... cMutex::waitForSignal(C,t); ... cMutex::unlock();'<BR>
/// See also 'cMutex::waitForSignal(pthread_cond_t&,timespec&,bool&)';
void cMutex::waitForSignal(cConditionalWaiting &cond, timespec &end){
 waitForSignal(cond.FCond,end,cond.FIsWaiting);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// ex: 'cMutex::lock(); ... cMutex::sendSignal(C); ... cMutex::unlock();'<BR>
/// The use of 'cMutex::sendSigmal' must always be used in conjunction with
/// a conditional variable to prevent allowed spurious wakeup ('waiting').
/// ** In some enviroments (Centos7!!) we need to check if 'waiting'
///   before sending the signal, otherwise it will hang at sendSignal.
void cMutex::sendSignal(pthread_cond_t &cond, bool &waiting){    
 #ifdef MUTEX_SENDSIGNAL_CHECK //##############################################
 if (!waiting) return; // see comments above
 #endif //#####################################################################
 waiting=false; // set atomically within waitForSignal/sendSignal.
 if (pthread_cond_signal(&cond)!=0) throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),"pthread_cond_signal");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/// ex: cMutex::lock(); ... cMutex::sendSignal(C); ... cMutex::unlock();<BR>
/// See also 'cMutex::waitForSignal(pthread_cond_t&,bool&)';
/// ** In some enviroments (Centos7!!) we need to check if 'cond.isWaiting()'
///   before sending the signal, otherwise it will hang at sendSignal.
void cMutex::sendSignal(cConditionalWaiting &cond){
 sendSignal(cond.FCond,cond.FIsWaiting);
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                              cWaitConditional                             */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
cWaitConditional::cWaitConditional()
:cMutex(),FCanWait(true),FWaiting(0){
 if (pthread_cond_init(&FCond,NULL)!=0) throw Exception
  ("Invalid Operation",cTypeID(THIS,__FUNCTION__),"pthread_cond_init");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cWaitConditional::~cWaitConditional(){
 pthread_cond_destroy(&FCond); // !=0 -> Fail to destroy cond object
}

/*===========================================================================*/
//! ex: ::lock(); ... ::waitForSignal(); ... ::unlock();<BR>
//! ** 'end' is absolute time. Use 'CUTIL::cTimeSpec' to get current time.
//! ** Note that in this case the waiting condition can be interrupted if
//!   the timer elapses before the broadcasting.
//! ** Check 'cMutex' for spurious wakeups ('FCanWait').
void cWaitConditional::waitForSignal(timespec &end){ int r=0;
 ++FWaiting; // Count number of threads waiting.
 if (signalCondition()) broadcastSignal(); // see 'cNWaitConditional'
 else while (FCanWait && r==0)// spurious wakeups .............................
  if ((r=pthread_cond_timedwait(&FCond,&mutex(),&end))!=0 && r!=ETIMEDOUT)
   throw Exception("Invalid Operation",cTypeID(THIS,__FUNCTION__),
    "pthread_cond_timedwait ");
 if (--FWaiting==0){ // after 'signalCondition' or user broadcast .............
  clearCondition(); FCanWait=true; }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! ex: ::lock(); ... ::waitForSignal(); ... ::unlock();<BR>
//! ** Check 'cMutex' for spurious wakeups FCanWait.
void cWaitConditional::waitForSignal(){
 ++FWaiting; // Count number of threads waiting.
 if (signalCondition()) broadcastSignal(); // see 'cNWaitConditional'
 else while (FCanWait) // spurious wakeups ....................................
  if (pthread_cond_wait(&FCond,&mutex())!=0) throw Exception
   ("Invalid Operation",cTypeID(THIS,__FUNCTION__),"pthread_cond_wait");
 if (--FWaiting==0){ // after 'signalCondition' or user boradcast .............
  clearCondition(); FCanWait=true; }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! ex: ::lock(); ... ::broadcastSignal(); ... ::unlock();<BR>
//! ** see '::FCanWait' when sending manually a wakeup signal.
void cWaitConditional::broadcastSignal(){
 FCanWait=false; // set atomically within clear/sendSignal.
 if (pthread_cond_broadcast(&FCond)!=0) throw Exception
  ("Invalid Operation",cTypeID(THIS,__FUNCTION__),"pthread_cond_broadcast");
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                              cNWaitBarrier                                */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
cNWaitBarrier::cNWaitBarrier(unsigned n){ int err;
 if ((err=pthread_barrier_init(&FBarrier,NULL,n))!=0) throw Exception
  ("Fail to Initialize the Handler",__FUNCTION__,CUTIL::cString(err));
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cNWaitBarrier::~cNWaitBarrier(){
 pthread_barrier_destroy(&FBarrier); // !=0 -> Fail to destroy barrier object
}

/*===========================================================================*/
//! When the required number of threads have called '::waitForN' specifying the
//! barrier, the constant PTHREAD_BARRIER_SERIAL_THREAD shall be returned to one
//! unspecified thread and zero shall be returned to each of the remaining threads.
//! At this point, the barrier shall be reset to the state it had as a result
//! of the most recent pthread_barrier_init() function that referenced it, meaning
//! the barrier can be reused (fo the same number of participating threads).
void cNWaitBarrier::waitForN(){ int err;
 if ((err=pthread_barrier_wait(&FBarrier))!=0
  && err!=PTHREAD_BARRIER_SERIAL_THREAD) throw Exception
   ("Fail to Wait on Barrier",__FUNCTION__,CUTIL::cString(err));
}

}
 
