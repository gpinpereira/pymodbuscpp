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





// Convert Dtype enum to string
inline std::string DtypeToString(Dtype dtype) {
    switch (dtype) {
        case FLOAT: return "FLOAT";
        case INTEGER: return "INTEGER";
        case SHORT: return "SHORT";
        case BOOL: return "BOOL";
        default: return "Unknown Dtype";
    }
}

// Convert Rtype enum to string
inline std::string RtypeToString(Rtype rtype) {
    switch (rtype) {
        case HOLDINGREGISTER: return "HOLDING_REGISTER";
        case INPUTREGISTER: return "INPUT_REGISTER";
        case COIL: return "COIL";
        case DESCRETEINPUT: return "DESCRETE_INPUT";
        default: return "Unknown Rtype";
    }
}

// Convert Endian enum to string
inline  std::string EndianToString(Endian endian) {
    switch (endian) {
        case BIG: return "BIG";
        case LITTLE: return "LITTLE";
        default: return "Unknown Endian";
    }
}

// Convert string to Dtype enum
inline Dtype stringToDtype(const std::string& str) {
    if (str == "FLOAT") return FLOAT;
    else if (str == "INTEGER") return INTEGER;
    else if (str == "SHORT") return SHORT;
    else if (str == "BOOL") return BOOL;
    throw std::invalid_argument("Invalid Dtype string: "+str);
}

// Convert string to Rtype enum
inline Rtype stringToRtype(const std::string& str) {
    if (str == "HOLDING_REGISTER") return HOLDINGREGISTER;
    else if (str == "INPUT_REGISTER") return INPUTREGISTER;
    else if (str == "COIL") return COIL;
    else if (str == "DESCRETE_INPUT") return DESCRETEINPUT;
    throw std::invalid_argument("Invalid Rtype string: "+str);
}

// Convert string to Endian enum
inline Endian stringToEndian(const std::string& str) {
    if (str == "BIG") return BIG;
    else if (str == "LITTLE") return LITTLE;
    throw std::invalid_argument("Invalid Endian string: "+str);
}


#endif 