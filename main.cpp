//#include "modbus_.h"
//#include <server_wrapper.h>


#include <wrapper.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>  // python interpreter
#include <pybind11/stl.h>  // type conversion
#include <iostream>
using namespace CUTIL;


int main(){

    //py::scoped_interpreter guard{}; // start interpreter, dies when out of scope
    //py::module Behaviours = py::module_::import("Behaviours");
    //py::object behaviour = Behaviours.attr("Bsetpoint")(2, 1);

    //float value = behaviour.attr("getValue")().cast<float>();
    py::scoped_interpreter guard{};


    Wrapper* wrapper = new Wrapper();

    /*WServer* server = new WServer(502);

    server->readCSV();
    //server->addChannel(0, 2, HOLDINGREGISTER, FLOAT, LITTLE);
    //server->addChannel(0, 1, COIL, BOOL, LITTLE);
    server->start();*/

   
    return 0;
}