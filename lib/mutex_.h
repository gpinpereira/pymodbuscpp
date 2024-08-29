/**
 * @file mutex_.h
 */

#ifndef _MUTEX_ //#############################################################
#define _MUTEX_ 

#ifdef CENTOS7_COMPAT
    #define MUTEX_SENDSIGNAL_CHECK
#endif

#include "pthread.h"

#include "exception_.h"

namespace CUTIL {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                    cMutex                                 */
/*! \author Francisco Neves                                                  */
/*! \date 2008.02.19 ( Last modified 2022.03.28 )                            */
/*! \brief pThread mutex wrapper class                                       */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/// \details
/// <BR><BR><B>Example:<BR></B> @include cMutex.cs
class cMutex{
public: class cConditionalWaiting { //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        friend class cMutex;
        pthread_cond_t FCond;
        bool FIsWaiting;
    public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        explicit cConditionalWaiting();
        virtual ~cConditionalWaiting(){ }
        bool isWaiting(){ return FIsWaiting; }
    }; //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    pthread_mutex_t FMutex;
    cMutex(cMutex&){ } // disable copy constructor.
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    friend class cThread;
    friend class ctThread;
    inline pthread_mutex_t& mutex(){ return FMutex; }
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cMutex(void);
    virtual ~cMutex(void);
    //.........................................................................
    void lock();
    void unlock();
    //.........................................................................
    void waitForSignal(pthread_cond_t &cond, timespec &end, bool &waiting);
    void waitForSignal(pthread_cond_t &cond, bool &waiting);
    void sendSignal(pthread_cond_t &cond, bool &waiting);
    //.........................................................................
    void waitForSignal(cConditionalWaiting &cond, timespec &end);
    void waitForSignal(cConditionalWaiting &cond);
    void sendSignal(cConditionalWaiting &cond);
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cMutexType                                 */
/*! \author Francisco Neves                                                  */
/*! \date 2018.11.29 ( Last modified 2019.08.28 )                            */
/*! \brief handler for simple objects requiring a lock/unlock                */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! Alias class to associate a mutex to simple types.
//! E.g: cMutexType<int> v1, *v2; *v1=5; [...] **v2=10;
template <class Type> class cMutexType: public cMutex {
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    Type FData; // Inner data containner.
    cMutexType(cMutexType&){ } //> disable.
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cMutexType():cMutex(),FData(){ }
    explicit cMutexType(Type v):cMutex(),FData(v){ }
    virtual ~cMutexType(){ }
    //.........................................................................
    inline Type* operator->(){ return &FData; }
    inline Type& operator*(){ return FData; }
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                            cConditionalWaitType                           */
/*! \author Francisco Neves                                                  */
/*! \date 2019.10.09 ( Last modified 2019.10.10 )                            */
/*! \brief handler for simple objects requiring a lock/unlock                */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! Alias class to associate a mutex and a conditional wait to simple types.
//! E.g: cMutexType<int> v1, *v2; *v1=5; [...] **v2=10;
//! Example ---------------------------
//! typedef CMATH::cChainLinkObject<int> uData;  // user data
//! typedef CMATH::cChainBuffer<uData> mutexObj; // object we want to protect
//! CUTIL::cConditionalWaitType<mutexObj> test;  // ^
//! test.lock(); // protectd acess
//! test->push_back(*(new uData(2)));
//! test.waitForSignal(); // wait here ...
//! test.unlock(); // release access
template <class Type>
class cConditionalWaitType: public cMutexType<Type> {
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cMutex::cConditionalWaiting FCond;
    cConditionalWaitType(cConditionalWaitType&){ } //> disable.
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cConditionalWaitType(){ }
    explicit cConditionalWaitType(Type v):cMutexType<Type>(v){ }
    virtual ~cConditionalWaitType(){ }
    //.........................................................................
    inline Type* operator->(){ return cMutexType<Type>::operator->(); }
    inline Type& operator*(){ return cMutexType<Type>::operator*(); }
    //.........................................................................
    inline void waitForSignal(timespec &end){ cMutex::waitForSignal(FCond,end); }
    inline void waitForSignal(){ cMutex::waitForSignal(FCond); }
    inline void sendSignal(){ cMutex::sendSignal(FCond); }
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                             cWaitConditional                              */
/*! \author Francisco Neves                                                  */
/*! \date 2020.04.20 ( Last modified 2020.09.06 )                            */
/*! \brief Conditional synchronization independent threads                   */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! ** Wrapper for pthread_cond_* functions family.
//! ** Allows to synchronize any number of threads waiting at
//!   '::waitForSignal' until a '::broadcastSignal' is issued by
//!   another thread once some condition is meet.
//! ** Inherite '::signalCondition' to broadcast the signal, independently
//!   of any other external/user condition (see: 'cNWaitCondition').
class cWaitConditional: protected cMutex {
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    bool FCanWait;
    unsigned FWaiting;
    pthread_cond_t FCond;
    cWaitConditional(cWaitConditional&):cMutex(){ } //> disable.
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    virtual void clearCondition(){ }
    virtual bool signalCondition(){ return false; }
    inline unsigned waiting(){ return FWaiting; }
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cWaitConditional();
    virtual ~cWaitConditional();
    inline void clear(){ FCanWait=true; } // reuse.
    //.........................................................................
    inline void lock(){ cMutex::lock(); }
    inline void unlock(){ cMutex::unlock(); }
    void waitForSignal(timespec &end);
    void waitForSignal();
    void broadcastSignal();
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                            cNWaitConditional                              */
/*! \author Francisco Neves                                                  */
/*! \date 2020.09.05 ( Last modified 2020.09.05 )                            */
/*! \brief Synchronization of N independent threads                          */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! ** Similar to 'cNWaitBarrier' but allowing to terminate early by
//!   broadcasting a signal to all threads already waiting.
//! ** Also similarly, can be reused after the condition is either
//!   internaly or by means of an external broadcast signal.
class cNWaitConditional: public cWaitConditional {
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    unsigned FSignalCount;
    bool signalCondition(){ return waiting()==FSignalCount; }
    cNWaitConditional(cNWaitConditional&):cWaitConditional(){ } //> disable
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cNWaitConditional(unsigned n):cWaitConditional(),FSignalCount(n){ }
    virtual ~cNWaitConditional(){ }
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                              cNWaitBarrier                                */
/*! \author Francisco Neves                                                  */
/*! \date 2020.09.01 ( Last modified 2020.09.01 )                            */
/*! \brief Synchronization of N independent threads                          */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! ** Wrapper for pthread_barrier_* functions family.
//! ** NOTE that there is not need for an associated mutex (lock, unlock).
//! ** 'cNWaitBarrier' synchronizes 'n' (defined at constructor) participating
//!   threads. These threads will be blocked at '::waitForN' until the required
//!   number of threads (n) call '::waitForN'.
//! ** There is no method to interrupt a barrier else then '::waitForN.
class cNWaitBarrier{
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    pthread_barrier_t FBarrier;
    cNWaitBarrier(cNWaitBarrier&){ } //> disable
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cNWaitBarrier(unsigned n);
    virtual ~cNWaitBarrier();
    //.........................................................................
    void waitForN();
};

}

#endif // _MUTEX_ #############################################################
