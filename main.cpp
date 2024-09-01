#include "modbus_.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>  // python interpreter
#include <pybind11/stl.h>  // type conversion

namespace py = pybind11;

#include <iostream>
//using namespace std;
using namespace CUTIL;


int main(){

    std::cout << "Running new" << std::endl;

    py::scoped_interpreter guard{}; // start interpreter, dies when out of scope

    py::module Behaviours = py::module::import("Behaviours");

    // Access the Python class
    py::object Bsetpoint = Behaviours.attr("Bsetpoint");

    // Create an instance of the Python class
    py::object bset = Bsetpoint(10, 2);
    bset.attr("setValue")(5);  // Increment the value
    float value = bset.attr("getValue")().cast<float>();

    std::cout << "The value is: " << value << "\n";



    CUTIL::cMODBUSServer *server;
    server = new CUTIL::cMODBUSServer();

    std::cout << "fri" << std::endl;

    std::string sc_address=server->getLocalIP("127.0.0.1"); // see: auto=dev
    std::cout << sc_address << std::endl;

    server->connect_TCP(sc_address, 502, 2);
    std::cout << "bender" << std::endl;
    
    server->execute();

    server->wait();
    std::cout << "fiished" << std::endl;

    //server->stop();
    return 0;
}