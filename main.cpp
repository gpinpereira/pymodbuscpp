#include "modbus_.h"
#include <iostream>
//using namespace std;
using namespace CUTIL;


int main(){

    std::cout << "Running new" << std::endl;


    CUTIL::cMODBUSServer *server;
    server = new CUTIL::cMODBUSServer();

    std::cout << "fri" << std::endl;

    std::string sc_address=server->getLocalIP("127.0.0.1"); // see: auto=dev
    std::cout << sc_address << std::endl;

    server->connect_TCP(sc_address, 502, 2);
    std::cout << "bender" << std::endl;
    
    server->execute();

    server->wait();
    std::cout << "fiished" << std::endl;

    //server->stop();
    return 0;
}