#include "channel.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>  // python interpreter
#include <pybind11/stl.h>  // type conversion


using namespace CUTIL;


Channel::Channel(int first_register, int n_registers, Dtype data_type){

    reg_start = first_register;
    reg_n = n_registers;
    dtype = data_type;
    rtype = HOLDINGREGISTER;
}

void Channel::setServer(WServer *server){
    mb_server = server;
}


void Channel::setBehaviour(char *behaviour_name){

    //py::scoped_interpreter guard{}; // start interpreter, dies when out of scope
    py::module Behaviours = py::module_::import("Behaviours");
    behaviour = Behaviours.attr("Bsetpoint")(2, 1);
}


void Channel::updateMBValue(){

    if (behaviour.attr("getValue").is_none()) {
            std::cout << "Python object doesn't have 'some_method'." << std::endl;
            return;
    }
        

    
    if(dtype == FLOAT || dtype == INTEGER) {
        std::cout << "The color is Red" << std::endl;
        uint32_t value32;

        if (dtype == INTEGER){
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

        std::cout << value32 << std::endl;
        mb_server->getMapping()->tab_registers[reg_start] = registers[0];
        mb_server->getMapping()->tab_registers[reg_start+1] = registers[1];

    } else if (dtype == SHORT) {

        uint16_t value16 = behaviour.attr("getValue")().cast<uint16_t>();
        mb_server->getMapping()->tab_registers[reg_start] = value16;

    } else if (dtype == BOOL) {
            
            bool valueb = behaviour.attr("getValue")().cast<bool>();
            std::cout << "The color is Blue " << valueb << std::endl;
            mb_server->getMapping()->tab_registers[reg_start] = valueb;

    } else{
            std::cout << "Unknown datatype" << std::endl;
    }

}

