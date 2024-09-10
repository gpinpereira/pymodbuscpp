/**
 * @file modbus_.h
*/

#ifndef _MODBUS_
#define _MODBUS_

#include <exception_.h>
#include <thread_.h>
#include <math_.h>
#include <buffer_.h>

#include <modbus.h>
#include <sys/socket.h>
#include <netinet/in.h>

//#include <server_wrapper.h>


using namespace CMATH;

namespace CUTIL {

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*                              cModBusServer                                */
/*! \author Francisco Neves                                                  */
/*! \date 2018.03.15 ( Last modified 2019.11.13 )                            */
/*! \brief libModBus Server wrapper                                          */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
//! /details
//! >> Adapted from: https://github.com/stephane/libmodbus/blob/master/tests/...
//! >> Installation of a /usr/local verions of libMODBUS 3.1.4:
//! ** ./configure --prefix=/usr/local/libmodbus-3.1.4; make; make install;
//! ** add to '/etc/ld.so.conf.d' a .conf file (e.g modbus.conf) containing
//!    the line '/usr/local/libmodbus-3.1.4/lib'
//! ** may need to uninstall the system version

/*===========================================================================*/
enum cMODBUSBackend { mbTCP, mbTCP_PI, mbRTU, mbUndefined };

/*===========================================================================*/
class cMODBUSServer: public cThread {
protected: enum cSocketStatus { ssError=-1, ssUndefined=-1 };
private: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    int FSocket;
    int FSelfPipe[2];
    modbus_t *FContext;
    cMODBUSBackend FBackEnd;
    int FRTUServerID, FHeaderLength, FnConnections;
    unsigned FTimeOut;
    uint8_t *FQuery;
    bool FStopped;
    modbus_mapping_t* mb_mapping;
    CMATH::cBuffer<unsigned> FStatus, FTmp;

    int max_register = 0;
    int max_coil = 0;
    int max_input = 0;
    int max_discrete = 0;
protected: //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    inline const uint8_t* query(){ return FQuery; }
    
    inline int headerLength(){ return FHeaderLength; }
    inline int nConnections(){ return FnConnections; }
    inline bool stopped(){ return FStopped; }
    //.........................................................................
    virtual void config();
    virtual void selfPipeTrick(int &fdmax, fd_set &refset);
    virtual void start_connection(sockaddr_in &/*clientaddr*/, int /*socket*/){ }
    void reply(unsigned req_length);
    virtual void end_connection(int /*socket*/){ }
    virtual void close();
    //.........................................................................
    virtual void OnStart();
    virtual void OnExecute();
    virtual void OnStop(){ close(); }
    virtual void OnRequest(unsigned req_length);
public: //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    explicit cMODBUSServer(unsigned timeout_=10);
    virtual ~cMODBUSServer(){ close(); }
    //.........................................................................
    void connect_TCP(std::string ip, int port, int nConnect=1);
    void connect_TCP_PI(std::string node, std::string service, int nConnect=1);
    void connect_RTU(int ID, std::string dev, int b, char p, int dBits, int sBit);
    void disconnect();
    //void setWServer(WServer *iwserver){ wserver = iwserver;};
    //.........................................................................
    inline cMODBUSBackend backend(){ return FBackEnd; }
    inline bool isEnabled(){ return FBackEnd!=mbUndefined; }
    inline unsigned timeout(){return FTimeOut; }
    void setMaxRegister(int value) { max_register = value; }
    void setMaxCoil(int value)     { max_coil = value; }
    void setMaxInput(int value)    { max_input = value; }
    void setMaxDiscrete(int value) { max_discrete = value; }
    // Optional: single-line getters
    int getMaxRegister() const { return max_register; }
    int getMaxCoil() const     { return max_coil; }
    int getMaxInput() const    { return max_input; }
    int getMaxDiscrete() const { return max_discrete; }
    
    modbus_mapping_t* getMapping(){return mb_mapping;};
    std::string getLocalIP(std::string address);
    inline modbus_t* context(){ return FContext; }

    enum cStatus { //......................................................
       stOK=0,                        // reg1: Running, No errors.
       stError=1,                     // reg1: Core error (code=1)
       stOpsTable_Connection=20,      // reg2: Fail to connect to opts table db (code=20).
       stConfigs_Read=21,             // reg2: Fail to Read JSon Configurations (code=21)
       stConfigs_Write=22,            // reg2: Fail to Write JSon Configurations (code=22)
       stRQDispacther=23,             // reg2: Fail to connect/write to RQ database (code=23)
       stIndexDispacther=24,          // reg2: Fail to connect/write to Index database (code=24)
       stConfigs_NoData=30,           // reg2: No data available from DAQ (code=30)
       stMonitoring_TPCHG_Area=100,   // reg2: Mean POD Area to High @ TPC HG (code=100)
       stMonitoring_TPCLG_Area=101,   // reg2: Mean POD Area to High @ TPC LG (code=101)
       stMonitoring_ODHG_Area=102,    // reg2: Mean POD Area to High @ OD HG (code=102)
       stMonitoring_ODLG_Area=103,    // reg2: Mean POD Area to High @ OD LG (code=103)
       stMonitoring_Skin_Area=104,    // reg2: Mean POD Area to High @ Skin (code=104)
       stMonitoring_TPCHG_Rate=110,   // reg2: Mean POD Rate to High @ TPC HG (code=110)
       stMonitoring_TPCLG_Rate=111,   // reg2: Mean POD Rate to High @ TPC LG (code=111)
       stMonitoring_ODHG_Rate=112,    // reg2: Mean POD Rate to High @ OD HG (code=112)
       stMonitoring_ODLG_Rate=113,    // reg2: Mean POD Rate to High @ OD LG (code=113)
       stMonitoring_Skin_Rate=114,    // reg2: Mean POD Rate to High @ Skin (code=114)
       stMonitoring_Flange_Rate=115,  // reg2: Mean POD Rate Outside Acceptable Range @ User Defined Flange (code=115)
       stMonitoring_TrgTPC_Rate=120,              // reg2: DAQ TPC Trigger Rate Outside Acceptable Range (code=120)
       stMonitoring_TrgSkin_Rate=121,             // reg2: DAQ Skin Trigger Rate Outside Acceptable Range (code=121)
       stMonitoring_TrgOD_Rate=122,               // reg2: DAQ OD Trigger Rate Outside Acceptable Range (code=122)
       stMonitoring_TrgGPS_Rate=123,              // reg2: DAQ GPS Trigger Rate Outside Acceptable Range (code=123)
       stMonitoring_TrgExternal_Rate=124,         // reg2: DAQ External Trigger Rate Outside Acceptable Range (code=124)
       stMonitoring_TrgRandom_Rate=125,           // reg2: DAQ Random Trigger Rate Outside Acceptable Range (code=125)
       stMonitoring_TrgGlobalCoincidence_Rate=126,// reg2: DAQ GlobalCoincidence Trigger Rate Outside Acceptable Range (code=126)
       stMonitoring_TPC_SERandom_Rate=130         // reg2: SE Rate from Random Triggers Outside Acceptable Range (code=130)
    };

};

}

#endif // _MODBUS_