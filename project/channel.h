#ifndef Channel_H
#define Channel_H

#include <pybind11/pybind11.h>
#include <modbus_.h>
#include <server_wrapper.h>

namespace py = pybind11;
using namespace CUTIL;

class WServer;

class Channel {

public:
    Channel(int index); 

    void updateMBValue();
    void setBehaviour(char *behaviour_name);
    void setServer(WServer* server);
private:
    py::object behaviour;
    int mapping_index;
    WServer *mb_server;

};

#endif 