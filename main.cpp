//#include "modbus_.h"
#include <server_wrapper.h>


#include <pybind11/pybind11.h>
#include <pybind11/embed.h>  // python interpreter
#include <pybind11/stl.h>  // type conversion

namespace py = pybind11;

#include <iostream>
//using namespace std;
using namespace CUTIL;


int main(){

    //py::scoped_interpreter guard{}; // start interpreter, dies when out of scope
    //py::module Behaviours = py::module_::import("Behaviours");
    //py::object behaviour = Behaviours.attr("Bsetpoint")(2, 1);

    //float value = behaviour.attr("getValue")().cast<float>();
    py::scoped_interpreter guard{};

    WServer* server = new WServer(502);
    server->addChannel(0);
    server->start();

   
    return 0;
}