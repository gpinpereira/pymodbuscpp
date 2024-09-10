#include "channel.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>  // python interpreter
#include <pybind11/stl.h>  // type conversion
#include "server_wrapper.h"

namespace py = pybind11;

using namespace CUTIL;


Channel::Channel(int first_register, int n_registers,  Rtype register_type, Dtype data_type, Endian endian){

    reg_start = first_register;
    reg_n = n_registers;
    dtype = data_type;
    rtype = register_type;
    endiantype = endian;

}

void Channel::setServer(WServer *server){
    mb_server = server;
}


void Channel::setBehaviour(char *behaviour_name){

    //py::scoped_interpreter guard{}; // start interpreter, dies when out of scope
    py::module Behaviours = py::module::import("Behaviours");
    behaviour = Behaviours.attr(behaviour_name)(2, 1);
}


void Channel::getBehaviourValue(){

    py::gil_scoped_acquire acquire;

    if (behaviour.attr("getValue").is_none()) {
            std::cout << "Python object doesn't have 'some_method'." << std::endl;
            return;
    }

    std::cout << "Getting value" << std::endl;
    std::cout << "Register value " << getStartingRegister() << std::endl;
        
    if(dtype == FLOAT || dtype == INTEGER) {
        uint32_t value32;

        if (dtype == INTEGER){
            std::cout << "Getting integer" << std::endl;
            value32 = behaviour.attr("getValue")().cast<uint32_t>();
        }
        else if (dtype == FLOAT){
            // Convert the float into its 32-bit binary representation
            float valuef = behaviour.attr("getValue")().cast<float>();
            value32 = *reinterpret_cast<uint32_t*>(&valuef);
        }

        uint16_t registers[2];
        registers[0] = (value32 >> 16) & 0xFFFF;  // High 16 bits
        registers[1] = value32 & 0xFFFF;          // Low 16 bits

        if(endiantype==BIG){
            setRegister(reg_start, registers[0]);
            setRegister(reg_start+1, registers[1]);
        } if(endiantype==LITTLE){
            setRegister(reg_start, registers[1]);
            setRegister(reg_start+1, registers[0]);
        }


    } else if (dtype == SHORT) {

        uint16_t value16 = behaviour.attr("getValue")().cast<uint16_t>();
        setRegister(reg_start, value16);

    } else if (dtype == BOOL) {
            
        bool valueb = behaviour.attr("getValue")().cast<bool>();
        setRegister(reg_start, valueb);

    } else{
            std::cout << "Unknown datatype" << std::endl;
    }

}

void Channel::setRegister(int reg, uint16_t value){


    if(rtype == HOLDINGREGISTER) {
        mb_server->getMapping()->tab_registers[reg] = value;
    } else if(rtype == COIL) {
        mb_server->getMapping()->tab_bits[reg] = value;
    } else if(rtype == INPUTREGISTER) {
        mb_server->getMapping()->tab_input_registers[reg] = value;
    } else if(rtype == DESCRETEINPUT) {
        mb_server->getMapping()->tab_input_bits[reg] = value;
    }
}



void Channel::setBehaviourValue(std::vector<uint16_t> registers){

    std::cout << "First register: " << reg_start << std::endl;
    std::cout << "N register: " << reg_n << std::endl;
    std::cout << "Datatype: " << dtype << std::endl;
    std::cout << "Registertype: " << rtype << std::endl;
    
    //py::gil_scoped_acquire acquire;
    py::gil_scoped_release release;
    py::gil_scoped_acquire acquire;
    
    if (behaviour.attr("setValue").is_none()) {
            std::cout << "Python object doesn't have 'some_method'." << std::endl;
            return;
    }


    if (dtype == INTEGER){
        int32_t value;
        if(endiantype==BIG){
            value = (static_cast<int32_t>(registers[0]) << 16) | registers[1];
        } if(endiantype==LITTLE){
            value = (static_cast<int32_t>(registers[1]) << 16) | registers[0];
        }
        std::cout << "Value: " << value << std::endl;
        behaviour.attr("setValue")(value);
    }
    else if (dtype == FLOAT){
        // Convert the float into its 32-bit binary representation

        uint32_t value32;
        if(endiantype==BIG){
            value32 = (static_cast<uint32_t>(registers[0]) << 16) | registers[1];
        } if(endiantype==LITTLE){
            value32 = (static_cast<uint32_t>(registers[1]) << 16) | registers[0];
        }

        float value = *reinterpret_cast<float*>(&value32);

        std::cout << "Value: " << value << std::endl;

        float valuef = behaviour.attr("getValue")().cast<float>();

        std::cout << "Value: " << value  << " : " << valuef << std::endl;
        behaviour.attr("setValue")(value);

    } else if (dtype == SHORT) {

        int16_t value = static_cast<int16_t>( registers[0]);
        std::cout << "Value: " << value << std::endl;
        behaviour.attr("setValue")(value);

    } else if (dtype == BOOL) {
            
        bool value = static_cast<bool>( registers[0]);
        behaviour.attr("setValue")(value);
        std::cout << "Value: " << value << std::endl;

    } else{
            std::cout << "Unknown datatype" << std::endl;
    }
    

}
