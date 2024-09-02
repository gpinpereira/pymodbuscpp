#include <channel.h>

using namespace CUTIL;


Channel::Channel(int index){

    mapping_index = index;
}

void Channel::setServer(CUTIL::cMODBUSServer *server){
    mb_server = server;
}


void setBehaviour(char behaviour_name){

    py::scoped_interpreter guard{}; // start interpreter, dies when out of scope
    py::module Behaviours = py::module::import("Behaviours");

    py::object Btype = Behaviours.attr(behaviour_name);
    behaviour = Btype(2, 1);
}


void Channel::updateMBValue(){

    mb_server->mb_mapping->tab_registers[mapping_index] = behaviour.attr("getValue")().cast<int>();
}