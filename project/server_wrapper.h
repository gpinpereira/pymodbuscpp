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
	WServer(); 

	void setID(int id){this->id = id;};
	void setPort(int port){this->port = port;};
	void setName(string name){this->name = name;};
	int getID(){ return id; };
    int getPort(){ return port; };
    std::string getName(){ return name; };

	void addChannel(int index, int size, Endian endian);
	void addChannel(int index, int size, Rtype register_type, Dtype data_type, Endian endian);
	void addChannel(Channel *channel);
	void start();

	void updateChannels();
	void OnRequest(unsigned req_length) override;

	vector<Channel*> getChannels(){return channels;};

private:
    vector<Channel*> channels; 
	int port;
	string name;
	int max_register;
	int id;
};



#endif // WServer_H