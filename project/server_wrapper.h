#ifndef WServer_H
#define WServer_H

#include "channel.h"
#include <modbus_.h>
#include <vector>  

using namespace CUTIL;
using namespace std;

class Channel;

class WServer: public CUTIL::cMODBUSServer {

public:
    WServer(int iport); 

	void addChannel(int index, int size, Endian endian);
	void addChannel(int index, int size, Rtype register_type, Dtype data_type, Endian endian);
	
	void start();

	void updateChannels();
	void OnRequest(unsigned req_length) override;

private:
    vector<Channel> channels; 
	//CUTIL::cMODBUSServer *mb_server;
	int port;
	int max_register;
};



#endif // WServer_H