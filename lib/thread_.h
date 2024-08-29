/**
 * @file thread_.h
 */

#ifndef _THREAD_ //############################################################
#define _THREAD_

#include "mutex_.h"

#include <functional>

namespace CUTIL {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                   cThread                                 */
/*! \author Francisco Neves                                                  */
/*! \date 2008.02.19 ( Last modified 2020.04.04 )                            */
/*! \brief Thread wrapper class                                              */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/// \details
/// <BR><BR><B>Example:<BR></B> @include cThread.cs
/// ** Detached threads MUST always be created using operator new! Otherwise the
/// thread destructor will be invoked twice: once the thread finish running and
/// (automatically) at the end of the scope where the object was defined!!
/// ** Inherite '::delete_detached' to implement specific actions when deleting
/// a detached thread. This is mostly useful if requiring 'QObject::deleteLater'
/// to handle signal quewing between different threads (check: cASyncHTTPSocket).
class cThread: public cMutex{
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    pthread_t FThread;
    pthread_attr_t FAttrib;
    bool FActive, FReady, FDetach;
    int FSchPolicy, FSchPriority;
    static void *FMain(void *T);
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    void* mainExecute(); // thread code.
    void operator()(int policy_, int priority_);
    inline void create_detached(){ if (!FActive) FDetach=true; }
    virtual void delete_detached(){ delete this; } // default
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    virtual ~cThread(){ }
    explicit cThread();
    explicit cThread(int policy_, int priority_);
    //.........................................................................
    inline int policy(){ return FSchPolicy; } ///< Thread scheduling policy
    inline int schedule(){ return FSchPriority; } ///< Thread scheduling priority
    inline bool detached(){ return FDetach; } ///< Thread is not joinable (see wait()).
    bool active(){ bool ret; lock(); ret=FActive; unlock(); return ret; }
    bool ready(){ bool ret; lock(); ret=FReady; unlock(); return ret; }
    //.........................................................................
    virtual void OnStart()=0;   ///< Inherit to implement.
    virtual void OnExecute()=0; ///< Inherit to implement.
    virtual void OnStop()=0;    ///< Inherit to implement.
    void execute();
    void wait();
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                               cFunctionThread                             */
/*! \author Francisco Neves                                                  */
/*! \date 2020.04.22 ( Last modified 2020.06.22 )                            */
/*! \brief Timed thread wrapper class                                        */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! Supply a simple interface to use lambdas as '::OnStart', '::OnExecute' and
//! '::OnStop' events to avoid inherit 'cThread' for simple tasks, e.g:
//! cFunctionThread test();
//! test( // outpts the current status.
//!   [](cFunctionThread&){ cout << "init" << endl << flush; },
//!   [](cFunctionThread&){ cout << "execute" << endl << flush; },
//!   [](cFunctionThread&){ cout << "end" << endl << flush; } );
//! test.execute(); test.wait();
class cFunctionThread: public CUTIL::cThread {
public: typedef std::function<void(cFunctionThread&)> cEvent;
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cEvent FOnStart, FOnExecute, FOnStop;
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    void OnStart(){ if (FOnStart) FOnStart(*this); }
    void OnExecute(){ if (FOnExecute) FOnExecute(*this); }
    void OnStop(){  if (FOnStop) FOnStop(*this); }
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cFunctionThread(bool detach_=false);
    virtual ~cFunctionThread(){ }
    //.........................................................................
    void operator()(cEvent start_, cEvent exec_, cEvent stop_);
};

}

#endif // _THREAD_ ############################################################
