#include "channel.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>  // python interpreter
#include <pybind11/stl.h>  // type conversion


using namespace CUTIL;


Channel::Channel(int index){

    mapping_index = index;
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
    float value = behaviour.attr("getValue")().cast<float>();
    mb_server->getMapping()->tab_registers[mapping_index] = value;
}