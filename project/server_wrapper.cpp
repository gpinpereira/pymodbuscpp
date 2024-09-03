#include <server_wrapper.h>
#include <chrono>
#include <thread>
#include <channel.h>

class Channel;


WServer::WServer(int iport){

    CUTIL::cMODBUSServer();
    port = iport;

    max_register = 2;

}


void WServer::addChannel(int index){

    Channel channel(0);

    channel.setBehaviour("Bsetpoint");
    channel.setServer(this);

    channels.push_back(channel);
}

void WServer::start(){


    std::string address= getLocalIP("127.0.0.1");

    std::cout << "port: " << port << std::endl;
    connect_TCP(address, port, 2);
    execute();

    while (true) {
        // Your code here (this block will run every second)
        updateChannels();
        // Pause execution for 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


    wait();
}


 void WServer::OnRequest(unsigned req_length)  {  // 'override' is optional but recommended for clarity
        std::cout << "THA NEW REQUEST" << req_length << std::endl;
}


void WServer::updateChannels(){

    for(int i=0; i<channels.size(); i++){
        channels[i].updateMBValue();
    }

}