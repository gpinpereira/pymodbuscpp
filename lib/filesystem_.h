/**
 * @file filesystem_.h
 */

#ifndef _FILESYSTEM_ //########################################################
#define _FILESYSTEM_

#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <cstring>
#include <limits.h> // integer types
#include <fnmatch.h>
#include <pwd.h>

#include <filesystem>
#include <functional>

#include "exception_.h"
#include "chainlink_.h"
#include "chainbuffer_.h"
#include "string_.h"

namespace COSYS {

#define DEF_FILE_PER (S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)

class cListDir;
class cLockFile;
class cMonitorFS;

// Directory handling functions.
std::string cAbsolutePath    (std::string);
std::string cAbsolutePath    (std::string, std::string);
std::string cChangeFileExt   (std::string, std::string);
std::string cChangeFileDir   (std::string, std::string);
bool        cCheckFileExt    (std::string, std::string);
std::string cConcatePaths    (std::string, std::string, char sep='/');
void        cCopyFile        (std::string, std::string);
std::string cCreateDir       (std::string, std::string, mode_t mode=DEF_FILE_PER);
void        cCreateDirRec    (std::string);
void        cCreateHardLink  (std::string, std::string);
void        cCreateSimLink   (std::string, std::string);
std::string cExtractBaseDir  (std::string, std::string&, bool root=true);
std::string cExtractFileExt  (std::string);
std::string cExtractFileDir  (std::string);
std::string cExtractFileName (std::string);
bool        cFileExists      (std::string);
int         cGetFileSize     (std::string);
std::string cGetWorkingDir   ();
void        cListDirRec      (std::string, std::string, std::string, cStringList&, bool verbose=false);
void        cListDirRec      (std::string, std::string, std::string, std::function<void(cListDir&)>&);
std::string cRelativePath    (std::string, std::string);
int         cRemoveDir       (std::string, bool, std::string filter="");
bool        cRemoveFile      (std::string);
bool        cRenameFile      (std::string, std::string);

// IO handling functions.
std::string cCurrentUser     ();
int         cCurrentUserID   ();
std::string cGetHomeDir      ();
int         cUserID          (std::string);


/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cListDir                                   */
/*! \author Francisco Neves                                                  */
/*! \date 2006.06.27 ( Last modified 2022.03.24 )                            */
/*! \brief Directory handler                                                 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/// \details
/// Supplies a easy way to list and iterate through a directory content.
class cListDir{
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    DIR* FHandle;
    dirent* FEntry;
    std::string FCurrent;
    cListDir(cListDir&){ } // disable copy constructor.
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cListDir(std::string aName);
    cListDir():FHandle(NULL),FEntry(NULL){ }
    ~cListDir(){ if (FHandle) closedir(FHandle); }
    //.........................................................................
    void close();
    void open(std::string aName);
    inline void seek(long pos){ seekdir(FHandle,pos); }
    inline long tell(){ return telldir(FHandle); }
    inline void rewind(){ rewinddir(FHandle); }
    inline std::string current(){ return FCurrent; }
    inline dirent* read(){ return FEntry=readdir(FHandle); }
    inline std::string entry_name(){ return FEntry->d_name;} // see: is_entry.
    inline std::string full_name(){ return cChangeFileDir(entry_name(),FCurrent); }
    inline int entry_type(){ return FEntry->d_type; } // see: is_entry.
    //.........................................................................
    inline bool is_open(){ return FHandle; }
    inline bool is_entry(){ return FEntry; }
    inline bool is_entry_dot(){ return strcmp(FEntry->d_name,".")==0; }
    inline bool is_entry_dotdot(){ return strcmp(FEntry->d_name,"..")==0; }
    inline bool is_entry_dots(){ return (is_entry_dot() || is_entry_dotdot()); }
    inline bool is_entry_chardev(){ return FEntry->d_type==DT_CHR; } // e.g. dev/tty*
    inline bool is_entry_file(){ return FEntry->d_type==DT_REG; }
    inline bool is_entry_fifo(){ return FEntry->d_type==DT_FIFO; }
    inline bool is_entry_sock(){ return FEntry->d_type==DT_SOCK; }
    inline bool is_entry_link(){ return FEntry->d_type==DT_LNK; }
    inline bool is_entry_block(){ return FEntry->d_type==DT_BLK; }
    inline bool is_entry_dir(){ return FEntry->d_type==DT_DIR; }
    inline bool is_unknown(){ return FEntry->d_type==DT_UNKNOWN; }
    inline bool is_whiteout(){ return FEntry->d_type==DT_WHT; }
    long count_entries(std::string format="", bool count_dots=true);
    long get_entries(std::string format, cStringList& matches, unsigned max=UINT_MAX);
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                  cLockFile                                */
/*! \author Francisco Neves                                                  */
/*! \date 2020.10.28 ( Last modified 2020.20.28 )                            */
/*! \brief Manage a lock file.                                               */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! ** Creates a file containning the pid of the process and an additional
//!   tag (optional). The file is deleted when the object is destroyed only
//!   if it exits normally. If there is a crash, the file will not be
//!   deleted. This can be used to know that another instance of the same app
//!   is already running (ownLock==false) or that a previous instance of the
//!   same app exit abnormally (previousLockErr==true), if the lock files
//!   exist the the stored pid do not exist anymore.
//! ** Different instances of cLockFile can exist in the same app (thread
//!   lock, for instance) simply by using different file names.
//! ** See also flock (from <sys/file.h> or the command line option below)
//!    (https://manpages.ubuntu.com/manpages/xenial/man1/flock.1.html)
class cLockFile {
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    std::string FFilename;
    bool FOwnLock, FPrevLockErr;
    cLockFile(){ } //> disable.
    int FRead_pid();
    void FWrite_pid();
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cLockFile(std::string filename);
    virtual ~cLockFile();
    //.........................................................................
    inline std::string filename(){ return FFilename; }
    inline bool previousLockErr(){ return FPrevLockErr; }
    inline bool ownLock(){ return FOwnLock; }
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 cMonitorFS                                */
/*! \author Francisco Neves                                                  */
/*! \date 2022.03.25 ( Last modified 2022.03.26 )                            */
/*! \brief monitors the file system using the inotify API                    */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! \details
//! ** Inherit '::watch(inotify_event &event)' to implement any specific
//!  behavior namely based on the 'event.mask' or 'event.name', e.g:
//!  -- 'event.mask & IN_CREATE (IN_DELETE, ...)
//!  -- 'event.mask & IN_ISDIR
//! ** When staging new watches there is no guarantee that before the process
//!  concludes (inotify_add_watch) there is no changes on the new directory,
//!  namely, new files added.
//! -----------------------------------
//! cMonitorFS mon;
//!  mon->add_dirWatch("my_path",IN_MODIFY|IN_MOVE|IN_CREATE|IN_DELETE,true);
//!  mon->start();
//!  ...
//!  mon->wait();
//!  ...

#ifdef CTHREAD_ENABLE //#######################################################

class cMonitorFS {
public: class cWatchNode;
protected: class cINotify;
protected: class cEventDispatcher;
public: using cWatchNodes=std::vector<cWatchNode>;
protected: using cWatchBlock=CMATH::cChainLinkObject<CMATH::cBuffer<char>>;
protected: using cWatchStream=CUTIL::cMutexType<CMATH::cChainBuffer<cWatchBlock>>;
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cINotify *FINotifyWorker;
    cEventDispatcher *FEventWorker;
    cWatchStream FWatchStream;
    CUTIL::cMutexType<std::vector<cWatchNode>> FWatches, FStaged;
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    cWatchNode& find(unsigned watchd_);
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cMonitorFS();
    virtual ~cMonitorFS();
    //.........................................................................
    void add_fileWatch(std::string path, uint32_t mask);
    void add_dirWatch(std::string path, uint32_t mask, bool rec, bool autodir);
    cWatchNodes watches();
    cWatchNodes staged();
    //.........................................................................
    virtual void onEvent(inotify_event &/*event*/){ }
    cWatchNode find(inotify_event &event);
    //.........................................................................
    void start();
    void stop();
    void wait();
};

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                          cMonitorFS::cWatchNode                           */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
class cMonitorFS::cWatchNode{
public: enum cType { wtUnknown=0, wtFile=1, wtDir=2 };
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    int FWatchd=-1; // inotify_add_watch.
    cType FType=wtUnknown;
    uint32_t FMask=0; // see inotify.h
    std::string FPath; // dir or file
    bool FWatchCreated=false; // dirs only
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    friend class cMonitorFS;
    cWatchNode(int d_):FWatchd(d_){ } // see cMonitorFS::find.
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cWatchNode(){ }
    // Search .................................................................
    inline bool operator==(cWatchNode &r){ return FWatchd==r.FWatchd; }
    inline bool operator<=(cWatchNode &r){ return FWatchd<=r.FWatchd; }
    inline bool operator>=(cWatchNode &r){ return FWatchd>=r.FWatchd; }
    inline bool operator<(cWatchNode &r){ return FWatchd<r.FWatchd; }
    inline bool operator>(cWatchNode &r){ return FWatchd>r.FWatchd; }
    //.........................................................................
    inline bool watch_created(){ return FWatchCreated; }
    inline std::string path(){ return FPath; }
    inline uint32_t mask(){ return FMask; }
    inline int watchd(){ return FWatchd; }
    inline cType type(){ return FType; }
};

#endif // CTHREAD_ENABLE ######################################################

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

}

#endif // _FILESYSTEM_ ########################################################
