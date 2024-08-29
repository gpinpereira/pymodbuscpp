#include "filesystem_.h" 
#include "mutex_.h"
#include "thread_.h"
#include "tthread_.h"
#include "sort_.h"

#include <fstream>

using namespace std;
using namespace CEXCP;
using namespace CMATH;
using namespace CUTIL;

namespace COSYS {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cListDir                                    */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
cListDir::cListDir(string aName):FEntry(NULL){
 FHandle=opendir((FCurrent=aName).c_str());
}

/*===========================================================================*/
void cListDir::close(){
 if (FHandle) closedir(FHandle);
 FHandle=NULL; FEntry=NULL;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cListDir::open(string aName){
 if (FHandle) closedir(FHandle);
 FHandle=opendir((FCurrent=aName).c_str());
 FEntry=NULL;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
long cListDir::count_entries(string format, bool count_dots){
 if (!is_open()) return -1; // nothing to do.
 long rec=tell(), count=0; // record position
 rewind(); while(read()){ // rewind and start counting.
  if (!count_dots && is_entry_dots()) continue;
  if (format.empty() || fnmatch(format.c_str(),FEntry->d_name,FNM_EXTMATCH)==0) count++;
 } seek(rec); return count; // back to original posiion.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
long cListDir::get_entries(string format, cStringList& matches, unsigned max){
 if (!is_open()) return -1; // nothing to do.
 long rec=tell(); rewind(); // record position
 matches.clear(); while(read()){ // rewind and start counting.
  if (fnmatch(format.c_str(),FEntry->d_name,FNM_EXTMATCH)==0){
   matches.push_back(FEntry->d_name); if (matches.size()==max) break;
 } } seek(rec); return matches.size(); // back to original posiion.
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                  cLockFile                                */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
int cLockFile::FRead_pid(){
std::ifstream in(FFilename); int pid=-1;
 if (in.is_open()) in >> pid; in.close();
 return pid;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cLockFile::FWrite_pid(){
std::ofstream out(FFilename); int pid=getpid();
 if (out.is_open()){ out << pid; out.close(); }
 else throw Exception("Fail to Save File",cTypeID(THIS,__FUNCTION__),FFilename);
}

/*===========================================================================*/
cLockFile::cLockFile(std::string filename_):FFilename(filename_){
 if (cFileExists(FFilename)){ // a Lock already exist .........................
  if (getpgid(FRead_pid())<0){ // prev pid do not exist, rewrite the lock.
   FWrite_pid(); FPrevLockErr=FOwnLock=true; }
  else { FPrevLockErr=FOwnLock=false; }
 } else { // no lock exists ...................................................
  FWrite_pid(); FPrevLockErr=false; FOwnLock=true;
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cLockFile::~cLockFile(){
 if (FOwnLock) cRemoveFile(FFilename);
}

#ifdef CTHREAD_ENABLE //#######################################################

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                        cMonitorFS::cEventDispatcher                       */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

class cMonitorFS::cEventDispatcher: public cThread{
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    bool FStopped;
    cMonitorFS *FParent;
    cMutex::cConditionalWaiting FCond;
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    void OnStart();
    void OnExecute();
    void OnStop(){ }
    void processEvents(cWatchBlock &block);
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cEventDispatcher(cMonitorFS &P);
    virtual ~cEventDispatcher(){ }
    //.........................................................................
    inline void sendSignal(){ cThread::sendSignal(FCond); }
    void stop();
};

/*===========================================================================*/
void cMonitorFS::cEventDispatcher::OnStart(){
 lock(); FStopped=false; unlock(); //######################################+1-1
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cMonitorFS::cEventDispatcher::OnExecute(){
bool stopped_; // local
 do { // while not stopped_
  FParent->FWatchStream.lock(); //###########################################+1
  while (!FParent->FWatchStream->empty()){
   cWatchBlock *block = &FParent->FWatchStream->front();
   FParent->FWatchStream->pop_front();
   FParent->FWatchStream.unlock(); //########################################-1
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   processEvents(*block); delete block; // thread safe.
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   FParent->FWatchStream.lock(); //##########################################+1
  } // interlock both mutexes - no corner cases.
  lock(); FParent->FWatchStream.unlock(); //###############################-1+2
  if (!FStopped){ waitForSignal(FCond); stopped_=FStopped; }
  unlock(); //###############################################################-2
 } while (!stopped_);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! ** When staging new watches there is no guarantee that before the process
//!  concludes (inotify_add_watch) there is no changes on the new directory,
//!  namely, new files added.
void cMonitorFS::cEventDispatcher::processEvents(cWatchBlock &block){
cBuffer<char> &block_=*block; // alias
unsigned i=0; // read buffer.
 while (i<block_.size()){
  inotify_event *event=(inotify_event*)(&block_[i]);
  if (event->len){ // safety.
   if (event->mask&IN_ISDIR && // Stage a new wath dir
    (event->mask&IN_CREATE || event->mask&IN_MOVED_TO)){
    FParent->FWatches.lock(); //###############################################
    cMonitorFS::cWatchNode node=FParent->find(event->wd);
    FParent->FWatches.unlock(); //#############################################
    if (node.watch_created()) FParent->add_dirWatch(
     cChangeFileDir(event->name,node.path()),node.mask(),false,true);
   } //--------------------------------
   FParent->onEvent(*event);
  } //---------------------------------
  i+=sizeof(inotify_event)+event->len;
} }

/*===========================================================================*/
//! checks are made every 0.1s
cMonitorFS::cEventDispatcher::cEventDispatcher(cMonitorFS &P)
:cThread(),FStopped(true),FParent(&P){ }

/*===========================================================================*/
void cMonitorFS::cEventDispatcher::stop(){
 lock(); //##################################################################+1
 FStopped=true; cThread::sendSignal(FCond);
 unlock(); //################################################################-1
}

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                           cMonitorFS::cINotify                            */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
class cMonitorFS::cINotify: public ctThread {
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    int FFiled;
    cMonitorFS *FParent;
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    void OnStart();
    void OnTimer();
    void OnStop();
    void watch(cWatchNodes &staged, cWatchNodes &watches);
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cINotify(cMonitorFS &P);
    virtual ~cINotify(){ }
};

/*===========================================================================*/
void cMonitorFS::cINotify::watch(cWatchNodes &staged, cWatchNodes &watches){
 for (cWatchNode &node: staged){
  node.FWatchd=inotify_add_watch(FFiled,node.path().c_str(),node.mask());
  if (node.FWatchd<0) throw Exception( // fatal!
   "Invalid operation",cTypeID(THIS,__FUNCTION__),"inotify_add_watch");
  else watches.push_back(node);
 } //----------------------------------
 std::sort(watches.begin(),watches.end());
 staged.clear(); // all being watched.
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cMonitorFS::cINotify::OnStart(){
 if ((FFiled=inotify_init())<0) throw Exception(
  "Invalid operation",cTypeID(THIS,__FUNCTION__),"inotify_init");
 FParent->FStaged.lock(); FParent->FWatches.lock(); //#####################+1+2
 watch(*(FParent->FStaged),*(FParent->FWatches));
 FParent->FStaged.unlock(); FParent->FWatches.unlock(); //#################+1+2
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cMonitorFS::cINotify::OnTimer(){
 // Check if there is new dirs to watch........................................
 FParent->FStaged.lock(); //#################################################+1
 if (!FParent->FStaged->empty()){ // data to watch
  FParent->FWatches.lock(); //###############################################+2
  watch(*(FParent->FStaged),*(FParent->FWatches));
  FParent->FWatches.unlock(); //#############################################-2
 } FParent->FStaged.unlock(); //#############################################-1
 // Read events from kernel ...................................................
 int toread_=0, read_;
 if (ioctl(FFiled,FIONREAD,&toread_)<0) throw Exception(
  "Invalid operation",cTypeID(THIS,__FUNCTION__),"ioctl");
 if (toread_>0){ //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  cWatchBlock *block = new cWatchBlock();
  (**block).size(toread_); // already available
  read_=read(FFiled,(**block).buffer(),(**block).size());
  if (read_<0){ delete block; // clean block whichis not chainned
   throw Exception("Invalid operation",
    cTypeID(THIS,__FUNCTION__),"ioctl"); }
  (**block).size(read_); // in case it read less: safety  
  FParent->FWatchStream.lock(); //###########################################+3
  FParent->FWatchStream->push_back(*block);
  FParent->FEventWorker->lock(); //##########################################+4
  FParent->FEventWorker->sendSignal(); // if waiting.
  FParent->FEventWorker->unlock(); //########################################-4
  FParent->FWatchStream.unlock(); //#########################################-3
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cMonitorFS::cINotify::OnStop(){
 for (cMonitorFS::cWatchNode &node: *(FParent->FWatches)){
  if (inotify_rm_watch(FFiled,node.FWatchd)<0) throw Exception(
  "Invalid operation",cTypeID(THIS,__FUNCTION__),"inotify_rm_watch");
  else node.FWatchd=-1;
 } //----------------------------------
 close(FFiled);
}

/*===========================================================================*/
//! checks are made every 0.1s
cMonitorFS::cINotify::cINotify(cMonitorFS &P)
:ctThread(0,1E8,true),FParent(&P){ }

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                cMonitor                                   */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
//! Not thread safe! Need FWatches::lock, ::unlock.
cMonitorFS::cWatchNode& cMonitorFS::find(unsigned watchd_){
cSSResult res=cSearchASorted(cWatchNode(watchd_),*FWatches);
 if (res.type==ssExact) return (*FWatches)[res.pos];
 else throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),cString(watchd_));
}

/*===========================================================================*/
cMonitorFS::cMonitorFS():FINotifyWorker(nullptr){
 FEventWorker = new cEventDispatcher(THIS);
 FINotifyWorker = new cINotify(THIS);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cMonitorFS::~cMonitorFS(){
 delete FINotifyWorker; delete FEventWorker;
 FWatchStream.lock(); //#####################################################+1
 if (!FWatchStream->empty()) FWatchStream->del_chain();
 FWatchStream.unlock(); //###################################################-1
}

/*===========================================================================*/
//! mask = IN_MODIFY | IN_CREATE | IN_DELETE
void cMonitorFS::add_fileWatch(std::string path, uint32_t mask){
cWatchNode node; // tmp (thread safe
 node.FType=cWatchNode::wtFile;
 node.FPath=path; node.FMask=mask;
 FStaged.lock(); FStaged->push_back(node); FStaged.unlock(); //############+1-1
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! mask = IN_MODIFY | IN_CREATE | IN_DELETE (see: inotify.h)
void cMonitorFS::add_dirWatch(std::string path,
uint32_t mask, bool rec, bool autodir){
cWatchNode node; // tmp (thread safe)
 node.FType=cWatchNode::wtDir;
 node.FPath=path; node.FMask=mask;
 node.FWatchCreated=autodir; // false by default for files.
 FStaged.lock(); FStaged->push_back(node); FStaged.unlock(); //############+1-1
 if (rec){ //..................................................................
  cListDir d(path);
  if (!d.is_open()) return; // dir do not exist or can't be open.
  while (d.read()) if (d.is_entry_dir() && !d.is_entry_dots())
   add_dirWatch(d.full_name(),mask,true,autodir); // 1 step deeper.
} }

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cMonitorFS::cWatchNodes cMonitorFS::watches(){ cWatchNodes ret;
 FWatches.lock(); ret=*FWatches; FWatches.unlock(); //#####################+1-1
 return ret;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cMonitorFS::cWatchNodes cMonitorFS::staged(){ cWatchNodes ret;
 FStaged.lock(); ret=*FStaged; FStaged.unlock(); //########################+1-1
 return ret;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
cMonitorFS::cWatchNode cMonitorFS::find(inotify_event &event){
cWatchNode ret; FWatches.lock(); ret=find(event.wd); FWatches.unlock(); //#+1-1
 return ret;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cMonitorFS::start(){
 FEventWorker->execute();
 FINotifyWorker->execute();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cMonitorFS::stop(){
 FINotifyWorker->stop();
 FEventWorker->stop();
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void cMonitorFS::wait(){
 FINotifyWorker->wait();
 FEventWorker->wait();
}

#endif // CTHREAD_ENABLE ######################################################

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                                 FUNCTION                                  */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
//! Converts path to an absolute path (see ~ and .).
string cAbsolutePath(string path){
 if ((path.size()==1 && path[0]=='~') ||       // eg. '~'
  (path.size()>1 && path.substr(0,2)=="~/"))   // eg. '~/name...'
   return path.replace(0,1,cGetHomeDir());
 else if ((path.size()==1 && path[0]=='.') ||  // eg. '.'
  (path.size()>1 && path.substr(0,2)=="./"))   // eg. './name...'
   return path.replace(0,1,cGetWorkingDir());
 else if ((path.size()==2 && path=="..") ||    // eg. '..'
  (path.size()>2 && path.substr(0,3)=="../"))  // eg. '../name...'
 return path.replace(0,2,cExtractFileDir(cGetWorkingDir()));
 else if (path.size()>0 && path[0]!='/')       // eg. 'name/...'
   return cGetWorkingDir()+'/'+path;
 else return path;
}

/*===========================================================================*/
//! Returns the absolute path of 'path' relatively to a base dir 'base'.
//! Check also 'cRelativePath(base,path)'.
string cAbsolutePath(string base, string path){
int dBase=*base.rbegin()=='/'?1:0, dPath=*path.begin()=='/'?1:0;
 return base.substr(0,base.size()-dBase)+"/"+path.substr(dPath,path.size()-dPath);
}

/*===========================================================================*/
//! Check file extension.
bool cCheckFileExt(string file, string ext){
 return ext==cExtractFileExt(file);
}

/*===========================================================================*/
//! Concatenates 'path1' and 'path2' into 'path1/path2' (sep='/'), mmaking
//! sure there is only one 'sep' connecting the 2 paths.
string cConcatePaths(string path1, string path2, char sep){
 if (!path1.empty() && !path2.empty()){
  unsigned c1=0, c2=0; // number of separators at end/begin of path1/path2.
  for (auto c=path1.rbegin(); c<path1.rend(); ++c) if (*c!=sep) break; else c1++;
  for (auto c=path2.begin(); c<path2.end(); ++c) if (*c!=sep) break; else c2++;
  return path1.substr(0,path1.length()-c1)+sep+path2.substr(c2,path2.length()-c2);
 } else return path1+path2;
}

/*===========================================================================*/
//! Copy a file (do not support directories) - preserve file permissions.
//! (Since C17) Check 'std::filesystem::copy' in <filesystem>.
void cCopyFile(string from, string to){
ifstream src(from.c_str(),std::ios::binary); if (!src.is_open())
  throw Exception("Fail to open file",__FUNCTION__,from);
ofstream dst(to.c_str(),std::ios::binary); if (!dst.is_open())
 throw Exception("Fail to create file",__FUNCTION__,to);
 dst << src.rdbuf(); if (dst.fail()) throw Exception("Fail to copy file",
  "cCopyFile(string,string)",from+" -> "+to);
 src.close(); dst.close(); // close both files
 struct stat per; // Update File Permissions ++++++++++++++++++++++++++++++++++
 if (stat(from.c_str(),&per)!=0 || chmod(to.c_str(),per.st_mode)!=0) throw
  Exception("Fail to set permissions",__FUNCTION__,to);
}

/*===========================================================================*/
//! Replace 'file' extension by 'ext' (Note that 'ext' do not have the '.'!).
string cChangeFileExt(string file, string ext){
string dir=""; int pdir=file.rfind("/"), pext; // UNIX/LINUX OS.
 if (pdir>=0){ // Equivalent to cExtractFileName
   dir=file.substr(0,pdir+1); // include '/'
   file=file.substr(pdir+1,file.length()-pdir);
 } //..........................................................................
 pext=file.rfind("."); // ext char
 if (pext>=0) file.replace(pext+(ext.empty()?0:1),file.length()-pext,ext);
 else file+=(ext.empty()?"":('.'+ext)); // no ext: put the new one.
 if (dir.empty()) return file; else return dir+file;
}

/*===========================================================================*/
//! Extract the file name from 'file' and change its directory to 'dir'.
string cChangeFileDir(string file, string dir){
 if (dir.empty()) dir+="./"; // relative path.
 else if (dir.back()!='/') dir.push_back('/');
 return dir+cExtractFileName(file); // UNIX/LINUX OS
}

/*===========================================================================*/
//! ** Create a new 'dir' at 'path'. If the directory already exists it does
//!   nothing. If 'name' is not a directory or it cannot be created it throws.
//!   On sucess, this function return the full (path+dir) path.
//! ** Supplly an empty 'path' to use 'dir' as an absolute path.
string cCreateDir(string path, string dir, mode_t mode){
struct stat check; // If existing, permission are not changed!
 if (!path.empty() && path.back()!='/') path.push_back('/'); dir=path+dir;
 if (stat(dir.c_str(),&check)==0){ // node exists ... +++++++++++++++++++++++++
  if (!S_ISDIR(check.st_mode)) // ... but not a dir! Otherwise, nothing to do.
   throw Exception("Invalid Directory",__FUNCTION__,dir);
 } else if (mkdir(dir.c_str(),mode)!=0) // Create a new dir +++++++++++++++++++
  throw Exception("Fail to Create Directory", __FUNCTION__,dir);
 return dir;
}

/*===========================================================================*/
//! Creates dirs recursively to get the full 'path' dir.
void cCreateDirRec(std::string path){ string cdir, ndir;
 while (!(ndir=cExtractBaseDir(path,path,true)).empty()){
  cCreateDir("",cdir+=(cdir.empty()?"":"/")+ndir);
} }


/*===========================================================================*/
//! Creates a hard 'link' to 'target'.
void cCreateHardLink(std::string target, std::string linkname){
 if (link(target.c_str(),linkname.c_str())!=0) throw Exception
  ("Fail to create link",__FUNCTION__,linkname+" -> "+target);
}

/*===========================================================================*/
//! Creates a symbolic 'link' to 'target'.
void cCreateSimLink(std::string target, std::string linkname){
 if (symlink(target.c_str(),linkname.c_str())!=0) throw Exception
  ("Fail to create link",__FUNCTION__,linkname+" -> "+target);
}

/*===========================================================================*/
//! Returns the base directory in 'dir' and the rest of the path in 'top'.
//! If the path starts with '/' (root) then the '/' is preserved.
string cExtractBaseDir(string dir, string &top, bool root){ size_t pos;
 if ((pos=dir.find("/"))!=string::npos){ // found at least 1 '/'.
  top=dir.substr(pos+1,dir.size()-pos-1); // path after the 1st '/'
  if (pos==0) return (root?"/":"")+cExtractBaseDir(top,top,false);
  else return dir.substr(0,pos); // before the 1st '/'
 } else { top=""; return dir; } // no '/'
}

/*===========================================================================*/
//! Extract file extension from 'path'.
string cExtractFileExt(string path){
int position=(path=cExtractFileName(path)).rfind(".");
 if (position>=0) return path.substr(position+1,path.length()-position);
 else return ""; // no extension (.) found after the last '/'
}

/*===========================================================================*/
//! Extract File directory from 'path'.
string cExtractFileDir(string path){
int position=path.rfind("/"); // UNIX/LINUX OS.
 if (position>=0) return path.substr(0,position);
 else return "";
}

/*===========================================================================*/
//! Extract File Name from 'path'.
string cExtractFileName(string path){
int position=path.rfind("/"); // UNIX/LINUX OS.
 if (position>=0) return path.substr(position+1,path.length()-position);
 else return path; // no directory.
}

/*===========================================================================*/
//! Check if a file exists.
bool cFileExists(string filename){ struct stat s;
 if (stat(filename.c_str(),&s)==0) return true; else return false;
}

/*===========================================================================*/
//! Get File size in bytes. Returns -1 if the file do not exist or
//! otherwise fail to stat (e.g permissions).
int cGetFileSize(std::string filename){ struct stat s;
 if (stat(filename.c_str(),&s)==0) return s.st_size; else return -1;
}

/*===========================================================================*/
/// Get the path to the current working directory.
string cGetWorkingDir(){
vector<char> txt(1024); trysize:;
 if (getcwd(txt.data(),txt.size())==NULL){
  if (errno==ERANGE){ txt.resize(txt.size()+1024); goto trysize; }
  else throw Exception("Fail to get working directory","cGetWorkingDir",
   strerror(errno));
 } return txt.data();
}

/*===========================================================================*/
//! ** List all 'file_pattern' files in 'dirName' into 'list' for all recursive
//!   sub-dirs matching the pattern 'subdir_pattern' (The depth of the sub dir
//!   patterns can be any). The 'verbose' allows to check the status/progress
//!   when scanning remote dirs having a very large number os files (e.g. LZ).
//! ** Use '...' at the 'subdir_pattern' to allow all sub-directories
//! ** EXAMPLE 1: 'cListDirRec(dir,"*","*.jpg",list);'
//!   - Reads into all directories at 'dir' level
//! ** EXAMPLE 2: 'cListDirRec(dir,"...","*.jpg",list);' (equal to EXAMPLE 1)
//!   - Reads into all levels directories from 'dir' level
//! ** EXAMPLE 3: cListDirRec(dir,"name1*","*.jpg",list);
//!   - Reads into all 'name*' directories at 'dir' level
//! ** EXAMPLE 4: cListDirRec(dir,"name1*/...","*.jpg",list);
//!   - Reads into all levels directories having 'name*' at the 'dir' level
//! ** EXAMPLE 5: cListDirRec(dir,"*/name1*/name2?/name3/...","*.jpg",list);
//!   - Reads into all levels directories having 'name1*' at the 'dir' level,
//!     'name2?' at next directory level and 'name3' at the next level.
void cListDirRec(string dirName, string subdir_pattern,
string file_pattern, cStringList &list, bool verbose){
cListDir d(dirName); string rPattern, cPattern;
 if (!d.is_open()) return; // dir do not exist or can't be open.
 while (d.read()){ //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (d.is_entry_dir() && !d.is_entry_dots()){ //..............................
   // Extract the pattern for the current directory level. The remaining pattern
   // for deaper directories is maintained ('rPattern') until its empty.
   // Once/if 'rPattern' is empty, all directories are accepted (i.e. '*').
   cPattern=cExtractBaseDir(subdir_pattern,rPattern,false);
   if (cPattern=="...") cPattern="*"; // accept all.
   if (rPattern.empty()) rPattern="..."; // accept all.
   if (fnmatch(cPattern.c_str(),d.entry_name().c_str(),FNM_EXTMATCH)==0)
    cListDirRec(cChangeFileDir(d.entry_name(),dirName),
     rPattern,file_pattern,list,verbose);
  } else if (d.is_entry_file() || d.is_entry_link()){ //.......................
   if (fnmatch(file_pattern.c_str(),d.entry_name().c_str(),FNM_EXTMATCH)==0){
    list.push_back(cChangeFileDir(d.entry_name(),dirName));
    if (verbose) cout << "Adding '" << list.back()
     << "' (" << list.size() << ")" << endl << flush;
} } } }

/*===========================================================================*/
//! Similar to 'cListDirRec(...,cStringList,...)' but calls 'onfile' for each
//! valid entry that is not a dir. This function maybe usefull when iterating
//! over very large directories or when no storage is required.
//! -----------------------------------
//! typedef std::function<void(cListDir&)> cOnFile;
//! cOnFile onFile = [](cListDir &file) { cout << file << endl; ... };
//! cListDirRec("basedir_path","...","*",onFile);
void cListDirRec(string dirName, string subdir_pattern,
string file_pattern, std::function<void(cListDir&)> &onfile){
cListDir d(dirName); string rPattern, cPattern;
 if (!d.is_open()) return; // dir do not exist or can't be open.
 while (d.read()){ //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (d.is_entry_dir() && !d.is_entry_dots()){ //..............................
   // Extract the pattern for the current directory level. The remaining pattern
   // for deaper directories is maintained ('rPattern') until its empty.
   // Once/if 'rPattern' is empty, all directories are accepted (i.e. '*').
   cPattern=cExtractBaseDir(subdir_pattern,rPattern,false);
   if (cPattern=="...") cPattern="*"; // accept all.
   if (rPattern.empty()) rPattern="..."; // accept all.
   if (fnmatch(cPattern.c_str(),d.entry_name().c_str(),FNM_EXTMATCH)==0)
    cListDirRec(cChangeFileDir(d.entry_name(),dirName),
     rPattern,file_pattern,onfile);
  } else { // all other entries ...............................................
   if (fnmatch(file_pattern.c_str(),d.entry_name().c_str(),FNM_EXTMATCH)==0)
    onfile(d); // use "d.is_..." to further constrain entry
} } }

/*===========================================================================*/
//! Returns the relative path of 'path' relatively to a base dir 'base'.
//! Check also 'cAbsolutePath(base,path)'.
string cRelativePath(string base, string path){
int pos=path.find(base); // initial position
 if (pos<0) pos=0; else if (path[pos+=base.size()]=='/') pos++;
 return path.substr(pos,path.size()-pos);
}

/*===========================================================================*/
// Delete tree starting at 'path' (inclusive if 'delRoot').
int cRemoveDir(string path, bool delRoot, string filter){
cListDir dir(path); string name;
 if (dir.is_open()){ while (dir.read()){
   if (dir.is_entry_dots()) continue; // not going back
   if (!filter.empty() && fnmatch(filter.c_str(), // have a filter...
      dir.entry_name().c_str(),FNM_EXTMATCH)!=0) continue; // and it does not match.
   name=cChangeFileDir(dir.entry_name(),path); // full patch.
   if (dir.is_entry_dir()){ if (cRemoveDir(name,true)!=0) return -1; }
   else if (remove(name.c_str())!=0) return -1;
  } return delRoot?remove(path.c_str()):0;
 } else return -1;
}

/*===========================================================================*/
//! Remove/delete file 'filename'.
bool cRemoveFile(string filename){
 return (remove(filename.c_str())==0);
}

/*===========================================================================*/
//! Rename/move 'oldFile' into 'newName'.
//! (Since C17) Check 'std::filesystem::rename' in <filesystem>.
bool cRenameFile(string oldName, string newName){ // mv.
 return rename(oldName.c_str(),newName.c_str())==0?true:false;
}

/*============================================================================*/
//! Returns the username of the current user.
string cCurrentUser(){
struct passwd *pwd; if ((pwd=getpwuid(geteuid()))!=NULL)
 return pwd->pw_name; else return "";
}

/*============================================================================*/
//! Returns the ID of the current user.
int cCurrentUserID(){
struct passwd *pwd; if ((pwd=getpwuid(geteuid()))!=NULL)
 return pwd->pw_uid; else return -1;
}

/*============================================================================*/
//! Get the current user home dir
std::string cGetHomeDir(){
struct passwd *pwd; if ((pwd=getpwuid(geteuid()))!=NULL)
 return pwd->pw_dir; else return "";
}

/*============================================================================*/
//! Returns the ID of the user 'name'
int cUserID(string name){
struct passwd *pwd; if ((pwd=getpwnam(name.c_str()))!=NULL)
 return pwd->pw_uid; else return -1;
}

}
