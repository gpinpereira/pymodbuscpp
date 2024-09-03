#include <modbus_.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h> // memset

#include "net_.h"


using namespace CEXCP;
using namespace CMATH;

namespace CUTIL {
//using namespace CUTIL;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                              cModBusServer                                */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/*===========================================================================*/
void cMODBUSServer::OnStart(){
 FStopped=false; FSelfPipe[0]=FSelfPipe[1]=ssUndefined;
}

void cMODBUSServer::OnRequest(unsigned req_length){
  std::cout << "OnRequest " << req_length << std::endl;

}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! 'OnExecute' will block on 'select' until it detects activity on one of the
//! file descriptor sets (including FSockect). '::disconnect' write dummy data
//! to 'FSocket' to unblock 'select'
void cMODBUSServer::OnExecute(){
int rc, fdmax, master_socket; fd_set refset, rdset;
struct timeval tv={0,0};
 // Clear the reference set of socket ; Add the server socket.
 FD_ZERO(&refset); FD_SET(FSocket,&refset);
 fdmax=FSocket; // Keep track of the max file descriptor
 selfPipeTrick(fdmax,refset); // add a self-pipe to safely '::disconnet'


 std::cout << "running " << std::endl;

 //............................................................................
 for (; !FStopped; ){ // Searching for existing connections +++++++++++++++++++
  tv={FTimeOut,0}; // is modified in select (returns remaining time).

  if (select(fdmax+1,&(rdset=refset),nullptr,nullptr,&tv)==-1) continue; // skip
  if (FD_ISSET(FSelfPipe[0], &rdset)) break; // see '::disconnet'
  // Run through existing connections looking for data to be read/new connections
  for (master_socket=0; master_socket<=fdmax && !FStopped; master_socket++){
   if (!FD_ISSET(master_socket,&rdset)) continue;
   if (master_socket==FSocket){ // A client is asking a new connection ........
    socklen_t addrlen; struct sockaddr_in clientaddr; int newfd;
    addrlen=sizeof(clientaddr); memset(&clientaddr,0, sizeof(clientaddr));
    newfd=accept(FSocket,(struct sockaddr*)&clientaddr,&addrlen);
    if (newfd!=-1){ // Handle new connection ..................................
     FD_SET(newfd,&refset); // Add new descriptor to set.
     if (newfd>fdmax) fdmax=newfd; // keep track of maximum.
     start_connection(clientaddr,newfd); // accepted
    } else start_connection(clientaddr,-1); // rejected.
   } else { //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    modbus_set_socket(FContext, master_socket);
    rc=modbus_receive(FContext,FQuery);
    if (rc>0){ 
      OnRequest(static_cast<unsigned>(rc));
      reply(static_cast<unsigned>(rc));
    } // Reply to request.
    else if (rc==-1){ // End connection and remove reference set ..............
     ::close(master_socket); FD_CLR(master_socket,&refset); // Remove from
     if (master_socket==fdmax) fdmax--; // keep track of maximum.
 } } } } // Socket is not shutdown while reading/writing.
}

/*===========================================================================*/
//! Inherit to implement user configuration (e.g modbus_set_debug),
//! registers definition (e.g modbus_mapping_new_start_address,
//! modbus_mapping_new), initializations (modbus_set_bits_from_bytes), etc
void cMODBUSServer::config(){
  modbus_set_debug(context(),FALSE);
  if (modbus_set_slave(context(),0)==-1) throw CEXCP::Exception
    ("Fail to set slave ID",CEXCP::cTypeID(THIS,__FUNCTION__),"modbus_set_slave");
  //mb_mapping = modbus_mapping_new(5,0,0,0);

  mb_mapping = modbus_mapping_new_start_address(0, 0, 0, 0, 0, 2, 0, 0);
  if (mb_mapping==NULL) throw CEXCP::Exception("Failed to allocate the mapping",
    CEXCP::cTypeID(THIS,__FUNCTION__),"modbus_mapping_new");
  //for (unsigned r=0; r<5; ++r) // set
    mb_mapping->tab_registers[0]=42;
    //mb_mapping->tab_registers[1]=22;
    //mb_mapping->tab_registers[r]=false;


  std::cout << "FStatus: " << FStatus.size() << std::endl;
}
void cMODBUSServer::reply(unsigned req_length){
 lock(); //####################################################################

 std::cout << "reply: " << req_length << std::endl;
 modbus_reply(context(),query(),req_length,mb_mapping);
 unlock(); //##################################################################
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Implements the self-pipe trick to be able to safely exit 'select'
void cMODBUSServer::selfPipeTrick(int &fdmax, fd_set &refset){
int flags; // tmp.
 if (pipe(FSelfPipe)==-1) throw Exception("Invalid Operation",
  cTypeID(THIS,__FUNCTION__),"Fail to create self-pipe");
 FD_SET(FSelfPipe[0],&refset); // Add read end of pipe to 'refset'
 fdmax=cMax(fdmax,FSelfPipe[0]+1); // .. adjust 'fdmax' if required
 // make read end non-blocking ................................................
 if ((flags=fcntl(FSelfPipe[0],F_GETFL))==-1) throw Exception
  ("invalid operation",cTypeID(THIS,__FUNCTION__),"fcntl(F_GETFL)");
 if (fcntl(FSelfPipe[0],F_SETFL,flags|=O_NONBLOCK)==-1) throw Exception
  ("invalid operation",cTypeID(THIS,__FUNCTION__),"fcntl(F_SETFL)");
 // make read end non-blocking ................................................
 if ((flags=fcntl(FSelfPipe[1], F_GETFL))==-1) throw Exception
  ("invalid operation",cTypeID(THIS,__FUNCTION__),"fcntl(F_GETFL)");
 if (fcntl(FSelfPipe[1],F_SETFL,flags|=O_NONBLOCK)==-1) throw Exception
  ("invalid operation",cTypeID(THIS,__FUNCTION__),"fcntl(F_SETFL)");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Free all associated memory, sockets, etc.
void cMODBUSServer::close(){
 FBackEnd=mbUndefined; // tag connect[TCP|TCP_IP|RTU].
 if (FSocket!=ssUndefined){ ::close(FSocket); FSocket=ssUndefined; }
 if (FContext){ modbus_close(FContext); modbus_free(FContext); FContext=nullptr; }
 if (FQuery){ free(FQuery); FQuery=nullptr; }
 FHeaderLength=0; FRTUServerID=-1;
 FStopped=true;
}

/*===========================================================================*/
cMODBUSServer::cMODBUSServer(unsigned timeout_):FSocket(ssUndefined),
FContext(nullptr),FBackEnd(mbUndefined),FRTUServerID(-1),FHeaderLength(0),
FTimeOut(timeout_),FQuery(nullptr),FStopped(true){

  Exception::debug=&std::cout;
 }

/*===========================================================================*/
//! Create a context for TCP/IPv4; create and listen a TCP Modbus socket.
void cMODBUSServer::connect_TCP(std::string ip, int port, int nConnect){
 try { 
  
  close(); // reset and create new context ...............................
  if (!(FContext=modbus_new_tcp(ip.c_str(),port))) throw Exception
   ("Fail to create context",cTypeID(THIS,__FUNCTION__),"modbus_new_tcp");
  if (!(FQuery=static_cast<uint8_t*>(malloc(MODBUS_TCP_MAX_ADU_LENGTH)))) throw
   Exception("Fail to allocate memory",cTypeID(THIS,__FUNCTION__),"malloc");

  FHeaderLength=modbus_get_header_length(FContext);

  //...........................................................................
  config(); // user configuration, registers definition, etc

  if ((FSocket=modbus_tcp_listen(FContext,FnConnections=nConnect))==ssError) throw
   Exception(modbus_strerror(errno),cTypeID(THIS,__FUNCTION__),"modbus_tcp_pi_listen");
  
 } catch (...){ close(); throw; }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Create a context for TCP Protocol Independent; ....
void cMODBUSServer::connect_TCP_PI
(std::string node, std::string service, int /*nConnect*/){
 try { close(); // reset and create new context ...............................
  if (!(FContext=modbus_new_tcp_pi(node.c_str(),service.c_str()))) throw Exception
   ("Fail to create context",cTypeID(THIS,__FUNCTION__),"modbus_new_tcp_pi");
  if (!(FQuery=static_cast<uint8_t*>(malloc(MODBUS_TCP_MAX_ADU_LENGTH)))) throw
   Exception("Fail to allocate memory",cTypeID(THIS,__FUNCTION__),"malloc");
  FHeaderLength=modbus_get_header_length(FContext);
  //...........................................................................
  config(); // user configuration, registers definition, etc

  /// ... equivalent to modbus_tcp_listen .... see pages above

 } catch (...){ close(); throw; }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! Create a libmodbus context for RTU; ....
void cMODBUSServer::connect_RTU(int serverID, std::string device,
int baud, char parity, int dataBits, int stopBit){
 try { close(); // reset and create new context ...............................
  if (!(FContext=modbus_new_rtu(device.c_str(),baud,parity,dataBits,stopBit))) throw
   Exception("Fail to create context",cTypeID(THIS,__FUNCTION__),"modbus_new_rtu");
  if (modbus_set_slave(FContext,FRTUServerID=serverID)!=0) throw Exception
   ("Fail to set slave",cTypeID(THIS,__FUNCTION__),"modbus_set_slave");
  if (!(FQuery=static_cast<uint8_t*>(malloc(MODBUS_RTU_MAX_ADU_LENGTH)))) throw
   Exception("Fail to allocate memory",cTypeID(THIS,__FUNCTION__),"malloc");
  FHeaderLength=modbus_get_header_length(FContext);
  config(); // user configuration, registers definition, etc

  /// ... equivalent to modbus_tcp_listen .... see pages above

 } catch (...){ close(); throw; }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//! See '::selfPipeTrick'
void cMODBUSServer::disconnect(){
 FStopped=true; // see loops within '::OnExecute'
 if (FSelfPipe[1]!=ssUndefined) ::write(FSelfPipe[1],"\0",1);
}

std::string cMODBUSServer::getLocalIP(std::string address){
        std::string key, value; // auto=inderface_dev
        if (CUTIL::cSeparateValues(address,"=","",2,&key,&value)==2){
            key.erase(std::remove_if(key.begin(),key.end(),isspace),key.end());
            if (key=="auto"){ //.........................................................
                value.erase(std::remove_if(value.begin(),value.end(),isspace),value.end());
                return CUTIL::cGetIPAddress(value);
            } else return address;
        } else return address;

      return address;
    }


}