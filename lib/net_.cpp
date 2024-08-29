#include <net_.h>

#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/wireless.h>
#include <ifaddrs.h>
#include <netdb.h>


#define ADDRSIZE 128

using namespace CEXCP;

namespace CUTIL {

/*===========================================================================*/
//! If loopback_dev==true, loopback devices (127.0.0.1) are considered.
bool cIsInterfaceUpAndRunning(std::string interface, bool loopback_dev){
int sock=-1, UpAndRunning; struct ifreq ifr;
 memset( &ifr, 0, sizeof(ifr) );
 strncpy(ifr.ifr_name,interface.c_str(),IFNAMSIZ);
 if ((sock=socket(AF_INET,SOCK_STREAM, 0))==-1) return false;
 if(ioctl(sock,SIOCGIFFLAGS,&ifr)!=-1){
  if (!loopback_dev && (ifr.ifr_flags&IFF_LOOPBACK)==IFF_LOOPBACK) return false;
  UpAndRunning=(ifr.ifr_flags&(IFF_UP|IFF_RUNNING))==(IFF_UP|IFF_RUNNING );
  close(sock); return UpAndRunning;
 } close(sock); return false;
}

/*===========================================================================*/
//! Get list of interfaces
cStringList cGetInterfaces(){
cStringList ifNames; // ret
struct ifaddrs *interfaces=nullptr;
struct ifaddrs *temp_addr=nullptr;
 if (getifaddrs(&interfaces)==0) {
  for (temp_addr=interfaces; temp_addr; temp_addr = temp_addr->ifa_next){
   if (temp_addr->ifa_addr->sa_family==AF_INET)
    ifNames.push_back(temp_addr->ifa_name);
 } } // Loop through linked list of interfaces
 freeifaddrs(interfaces);
 return ifNames;
}

/*===========================================================================*/
//! Check if an interface is wireless
bool cIsInterfaceWireless(std::string interface, std::string *protocol){
int sock = -1; struct iwreq pwrq;
 memset(&pwrq, 0, sizeof(pwrq));
 strncpy(pwrq.ifr_name,interface.c_str(),IFNAMSIZ);
 if ((sock=socket(AF_INET,SOCK_STREAM, 0))==-1) return false;
 // wifi extension protocol version, available only on interfaces for wifi
 if (ioctl(sock,SIOCGIWNAME,&pwrq) != -1) {
  if (protocol) (*protocol)=pwrq.u.name; close(sock); return true; }
 close(sock); return false;
}

/*===========================================================================*/
//! interface = eth0, wlp2s0, etc
std::string cGetIPAddress(std::string interface){
std::string ipAddress;
struct ifaddrs *interfaces=nullptr;
struct ifaddrs *temp_addr=nullptr;
 if (getifaddrs(&interfaces)==0) {
  for (temp_addr=interfaces; temp_addr; temp_addr = temp_addr->ifa_next){
   if (temp_addr->ifa_addr && temp_addr->ifa_addr->sa_family==AF_INET) {
    if (strcmp(temp_addr->ifa_name,interface.c_str())==0){
     ipAddress=inet_ntoa(((struct sockaddr_in*)temp_addr->ifa_addr)->sin_addr);
 } } } } // Loop through linked list of interfaces
 freeifaddrs(interfaces);
 return ipAddress;
}

/*===========================================================================*/
//! Get the IP (e.g 172.168.32.45) for 'domainName' host (e.g my.target.host).
//! ** NOTE that passing directly an IP also works.
std::string cGetHostIpAddress(std::string domainName){
char addrstr[ADDRSIZE]; void* ptr;
struct addrinfo hints, *res;
 memset(&hints,0,sizeof(hints));
 hints.ai_family=PF_UNSPEC;
 hints.ai_socktype=SOCK_STREAM;
 hints.ai_flags|=AI_CANONNAME;
 if (getaddrinfo(domainName.c_str(),nullptr,&hints,&res)!=0) return ""; // fail
  for (; res; res = res->ai_next){ //..........................................
   switch (res->ai_family){
    case AF_INET:
     ptr=&((struct sockaddr_in*)res->ai_addr)->sin_addr;
     inet_ntop(res->ai_family,ptr,addrstr,ADDRSIZE); break;
    case AF_INET6:
     ptr=&((struct sockaddr_in6*)res->ai_addr)->sin6_addr;
     inet_ntop(res->ai_family,ptr,addrstr,ADDRSIZE); break;
 } } //........................................................................
 freeaddrinfo(res);
 return addrstr;
}

}
 
