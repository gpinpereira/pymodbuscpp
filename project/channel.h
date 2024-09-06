#ifndef Channel_H
#define Channel_H

#include <pybind11/pybind11.h>
#include <modbus_.h>
#include <server_wrapper.h>

namespace py = pybind11;
using namespace CUTIL;

class WServer;


enum Dtype {
    FLOAT,
    INTEGER,
    SHORT,
    BOOL
};

enum Rtype{
    HOLDINGREGISTER, //tab_registers 
    INPUTREGISTER, //tab_input_registers 
    COIL, //tab_bits 
    DESCRETEINPUT //tab_input_bits 
}

class Channel {

public:
    Channel(int first_register, int n_registers, Dtype data_type); 

    void updateMBValue();
    void setBehaviour(char *behaviour_name);
    void setServer(WServer* server);
private:
    py::object behaviour;
    int reg_start;
    int reg_n;
    Dtype dtype;
    Rtype rtype;
    WServer *mb_server;

};

#endif 