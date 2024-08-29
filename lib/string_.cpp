#include "string_.h"

#include <fstream>
#include <termios.h>
#include <unistd.h>

using namespace std;
using namespace CEXCP;
using namespace CMATH;

namespace CUTIL {

/*===========================================================================*/
//! Concatenates 'list' using 'sep' as separator
std::string cConcate(cStringList &list, string sep){
std::string res; for (string &i: list) res+=((res.empty()?"":sep)+i);
 return res;
}

/*===========================================================================*/
//! Returns a string representation of the current date/time.
string cDateToStr(){ time_t t; time(&t);
 return cReplace(string(ctime(&t)),"\n","");
}

/*===========================================================================*/
//! Returns a string representation 't'
string cDateToStr(time_t &t){
 return cReplace(string(ctime(&t)),"\n","");
}

/*===========================================================================*/
//! Returns a string representation accordingly to 'fmt' of current time.
string cDateToStr(string fmt){
time_t rawtime; struct tm *timeinfo; vector<char> bff(256);
 time(&rawtime); timeinfo=localtime(&rawtime);
 while (strftime(bff.data(),bff.size(),fmt.c_str(),timeinfo)==0)
  bff.resize(bff.size()+256); return bff.data();
}

/*===========================================================================*/
/// The Daylight Saving Time flag (isDayLighSaving) is greater than zero if
/// Daylight Saving Time is in effect, zero if Daylight Saving Time is not
/// in effect, and less than zero if the information is not available.
time_t cDateStrtoTime(string date, string format, int isDayLighSaving){
struct tm tm; if (strptime(date.c_str(),format.c_str(), &tm)==NULL)
 throw Exception("Fail to Parse Time",__FUNCTION__,date);
 tm.tm_isdst = isDayLighSaving; return mktime(&tm);
}

/*===========================================================================*/
//! Returns true if 'str' is empty or contains only whitespace characters
//! and false otherwise (' ', '\\t', '\\n' '\\v', '\\f' and '\\r').
bool cHasOnlySpaces(string &str){
 for (string::const_iterator i=str.begin(); i!=str.end();++i)
  if (!isspace(int(*i))) return false; return true;
}

/*===========================================================================*/
//! Waits for the user to press any key, display a message if 'info==true'
int cPressAnyKey(bool info){
 if (info) cout << "Press Any Key to Continue..." << endl << flush;
 struct termios oldt, newt; int ch;
 tcgetattr( STDIN_FILENO, &oldt ); newt = oldt;
 newt.c_lflag &= ~(ICANON|ECHO );
 tcsetattr( STDIN_FILENO, TCSANOW, &newt );
 ch = getchar(); // read a char
 tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
 return ch;
}

/*===========================================================================*/
//! Reads a password (no echo) from the command line, adding a newline
//! at the end if 'newline==true'.
std::string cReadPassword(bool newline){
struct termios tty; std::string pass;
 tcgetattr(STDIN_FILENO, &tty);
 tty.c_lflag &= ~ECHO; // turn off echo
 tcsetattr(STDIN_FILENO, TCSANOW, &tty);
 std::cin >> pass; // read
 tty.c_lflag |= ECHO; // turn echo back on
 tcsetattr(STDIN_FILENO, TCSANOW, &tty);
 if (newline) std::cout << endl;
 return pass;
}

/*===========================================================================*/
//! Generates a random string containing 'size' 'numeric', 'ALPHA' or 'alpha' chars.
string cRandomString(unsigned size, bool numeric, bool ALPHA, bool alpha){
static cRandom generator; string rText; char ch;
 if (!numeric && !ALPHA && !alpha) throw Exception
  ("Invalid operation",__FUNCTION__,"Check string type selection!");
 while (rText.length()<size){ // randomize.
  ch=char(generator.random()*62.0); // 0..9+A..Z+a..z
  if (ch<10 && numeric) rText.push_back(ch+48); // 0..9
  else if (ch>9 && ch<36 && ALPHA) rText.push_back(ch+55); // A..Z
  else if (ch>35 && alpha) rText.push_back(ch+61); // a-z
 } return rText;  //0..9-A..Z-a..z
}

/*===========================================================================*/
//! Replace for 'to' all ocurrences of 'from' in 'text'.
string cReplace(string text, string from, string to){
int pos=0, fL=from.length(), tL=to.length();
 while ((pos=text.find(from,pos))>=0){
  if (tL>fL) text.insert(pos,tL-fL,0);
  else if (fL>tL) text.erase(pos,fL-tL);
  text.replace(pos,tL,to); pos+=tL;
 } return text;
}

/*===========================================================================*/
//! ** Replaces all occurrences of 'what[...].first' by 'what[...].second' in
//!   'infile' and write it to 'outfile' (which can be the same).
//! ** NOTE that the search is made by line.
//! ** e.g. cReplace("in","out",{{"text1","repl1"},{"test2","repl2"}});
void cReplace(std::string infile, std::string outfile,
const std::vector<std::pair<std::string,std::string>> &what){
std::ifstream in(infile.c_str()); std::stringstream out_;
 if (in.is_open()){ //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  std::string line; size_t start_pos;
  while (std::getline(in, line)){
   for (const std::pair<std::string,std::string> &oc: what){
    start_pos = 0; // start of the line for each 'oc'.
    while((start_pos=line.find(oc.first,start_pos))!=std::string::npos){
    line.replace(start_pos,oc.first.length(),oc.second);
    start_pos+=oc.second.length();
  } } // writes the modified 'line'
  out_ << line << std::endl;
 } } else throw CEXCP::Exception("Fail to Open File",__FUNCTION__,infile);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 in.close(); // for the case 'infile' and 'outfile' are the same.
std::ofstream out(outfile);
 if (out.is_open()){ out << out_.rdbuf(); }
 else throw CEXCP::Exception("Fail to Open File",__FUNCTION__,outfile);
}

/*===========================================================================*/
//! Separate 'text' by 'sep' excluding '[no]sep' and return then in keys.
//! Note that if 'keys' is not empty then the old items will be kept.
void cSeparateValues(vector<string>& keys, string& text, string sep, string no){
unsigned S0=0, S=0, i, ni; // values separated by 'sep' (excluding '[no]sep')
 while ((i=text.find(sep,S))!=(unsigned)string::npos){
  if (!no.empty() && (ni=text.rfind(no,i))!=(unsigned)string::npos)
   if(i==no.length()+ni) goto skip; keys.push_back(text.substr(S0,i-S0));
  S0=i+sep.length(); skip:S=i+sep.length(); // guides.
 } keys.push_back(text.substr(S0,text.length()-S0));
}

/*===========================================================================*/
//! Separate 'text' by 'sep' excluding '[no]sep' and feed the values into
//! the ... list of strings (pointer of strings). Only the 'keys.size()' first
//! ocurrences will be filled if 'keys.size()<n' and only 'n' if 'keys.size()>n'.
unsigned cSeparateValues(string& text, string sep, string no, unsigned n,...){
 if (text.empty()) return 0; else { //+++++++++++++++++++++++++++++++++++++++++
  vector<string> keys; cSeparateValues(keys,text,sep,no);
  unsigned imax=cMin(n,(unsigned)keys.size()), i; va_list vList;
   for (va_start(vList,n), i=0; i<imax; ++i) (*va_arg(vList,string*))=keys[i];
   va_end(vList); return keys.size();
} }

/*===========================================================================*/
//! Alias to sscanf (vsscanf) which will through in case of failure
void cValues(string format, string from, unsigned n, ...){
va_list args; unsigned ret;
 va_start (args, n); // init list of args.
 ret=vsscanf (from.c_str(),format.c_str(),args);
 va_end (args); // close list of args.
 if (ret!=n) throw Exception("Fail to read '"+from+"'",
  __FUNCTION__,format);
}

/*===========================================================================*/
//! convert std::string to lower case.
string cLowerCase(string source){
string target=source; for (auto &ch: target) ch=tolower(ch);
 return target;
}

/*===========================================================================*/
string cUpperCase(string source){ // convert std::string to upper case.
string target=source; for (auto &ch: target) ch=toupper(ch);
 return target;
}

}
