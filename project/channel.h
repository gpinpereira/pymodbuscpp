#ifndef Channel_H
#define Channel_H

#include <pybind11/pybind11.h>
#include <modbus_.h>
#include <vector>

namespace py = pybind11;
using namespace CUTIL;

class WServer;


enum Dtype {
    FLOAT=0,
    INTEGER=1,
    SHORT=2,
    BOOL=3
};

enum Rtype{
    HOLDINGREGISTER, //tab_registers 
    INPUTREGISTER, //tab_input_registers 
    COIL, //tab_bits 
    DESCRETEINPUT //tab_input_bits 
};

enum Endian{
    BIG,
    LITTLE
};



class Channel {

public:


    Channel(int first_register, int n_registers, Rtype register_type, Dtype data_type, Endian endian); 

    int getStartingRegister(){return reg_start;};
    int getTotalRegister(){return reg_n;};
    Rtype getRegisterType(){return rtype;};

    void getBehaviourValue();
    void setBehaviour(char *behaviour_name);
    void setServer(WServer* server);
    void setBehaviourValue(std::vector<uint16_t> registers);
private:
    py::object behaviour;
    int reg_start;
    int reg_n;
    Dtype dtype;
    Rtype rtype;
    Endian endiantype;
    WServer *mb_server;

    void setRegister(int reg, uint16_t value);

};

#endif 